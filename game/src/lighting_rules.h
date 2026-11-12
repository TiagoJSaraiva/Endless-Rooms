#pragma once

#include "raylib.h"

#include <optional>
#include <vector>

#include "room_types.h"

class Room;

// Regras da escuridão em tela cheia (decisão 0009). O nível é a luminosidade
// multiplicada sobre o mundo: 1 = sem escurecimento; 0,65 = filtro preto com alpha 0,35.
namespace LightingRules {

float AmbientLevel(BiomeType biome);

// Área em pixels de mundo. Em salas, startLevel == endLevel. Em corredores, o nível
// vai de startLevel (lado da sala de origem) a endLevel seguindo `direction`.
struct AmbientZone {
    Rectangle area{};
    Direction direction{Direction::North};
    float startLevel{1.0f};
    float endLevel{1.0f};
};

// Zonas montadas a partir das salas recebidas; os ponteiros só precisam valer durante a chamada.
// Portas seladas ou sem corredor não geram zona. Se a sala de destino não estiver na lista,
// o corredor mantém o nível da sala de origem.
std::vector<AmbientZone> BuildAmbientZones(const std::vector<const Room*>& rooms);

// Corredores têm precedência sobre salas. Retorna vazio fora de qualquer zona, para que o
// chamador mantenha o último nível em vez de saltar.
std::optional<float> AmbientLevelAt(const std::vector<AmbientZone>& zones, Vector2 point);

} // namespace LightingRules
