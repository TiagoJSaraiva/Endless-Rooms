#include "enemy_common.h"

#include "enemy_attacks.h"
#include "projectile.h"
#include "raymath.h"
#include "room.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace {

// Mantém texturas de inimigos em cache para evitar recarregamentos.

struct CachedEnemyTexture {
    Texture2D texture{};
    bool attempted{false};
};

std::unordered_map<std::string, CachedEnemyTexture> g_enemyTextureCache;

constexpr float kHealthBarWidthPadding = 8.0f;
constexpr float kHealthBarHeight = 2.0f;
constexpr float kHealthBarVerticalOffset = 80.0f;
// Folga entre o topo do sprite e a barra de vida dos inimigos com definição própria.
constexpr float kHealthBarSpriteGap = 6.0f;
constexpr float kHealthBarBackgroundThickness = 1.0f;
const Color kHealthBarBackgroundColor = Color{12, 12, 18, 200};
const Color kHealthBarFillColor = Color{196, 64, 64, 230};

// Verifica sufixo ignorando caixa para tentar fallback de extensão.
bool EndsWithExtension(const std::string& path, const std::string& extension) {
    if (path.size() < extension.size()) {
        return false;
    }
    return std::equal(extension.rbegin(), extension.rend(), path.rbegin(), [](char a, char b) {
        return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
    });
}

// Carrega textura como está, aplicando filtro point se possível.
Texture2D LoadTextureExact(const std::string& path) {
    if (path.empty()) {
        return Texture2D{};
    }
    if (!FileExists(path.c_str())) {
        return Texture2D{};
    }
    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id != 0) {
        SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    }
    return texture;
}

// Tenta carregar arquivo e, se faltar extensão, força sufixo .png.
Texture2D LoadTextureWithFallback(const std::string& rawPath) {
    Texture2D texture = LoadTextureExact(rawPath);
    if (texture.id != 0) {
        return texture;
    }

    if (!rawPath.empty() && !EndsWithExtension(rawPath, ".png")) {
        std::string fallbackPath = rawPath + ".png";
        texture = LoadTextureExact(fallbackPath);
    }
    return texture;
}

// Retorna textura cacheada ou dispara carregamento lazy.
Texture2D AcquireEnemyTexture(const std::string& path) {
    if (path.empty()) {
        return Texture2D{};
    }
    auto& entry = g_enemyTextureCache[path];
    if (!entry.attempted) {
        entry.attempted = true;
        entry.texture = LoadTextureWithFallback(path);
    }
    return entry.texture;
}

} // namespace

EnemyCommon::EnemyCommon(const EnemyConfig& config,
                         float range,
                         const WeaponBlueprint* weapon,
                         const EnemySpriteInfo& spriteInfo)
    : Enemy(config), weapon_(weapon), range_(range), spriteInfo_(spriteInfo) {}

EnemyCommon::EnemyCommon(const EnemyConfig& config,
                         const EnemyRules::EnemyDefinition& definition,
                         const EnemySpriteInfo& spriteInfo,
                         float damageMultiplier)
    : Enemy(config),
      range_(definition.attack.startRange),
      spriteInfo_(spriteInfo),
      definition_(&definition),
      damageMultiplier_(damageMultiplier) {}

// Carrega texturas de idle/caminhada apenas uma vez.
void EnemyCommon::EnsureTexturesLoaded() const {
    if (texturesLoaded_) {
        return;
    }
    idleTexture_ = AcquireEnemyTexture(spriteInfo_.idleSpritePath);
    walkingTexture_ = AcquireEnemyTexture(spriteInfo_.walkingSpriteSheetPath);
    chargeTexture_ = AcquireEnemyTexture(spriteInfo_.chargeSpritePath);
    texturesLoaded_ = true;
}

// Movimenta o inimigo em direção ao jogador e administra ataques.
void EnemyCommon::Update(const EnemyUpdateContext& context) {
    float delta = context.deltaSeconds;
    attackCooldown_ = std::max(0.0f, attackCooldown_ - delta);

    float alpha = UpdateLifecycle(delta, context.playerInSameRoom);
    (void)alpha;

    const RoomLayout& layout = context.room.Layout();

    if (IsReturningToOrigin()) {
        Vector2 before = GetPosition();
        MoveTowardsOriginal(delta, layout);
        isMoving_ = Vector2Distance(before, GetPosition()) > 1e-3f;
        UpdateAnimation(delta, isMoving_);
        return;
    }

    if (!context.playerInSameRoom || !HasCompletedFade() || !IsAlive()) {
        // Sem o jogador na sala, um ataque em curso é abandonado (os projéteis já criados seguem).
        EnemyRules::CancelAttack(attackState_);
        isMoving_ = false;
        UpdateAnimation(delta, false);
        return;
    }

    Vector2 toPlayer = Vector2Subtract(context.playerPosition, GetPosition());
    float distance = Vector2Length(toPlayer);

    if (definition_ != nullptr) {
        UpdateDefinedEnemy(context, toPlayer, distance);
        return;
    }

    bool withinRange = (distance <= range_);

    if (!withinRange) {
        Vector2 before = GetPosition();
        Vector2 desired = MoveTowards(context.playerPosition, delta, GetSpeed());
        Vector2 resolved = ResolveRoomCollision(layout, desired);
        SetPosition(resolved);
        isMoving_ = Vector2Distance(before, resolved) > 1e-3f;
        if (!IsInsideRoomBounds(layout, resolved)) {
            StartReturnToOrigin();
        }
    } else {
        isMoving_ = false;
        AttemptAttack(context, toPlayer, distance);
    }

    facingLeft_ = (toPlayer.x < 0.0f);
    UpdateAnimation(delta, isMoving_);
}

// Persegue o jogador e conduz a máquina de estados de ataque (carga, disparo, intervalo).
void EnemyCommon::UpdateDefinedEnemy(const EnemyUpdateContext& context, const Vector2& toPlayer, float distance) {
    const EnemyRules::AttackDefinition& attack = definition_->attack;
    const float delta = context.deltaSeconds;
    const RoomLayout& layout = context.room.Layout();

    Vector2 aimDir{1.0f, 0.0f};
    if (distance > 1e-5f) {
        aimDir = Vector2Scale(toPlayer, 1.0f / distance);
    }

    EnemyRules::AttackTick tick = EnemyRules::AdvanceAttack(attackState_, attack, delta, distance <= attack.startRange);
    // A mira trava ao começar a carga: quem sai da frente a tempo desvia do golpe.
    if (tick.startedCharge || (tick.fire && attack.chargeSeconds <= 0.0f)) {
        lockedAim_ = aimDir;
    }
    if (tick.fire) {
        FireDefinedAttack(context);
    }

    bool chasing = false;
    if (EnemyRules::CanMove(attackState_, attack)) {
        const bool attackActive = attackState_.phase == EnemyRules::AttackPhase::Active;
        const float stopDistance = attackActive ? EnemyRules::kChaseContactDistance : attack.startRange;
        chasing = distance > stopDistance;
    }

    if (chasing) {
        Vector2 before = GetPosition();
        Vector2 desired = MoveTowards(context.playerPosition, delta, GetSpeed());
        Vector2 resolved = ResolveRoomCollision(layout, desired);
        SetPosition(resolved);
        isMoving_ = Vector2Distance(before, resolved) > 1e-3f;
        if (!IsInsideRoomBounds(layout, resolved)) {
            StartReturnToOrigin();
        }
    } else {
        isMoving_ = false;
    }

    // Durante a carga o corpo já olha para onde o golpe vai.
    const Vector2& facing = (attackState_.phase == EnemyRules::AttackPhase::Charging) ? lockedAim_ : aimDir;
    facingLeft_ = (facing.x < 0.0f);
    UpdateAnimation(delta, isMoving_);
}

void EnemyCommon::FireDefinedAttack(const EnemyUpdateContext& context) {
    ProjectileBlueprint projectile = EnemyAttacks::BuildProjectile(*definition_, damageMultiplier_);

    ProjectileSpawnContext spawnContext{};
    spawnContext.origin = GetPosition();
    spawnContext.followTarget = GetPositionAddress();
    // Estocada e giro seguem o inimigo: somem junto com ele (ver diagnostico-de-crashes.md).
    spawnContext.ownerLifetime = GetProjectileLifetime();
    spawnContext.aimDirection = lockedAim_;
    context.projectileSystem.SpawnProjectile(projectile, spawnContext);
}

// Instancia projétil baseado no blueprint da arma configurada.
void EnemyCommon::AttemptAttack(const EnemyUpdateContext& context,
                                const Vector2& toPlayer,
                                float distanceToPlayer) {
    if (weapon_ == nullptr) {
        return;
    }
    if (attackCooldown_ > 0.0f) {
        return;
    }

    ProjectileBlueprint projectile = weapon_->projectile;
    projectile.common.damage = weapon_->damage.baseDamage;
    projectile.common.criticalChance = weapon_->critical.baseChance;
    projectile.common.criticalMultiplier = (weapon_->critical.multiplier > 0.0f) ? weapon_->critical.multiplier : 1.0f;

    for (auto& thrown : projectile.thrownProjectiles) {
        thrown.common.damage = weapon_->damage.baseDamage;
        thrown.common.criticalChance = weapon_->critical.baseChance;
        thrown.common.criticalMultiplier = (weapon_->critical.multiplier > 0.0f) ? weapon_->critical.multiplier : 1.0f;
    }

    Vector2 aimDir = Vector2{1.0f, 0.0f};
    if (distanceToPlayer > 1e-5f) {
        aimDir = Vector2Scale(toPlayer, 1.0f / distanceToPlayer);
    }

    ProjectileSpawnContext spawnContext{};
    spawnContext.origin = GetPosition();
    spawnContext.followTarget = GetPositionAddress();
    spawnContext.ownerLifetime = GetProjectileLifetime();
    spawnContext.aimDirection = aimDir;

    context.projectileSystem.SpawnProjectile(projectile, spawnContext);

    float attackInterval = weapon_->cooldownSeconds;
    if (weapon_->cadence.baseAttacksPerSecond > 0.0f) {
        attackInterval = 1.0f / weapon_->cadence.baseAttacksPerSecond;
    }
    attackCooldown_ = std::max(0.05f, attackInterval);
}

// Controla frames do spritesheet conforme o estado de movimento.
void EnemyCommon::UpdateAnimation(float deltaSeconds, bool moving) {
    if (!moving || spriteInfo_.frameCount <= 1 || spriteInfo_.secondsPerFrame <= 0.0f) {
        animationTimer_ = 0.0f;
        currentFrame_ = 0;
        return;
    }

    animationTimer_ += deltaSeconds;
    while (animationTimer_ >= spriteInfo_.secondsPerFrame) {
        animationTimer_ -= spriteInfo_.secondsPerFrame;
        currentFrame_ = (currentFrame_ + 1) % spriteInfo_.frameCount;
    }
}

// Renderiza sprite e barra de vida com alpha baseado em fade.
void EnemyCommon::Draw(const EnemyDrawContext& context) const {
    if (!IsAlive()) {
        return;
    }

    float visibleAlpha = std::clamp(GetAlpha() * context.roomVisibility, 0.0f, 1.0f);
    if (visibleAlpha <= 0.0f) {
        return;
    }

    EnsureTexturesLoaded();

    Color tint{255, 255, 255, static_cast<unsigned char>(visibleAlpha * 255.0f)};
    Vector2 position = GetPosition();

    auto drawTexture = [&](const Texture2D& texture, int frameWidth, int frameHeight, int frameIndex) {
        if (texture.id == 0) {
            return false;
        }

        int columns = (frameWidth > 0) ? texture.width / frameWidth : 1;
        if (columns <= 0) {
            columns = 1;
        }
        int rows = (frameHeight > 0) ? texture.height / frameHeight : 1;
        if (rows <= 0) {
            rows = 1;
        }
        int clampedFrame = std::clamp(frameIndex, 0, columns * rows - 1);
        int srcX = (frameWidth > 0) ? (clampedFrame % columns) * frameWidth : 0;
        int srcY = (frameHeight > 0) ? (clampedFrame / columns) * frameHeight : 0;

        Rectangle src{static_cast<float>(srcX), static_cast<float>(srcY), static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
        if (facingLeft_) {
            src.width = -src.width;
        }

        Rectangle dest{position.x, position.y, static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
        Vector2 origin{static_cast<float>(frameWidth) * 0.5f, static_cast<float>(frameHeight)};
        DrawTexturePro(texture, src, dest, origin, 0.0f, tint);
        return true;
    };

    bool drew = false;
    if (attackState_.phase == EnemyRules::AttackPhase::Charging && chargeTexture_.id != 0) {
        drew = drawTexture(chargeTexture_, chargeTexture_.width, chargeTexture_.height, 0);
    }
    if (!drew && isMoving_ && walkingTexture_.id != 0 && spriteInfo_.frameCount > 0) {
        drew = drawTexture(walkingTexture_, spriteInfo_.frameWidth, spriteInfo_.frameHeight, currentFrame_);
    }

    if (!drew && idleTexture_.id != 0) {
        drew = drawTexture(idleTexture_, idleTexture_.width, idleTexture_.height, 0);
    }

    if (!drew) {
        float radius = GetCollisionRadius();
        DrawCircleV(position, radius, tint);
    }

    if (HasTakenDamage()) {
        float baseWidth = 0.0f;
        if (spriteInfo_.frameWidth > 0) {
            baseWidth = static_cast<float>(spriteInfo_.frameWidth);
        } else if (idleTexture_.id != 0) {
            baseWidth = static_cast<float>(idleTexture_.width);
        } else {
            baseWidth = GetCollisionRadius() * 2.0f;
        }
        float barWidth = std::max(8.0f, baseWidth + kHealthBarWidthPadding);
        float barHeight = std::max(1.0f, kHealthBarHeight);
        float border = std::max(0.0f, kHealthBarBackgroundThickness);

        float centerX = position.x;
        // Inimigos com sprite próprio têm alturas diferentes: a barra acompanha o topo do frame.
        float barY = position.y - ((definition_ != nullptr && spriteInfo_.frameHeight > 0)
                                       ? static_cast<float>(spriteInfo_.frameHeight) + kHealthBarSpriteGap
                                       : kHealthBarVerticalOffset);
        Rectangle background{
            centerX - barWidth * 0.5f,
            barY,
            barWidth,
            barHeight + border * 2.0f
        };
        DrawRectangleRec(background, kHealthBarBackgroundColor);

        float fillWidth = std::max(0.0f, (barWidth - border * 2.0f) * std::clamp(GetHealthFraction(), 0.0f, 1.0f));
        if (fillWidth > 0.0f) {
            Rectangle fill{
                background.x + border,
                background.y + border,
                fillWidth,
                barHeight
            };
            DrawRectangleRec(fill, kHealthBarFillColor);
        }
    }
}

// Libera texturas carregadas estaticamente para este tipo de inimigo.
void EnemyCommon::ShutdownSpriteCache() {
    for (auto& entry : g_enemyTextureCache) {
        if (entry.second.texture.id != 0) {
            UnloadTexture(entry.second.texture);
            entry.second.texture = Texture2D{};
        }
        entry.second.attempted = false;
    }
    g_enemyTextureCache.clear();
}
