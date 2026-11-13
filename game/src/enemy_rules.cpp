#include "enemy_rules.h"

#include <algorithm>
#include <cmath>

namespace EnemyRules {

namespace {

constexpr float kFullTurnDegrees = 360.0f;

// ---- Metas de balanceamento (docs/sistemas/balanceamento.md, "Atributos dos inimigos (1.7)") ----
constexpr float kEnemiesPerTile = 0.02f;
constexpr int kMinEnemiesPerRoom = 2;
constexpr int kEnemiesPerExecutioner = 4;

constexpr int kScalingFreeRooms = 5;
constexpr float kHealthPerExtraRoom = 0.05f;
constexpr float kDamagePerExtraRoom = 0.03f;

constexpr float kMinSpawnWeight = 0.0001f;

constexpr float SpinSeconds(float revolutions, float degreesPerSecond) {
    return revolutions * kFullTurnDegrees / degreesPerSecond;
}

// Estocada: extensão + pausa + retração (mesmos tempos da Machadinha do jogador).
constexpr float kThrustExtendSeconds = 0.22f;
constexpr float kThrustIdleSeconds = 0.05f;
constexpr float kThrustRetractSeconds = 0.20f;

constexpr float kExecutionerSpinRevolutions = 1.6f;
constexpr float kExecutionerSpinSpeed = 480.0f;

const std::array<EnemyDefinition, 3> kDungeonEnemies = [] {
    std::array<EnemyDefinition, 3> table{};

    EnemyDefinition& victim = table[static_cast<std::size_t>(EnemyKind::Victim)];
    victim.kind = EnemyKind::Victim;
    victim.id = 110;
    victim.name = "dungeon_victim";
    victim.maxHealth = 20.0f;
    victim.speed = 170.0f;
    victim.collisionRadius = 22.0f;
    victim.spawnWeight = 1.0f;
    victim.pouches = {35, 5, 0};
    victim.sprite = {"./assets/img/enemies/dungeon_victim", 48, 87, 4, 0.14f, "",
                     "assets/img/enemies/dungeon_victim/bone.png"};
    victim.attack.pattern = AttackPattern::ThrownBone;
    victim.attack.damage = 5.0f;
    victim.attack.startRange = 480.0f;
    victim.attack.chargeSeconds = 0.0f;
    victim.attack.intervalSeconds = 1.6f;
    victim.attack.boneSpeed = 380.0f;
    victim.attack.boneMaxDistance = 600.0f;
    victim.attack.boneRadius = 8.0f;
    victim.attack.boneLength = 28.0f;

    EnemyDefinition& guard = table[static_cast<std::size_t>(EnemyKind::Guard)];
    guard.kind = EnemyKind::Guard;
    guard.id = 111;
    guard.name = "dungeon_guard";
    guard.maxHealth = 45.0f;
    guard.speed = 140.0f;
    guard.collisionRadius = 24.0f;
    guard.spawnWeight = 1.0f;
    guard.pouches = {35, 20, 0};
    guard.sprite = {"./assets/img/enemies/dungeon_guard", 56, 98, 4, 0.16f,
                    "assets/img/enemies/dungeon_guard/spear.png", ""};
    guard.attack.pattern = AttackPattern::Thrust;
    guard.attack.damage = 10.0f;
    guard.attack.startRange = 120.0f;
    guard.attack.chargeSeconds = 0.35f;
    guard.attack.intervalSeconds = 1.8f;
    guard.attack.activeSeconds = kThrustExtendSeconds + kThrustIdleSeconds + kThrustRetractSeconds;
    guard.attack.perTargetHitCooldownSeconds = 0.6f;
    guard.attack.thrustReach = 58.0f;
    guard.attack.thrustLength = 62.0f;
    guard.attack.thrustThickness = 22.0f;

    EnemyDefinition& executioner = table[static_cast<std::size_t>(EnemyKind::Executioner)];
    executioner.kind = EnemyKind::Executioner;
    executioner.id = 112;
    executioner.name = "dungeon_executioner";
    executioner.maxHealth = 130.0f;
    executioner.speed = 90.0f;
    executioner.collisionRadius = 30.0f;
    executioner.spawnWeight = 0.35f;
    executioner.pouches = {0, 40, 40};
    executioner.sprite = {"./assets/img/enemies/dungeon_executioner", 81, 123, 4, 0.22f,
                          "assets/img/enemies/dungeon_executioner/axe.png", ""};
    executioner.attack.pattern = AttackPattern::Spin;
    executioner.attack.damage = 16.0f;
    executioner.attack.startRange = 130.0f;
    executioner.attack.chargeSeconds = 0.8f;
    executioner.attack.intervalSeconds = 4.0f;
    executioner.attack.activeSeconds = SpinSeconds(kExecutionerSpinRevolutions, kExecutionerSpinSpeed);
    executioner.attack.movesDuringAttack = true;
    // 0,6 s por alvo em 1,2 s de giro: no máximo 2 acertos por giro.
    executioner.attack.perTargetHitCooldownSeconds = 0.6f;
    executioner.attack.spinRadius = 130.0f;
    executioner.attack.spinThickness = 34.0f;
    executioner.attack.spinRevolutions = kExecutionerSpinRevolutions;
    executioner.attack.spinDegreesPerSecond = kExecutionerSpinSpeed;

    return table;
}();

float PouchAverageCoins(RewardPouchType type) {
    const RewardPouchDefinition& definition = GetRewardPouchDefinition(type);
    return 0.5f * static_cast<float>(definition.minCoins + definition.maxCoins);
}

}  // namespace

const float kChaseContactDistance = 48.0f;

const std::array<EnemyDefinition, 3>& DungeonEnemies() {
    return kDungeonEnemies;
}

const EnemyDefinition& Definition(EnemyKind kind) {
    return kDungeonEnemies[static_cast<std::size_t>(kind)];
}

RewardPouchDropTable DropTable(const PouchChances& chances) {
    RewardPouchDropTable table;
    if (chances.small > 0) table.push_back({RewardPouchType::Small, chances.small});
    if (chances.medium > 0) table.push_back({RewardPouchType::Medium, chances.medium});
    if (chances.big > 0) table.push_back({RewardPouchType::Big, chances.big});
    return table;
}

float ExpectedCoins(const PouchChances& chances) {
    constexpr float kPercent = 100.0f;
    return (static_cast<float>(chances.small) * PouchAverageCoins(RewardPouchType::Small) +
            static_cast<float>(chances.medium) * PouchAverageCoins(RewardPouchType::Medium) +
            static_cast<float>(chances.big) * PouchAverageCoins(RewardPouchType::Big)) / kPercent;
}

int EnemyCountForArea(int areaTiles) {
    const long count = std::lround(static_cast<float>(std::max(areaTiles, 0)) * kEnemiesPerTile);
    return std::max(static_cast<int>(count), kMinEnemiesPerRoom);
}

int ExecutionerCap(int enemyCount) {
    return std::max(enemyCount, 0) / kEnemiesPerExecutioner;
}

std::vector<EnemyKind> ComposeRoom(int enemyCount, std::mt19937& rng) {
    std::vector<double> weights;
    for (const EnemyDefinition& definition : kDungeonEnemies) {
        weights.push_back(std::max(static_cast<double>(definition.spawnWeight), static_cast<double>(kMinSpawnWeight)));
    }
    std::discrete_distribution<int> pick(weights.begin(), weights.end());
    std::bernoulli_distribution coin(0.5);

    const int cap = ExecutionerCap(enemyCount);
    int executioners = 0;
    std::vector<EnemyKind> kinds;
    for (int i = 0; i < enemyCount; ++i) {
        EnemyKind kind = static_cast<EnemyKind>(pick(rng));
        if (kind == EnemyKind::Executioner) {
            if (executioners < cap) {
                ++executioners;
            } else {
                kind = coin(rng) ? EnemyKind::Victim : EnemyKind::Guard;
            }
        }
        kinds.push_back(kind);
    }
    return kinds;
}

DifficultyScale ScaleForClearedRooms(int normalRoomsCleared) {
    const int k = std::max(0, normalRoomsCleared - kScalingFreeRooms);
    return {1.0f + kHealthPerExtraRoom * static_cast<float>(k), 1.0f + kDamagePerExtraRoom * static_cast<float>(k)};
}

bool ClearedRooms::MarkCleared(const RoomCoords& coords) {
    return rooms_.insert(coords).second;
}

AttackTick AdvanceAttack(AttackState& state, const AttackDefinition& attack, float deltaSeconds, bool targetInRange) {
    AttackTick tick;
    state.cooldown = std::max(0.0f, state.cooldown - deltaSeconds);

    // Ativa o ataque e entra na fase ativa (se o ataque durar) ou volta a Ready.
    auto activate = [&] {
        tick.fire = true;
        if (attack.activeSeconds > 0.0f) {
            state.phase = AttackPhase::Active;
            state.phaseTimer = attack.activeSeconds;
        } else {
            state.phase = AttackPhase::Ready;
            state.phaseTimer = 0.0f;
        }
    };

    switch (state.phase) {
        case AttackPhase::Ready:
            if (targetInRange && state.cooldown <= 0.0f) {
                state.cooldown = attack.intervalSeconds;
                if (attack.chargeSeconds > 0.0f) {
                    state.phase = AttackPhase::Charging;
                    state.phaseTimer = attack.chargeSeconds;
                    tick.startedCharge = true;
                } else {
                    activate();
                }
            }
            break;
        case AttackPhase::Charging:
            state.phaseTimer -= deltaSeconds;
            if (state.phaseTimer <= 0.0f) {
                activate();
            }
            break;
        case AttackPhase::Active:
            state.phaseTimer -= deltaSeconds;
            if (state.phaseTimer <= 0.0f) {
                state.phase = AttackPhase::Ready;
                state.phaseTimer = 0.0f;
            }
            break;
    }
    return tick;
}

void CancelAttack(AttackState& state) {
    state.phase = AttackPhase::Ready;
    state.phaseTimer = 0.0f;
}

bool CanMove(const AttackState& state, const AttackDefinition& attack) {
    switch (state.phase) {
        case AttackPhase::Ready: return true;
        case AttackPhase::Charging: return false;
        case AttackPhase::Active: return attack.movesDuringAttack;
    }
    return true;
}

}  // namespace EnemyRules
