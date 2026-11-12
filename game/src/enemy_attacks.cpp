#include "enemy_attacks.h"

namespace EnemyAttacks {

namespace {

// Tempos da estocada: mesmos da Machadinha do jogador (extensão, pausa, retração).
constexpr float kThrustExtendSeconds = 0.22f;
constexpr float kThrustIdleSeconds = 0.05f;
constexpr float kThrustRetractSeconds = 0.20f;
// Folga no tempo de vida do osso, só para não encerrá-lo antes da distância máxima.
constexpr float kBoneLifespanSlack = 0.5f;

ProjectileBlueprint BuildBone(const EnemyRules::AttackDefinition& attack, const EnemyRules::SpriteDefinition& sprite) {
    ProjectileBlueprint blueprint{};
    // O tipo Ranged cria o projétil de exibição (oculto) e os projéteis arremessados da lista.
    blueprint.kind = ProjectileKind::Ranged;
    blueprint.common.projectilesPerShot = 1;
    blueprint.common.displayMode = WeaponDisplayMode::Hidden;
    blueprint.common.debugColor = Color{230, 225, 205, 255};

    ThrownProjectileBlueprint bone{};
    bone.kind = ThrownProjectileKind::Ammunition;
    bone.common.projectilesPerShot = 1;
    bone.common.projectileSize = attack.boneLength;
    bone.common.projectileSpritePath = sprite.bonePath;
    bone.common.debugColor = Color{230, 225, 205, 255};
    // Folga sobre o tempo de voo: a distância máxima é quem encerra o osso.
    bone.common.lifespanSeconds = attack.boneMaxDistance / attack.boneSpeed + kBoneLifespanSlack;
    bone.ammunition.speed = attack.boneSpeed;
    bone.ammunition.maxDistance = attack.boneMaxDistance;
    bone.ammunition.radius = attack.boneRadius;
    blueprint.thrownProjectiles.push_back(bone);
    return blueprint;
}

ProjectileBlueprint BuildThrust(const EnemyRules::AttackDefinition& attack, const EnemyRules::SpriteDefinition& sprite) {
    ProjectileBlueprint blueprint{};
    blueprint.kind = ProjectileKind::Spear;
    blueprint.common.projectilesPerShot = 1;
    blueprint.common.lifespanSeconds = kThrustExtendSeconds + kThrustIdleSeconds + kThrustRetractSeconds;
    blueprint.common.debugColor = Color{190, 190, 200, 255};
    blueprint.common.weaponSpritePath = sprite.weaponSprite;
    blueprint.common.displayLength = attack.thrustLength;
    blueprint.common.displayThickness = attack.thrustThickness;
    blueprint.common.perTargetHitCooldownSeconds = attack.perTargetHitCooldownSeconds;

    blueprint.spear.length = attack.thrustLength;
    blueprint.spear.thickness = attack.thrustThickness;
    blueprint.spear.reach = attack.thrustReach;
    blueprint.spear.extendDuration = kThrustExtendSeconds;
    blueprint.spear.idleTime = kThrustIdleSeconds;
    blueprint.spear.retractDuration = kThrustRetractSeconds;
    blueprint.spear.followOwner = true;
    // Sem deslocamento de mão: a ponta chega exatamente a reach + length do inimigo.
    blueprint.spear.offset = Vector2{0.0f, 0.0f};
    return blueprint;
}

ProjectileBlueprint BuildSpin(const EnemyRules::AttackDefinition& attack, const EnemyRules::SpriteDefinition& sprite) {
    ProjectileBlueprint blueprint{};
    blueprint.kind = ProjectileKind::FullCircleSwing;
    blueprint.common.projectilesPerShot = 1;
    blueprint.common.debugColor = Color{200, 90, 80, 255};
    blueprint.common.weaponSpritePath = sprite.weaponSprite;
    blueprint.common.displayMode = WeaponDisplayMode::AimAligned;
    blueprint.common.displayLength = attack.spinRadius;
    blueprint.common.displayThickness = attack.spinThickness;
    blueprint.common.perTargetHitCooldownSeconds = attack.perTargetHitCooldownSeconds;

    blueprint.fullCircle.length = attack.spinRadius;
    blueprint.fullCircle.thickness = attack.spinThickness;
    blueprint.fullCircle.revolutions = attack.spinRevolutions;
    blueprint.fullCircle.angularSpeedDegreesPerSecond = attack.spinDegreesPerSecond;
    blueprint.fullCircle.followOwner = true;
    return blueprint;
}

}  // namespace

ProjectileBlueprint BuildProjectile(const EnemyRules::EnemyDefinition& definition, float damageMultiplier) {
    const EnemyRules::AttackDefinition& attack = definition.attack;
    ProjectileBlueprint blueprint{};
    switch (attack.pattern) {
        case EnemyRules::AttackPattern::ThrownBone: blueprint = BuildBone(attack, definition.sprite); break;
        case EnemyRules::AttackPattern::Thrust: blueprint = BuildThrust(attack, definition.sprite); break;
        case EnemyRules::AttackPattern::Spin: blueprint = BuildSpin(attack, definition.sprite); break;
    }

    const float damage = attack.damage * damageMultiplier;
    blueprint.common.damage = damage;
    blueprint.common.criticalChance = 0.0f;
    blueprint.common.criticalMultiplier = 1.0f;
    for (ThrownProjectileBlueprint& thrown : blueprint.thrownProjectiles) {
        thrown.common.damage = damage;
        thrown.common.criticalChance = 0.0f;
        thrown.common.criticalMultiplier = 1.0f;
        thrown.common.perTargetHitCooldownSeconds = attack.perTargetHitCooldownSeconds;
    }
    return blueprint;
}

float MaxReach(const EnemyRules::AttackDefinition& attack) {
    switch (attack.pattern) {
        case EnemyRules::AttackPattern::ThrownBone: return attack.boneMaxDistance;
        case EnemyRules::AttackPattern::Thrust: return attack.thrustReach + attack.thrustLength;
        case EnemyRules::AttackPattern::Spin: return attack.spinRadius;
    }
    return 0.0f;
}

}  // namespace EnemyAttacks
