#pragma once

#include "raylib.h"
#include "room_types.h"

#include <vector>

class Room;

namespace RoomCollisionRules {

// Recuo aplicado entre o limite visual inferior e o limite de movimento da sala.
int BottomInsetPixels(BiomeType biome);

// Constrói os limites de colisão em pixels para o retângulo de tiles da sala.
Rectangle RoomBoundsForCollision(const TileRect& tileBounds, BiomeType biome);

// Área por onde projéteis podem voar, em pixels: pisos das salas mais corredores de portas
// abertas. Porta fechada ou selada bloqueia, então o corredor dela fica de fora.
std::vector<Rectangle> ProjectilePassableAreas(const std::vector<const Room*>& rooms);

} // namespace RoomCollisionRules
