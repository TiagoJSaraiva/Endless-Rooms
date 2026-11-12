#pragma once

#include <algorithm>

// Regras de arma que não dependem de janela nem de estado de jogo (decisão 0007).
struct WeaponBlueprint;

namespace WeaponRules {

// Cada ponto de Poder soma 1% ao dano final do golpe.
constexpr float kPowerDamagePerPoint = 0.01f;

// Multiplicador de dano vindo do Poder: 1 + poder/100, nunca negativo (Poder muito
// negativo zera o dano em vez de invertê-lo em cura).
inline float PowerDamageMultiplier(int poder) {
    return std::max(0.0f, 1.0f + kPowerDamagePerPoint * static_cast<float>(poder));
}

// Quantos acertos um laser dá num mesmo alvo parado por pulso: um a cada
// `perTargetHitCooldownSeconds`, começando no instante 0, dentro de `duration`.
int LaserTicksPerPulse(float durationSeconds, float perTargetHitCooldownSeconds);

// Estimativa de DPS contra alvo único: dano × usos/s × acertos por uso × (1 + crit × (mult - 1)).
// Considera atributos de ataque e Poder zerados e só os passivos da própria arma
// (Destreza e Letalidade), a mesma convenção de docs/sistemas/balanceamento.md.
// `hitsPerUse` é quantos acertos um uso da arma dá no alvo (1 na maioria; ver docs/sistemas/armas.md).
float EstimateDps(const WeaponBlueprint& blueprint, float damagePerHit, int hitsPerUse);

// Igual ao anterior, usando o dano base do blueprint.
float EstimateDps(const WeaponBlueprint& blueprint, int hitsPerUse);

} // namespace WeaponRules
