#include "weapon_rules.h"

#include <algorithm>
#include <cmath>

#include "weapon.h"

namespace WeaponRules {

namespace {
// Mesmo teto de crítico aplicado em WeaponState::RecalculateDerivedStats.
constexpr float kMaxCriticalChance = 0.75f;
// Tolerância para não contar um tick a mais por erro de ponto flutuante (0,24 / 0,08 etc.).
constexpr float kTickEpsilon = 1e-4f;
} // namespace

int LaserTicksPerPulse(float durationSeconds, float perTargetHitCooldownSeconds) {
    if (durationSeconds <= 0.0f) {
        return 0;
    }
    if (perTargetHitCooldownSeconds <= 0.0f) {
        return 1;
    }
    // Ticks em 0, cd, 2cd... enquanto o instante for menor que a duração.
    return static_cast<int>(std::ceil(durationSeconds / perTargetHitCooldownSeconds - kTickEpsilon));
}

float EstimateDps(const WeaponBlueprint& blueprint, float damagePerHit, int hitsPerUse) {
    float attacksPerSecond = 0.0f;
    if (blueprint.cadence.baseAttacksPerSecond > 0.0f) {
        attacksPerSecond = blueprint.cadence.baseAttacksPerSecond +
                           blueprint.cadence.dexterityGainPerPoint *
                               static_cast<float>(blueprint.passiveBonuses.primary.destreza);
        if (blueprint.cadence.attacksPerSecondCap > 0.0f) {
            attacksPerSecond = std::min(attacksPerSecond, blueprint.cadence.attacksPerSecondCap);
        }
    } else if (blueprint.cooldownSeconds > 0.0f) {
        attacksPerSecond = 1.0f / blueprint.cooldownSeconds;
    }

    const float critChance = std::clamp(
        blueprint.critical.baseChance + blueprint.critical.chancePerLetalidade * blueprint.passiveBonuses.secondary.letalidade,
        0.0f, kMaxCriticalChance);
    const float critMultiplier = (blueprint.critical.multiplier > 0.0f) ? blueprint.critical.multiplier : 1.0f;

    return damagePerHit * attacksPerSecond * static_cast<float>(hitsPerUse) *
           (1.0f + critChance * (critMultiplier - 1.0f));
}

float EstimateDps(const WeaponBlueprint& blueprint, int hitsPerUse) {
    return EstimateDps(blueprint, blueprint.damage.baseDamage, hitsPerUse);
}

} // namespace WeaponRules
