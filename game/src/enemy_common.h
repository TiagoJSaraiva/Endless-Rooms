#pragma once

#include "enemy.h"
#include "enemy_rules.h"

#include <string>

#include "weapon.h"

// Define sprites usados para idle/movimento do inimigo comum.
struct EnemySpriteInfo {
    std::string idleSpritePath{};
    std::string walkingSpriteSheetPath{};
    // Frame único mostrado durante a carga do ataque; vazio = usa o idle.
    std::string chargeSpritePath{};
    int frameWidth{64};
    int frameHeight{64};
    int frameCount{1};
    float secondsPerFrame{0.18f};
};

// Implementa comportamento padrão de inimigos (andar/atacar à distância).
class EnemyCommon : public Enemy {
public:
    EnemyCommon(const EnemyConfig& config,
                float range,
                const WeaponBlueprint* weapon,
                const EnemySpriteInfo& spriteInfo);

    // Inimigo com ataque próprio (tabela de enemy_rules): carga, estocada, giro ou osso.
    // `damageMultiplier` é o escalonamento temporário de dificuldade; `definition` deve
    // viver mais que o inimigo (a tabela de enemy_rules é estática).
    EnemyCommon(const EnemyConfig& config,
                const EnemyRules::EnemyDefinition& definition,
                const EnemySpriteInfo& spriteInfo,
                float damageMultiplier);

    void Update(const EnemyUpdateContext& context) override;
    void Draw(const EnemyDrawContext& context) const override;

    // Limpa cache de sprites compartilhados entre instâncias.
    static void ShutdownSpriteCache();

private:
    // Garante que texturas foram carregadas antes de desenhar.
    void EnsureTexturesLoaded() const;
    // Avança frames de animação conforme movimento.
    void UpdateAnimation(float deltaSeconds, bool isMoving);
    // Comportamento dos inimigos com definição própria: persegue, carrega e ataca.
    void UpdateDefinedEnemy(const EnemyUpdateContext& context, const Vector2& toPlayer, float distance);
    // Cria o projétil do ataque definido, usando a mira travada no início da carga.
    void FireDefinedAttack(const EnemyUpdateContext& context);
    // Dispara projéteis caso o jogador esteja no alcance.
    void AttemptAttack(const EnemyUpdateContext& context, const Vector2& toPlayer, float distanceToPlayer);

    const WeaponBlueprint* weapon_{nullptr};
    float range_{320.0f};
    EnemySpriteInfo spriteInfo_{};

    mutable Texture2D idleTexture_{};
    mutable Texture2D walkingTexture_{};
    mutable Texture2D chargeTexture_{};
    mutable bool texturesLoaded_{false};

    // Ataque definido por enemy_rules (nullptr = usa `weapon_`, como caverna e mansão).
    const EnemyRules::EnemyDefinition* definition_{nullptr};
    EnemyRules::AttackState attackState_{};
    float damageMultiplier_{1.0f};
    Vector2 lockedAim_{1.0f, 0.0f};

    float attackCooldown_{0.0f};
    float animationTimer_{0.0f};
    int currentFrame_{0};
    bool facingLeft_{false};
    bool isMoving_{false};
};
