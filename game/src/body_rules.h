#pragma once

#include "raylib.h"

// Corpo de um personagem como aparece no sprite, medido a partir dos pés (base do sprite).
// A área de dano cobre o corpo visível, e os ataques saem da altura das mãos; o colisor de
// movimento é outro, menor e restrito aos pés.
struct BodyGeometry {
    float width{0.0f};      // largura do corpo sem a arma, centrada no x dos pés
    float height{0.0f};     // dos pés ao topo da cabeça
    float handHeight{0.0f}; // altura das mãos no gesto de ataque, acima dos pés
};

namespace BodyRules {

// Falso para corpos não medidos (largura ou altura zero): quem chama mantém o comportamento antigo.
bool HasBody(const BodyGeometry& body);

// Retângulo de dano com a base nos pés e centrado em x.
Rectangle Hurtbox(Vector2 feet, const BodyGeometry& body);

// Ponto de onde saem projéteis e golpes.
Vector2 HandAnchor(Vector2 feet, const BodyGeometry& body);

// Centro da área de dano: ponto para onde os inimigos miram.
Vector2 Center(Vector2 feet, const BodyGeometry& body);

}  // namespace BodyRules
