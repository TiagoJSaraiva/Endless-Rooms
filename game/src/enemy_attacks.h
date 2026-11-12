#pragma once

#include "enemy_rules.h"
#include "projectile.h"

// Traduz as definições de ataque dos inimigos (enemy_rules) em blueprints de projétil.
// Os inimigos nunca usam as armas do jogador: reaproveitam só as famílias de projétil
// (Spear, FullCircleSwing e munição arremessada) com parâmetros próprios.
namespace EnemyAttacks {

// `damageMultiplier` é o escalonamento temporário de dificuldade (1 = sem efeito).
// Crítico sempre 0: inimigos não causam crítico.
ProjectileBlueprint BuildProjectile(const EnemyRules::EnemyDefinition& definition, float damageMultiplier);

// Distância máxima da ponta do ataque ao inimigo (estocada: reach + length; giro: raio;
// osso: alcance máximo). Usado nos testes de alcance.
float MaxReach(const EnemyRules::AttackDefinition& attack);

}  // namespace EnemyAttacks
