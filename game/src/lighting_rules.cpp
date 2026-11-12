#include "lighting_rules.h"

#include <algorithm>
#include <unordered_map>

#include "room.h"

namespace LightingRules {

namespace {

struct BiomeAmbient {
    BiomeType biome;
    float level;
};

constexpr BiomeAmbient kAmbientTable[] = {
    {BiomeType::Lobby, 1.0f},
    {BiomeType::Dungeon, 0.65f},
    {BiomeType::Cave, 0.75f},
    {BiomeType::Mansion, 0.85f},
};

constexpr float kDefaultAmbient = 1.0f;

Rectangle TilesToPixels(const TileRect& tiles) {
    return Rectangle{static_cast<float>(tiles.x * TILE_SIZE), static_cast<float>(tiles.y * TILE_SIZE),
                     static_cast<float>(tiles.width * TILE_SIZE), static_cast<float>(tiles.height * TILE_SIZE)};
}

bool Contains(const Rectangle& area, Vector2 point) {
    return point.x >= area.x && point.x <= area.x + area.width &&
           point.y >= area.y && point.y <= area.y + area.height;
}

// Progresso de 0 (borda da sala de origem) a 1 (borda da sala de destino).
float CorridorProgress(const AmbientZone& zone, Vector2 point) {
    const Rectangle& a = zone.area;
    float progress = 0.0f;
    switch (zone.direction) {
        case Direction::North: progress = (a.y + a.height - point.y) / a.height; break;
        case Direction::South: progress = (point.y - a.y) / a.height; break;
        case Direction::East: progress = (point.x - a.x) / a.width; break;
        case Direction::West: progress = (a.x + a.width - point.x) / a.width; break;
    }
    return std::clamp(progress, 0.0f, 1.0f);
}

bool IsCorridor(const AmbientZone& zone) {
    return zone.startLevel != zone.endLevel;
}

} // namespace

float AmbientLevel(BiomeType biome) {
    for (const auto& entry : kAmbientTable) {
        if (entry.biome == biome) return entry.level;
    }
    return kDefaultAmbient;
}

std::vector<AmbientZone> BuildAmbientZones(const std::vector<const Room*>& rooms) {
    std::unordered_map<RoomCoords, const Room*, RoomCoordsHash> lookup;
    for (const Room* room : rooms) lookup[room->GetCoords()] = room;

    std::vector<AmbientZone> zones;
    for (const Room* room : rooms) {
        const float level = AmbientLevel(room->GetBiome());
        zones.push_back({TilesToPixels(room->Layout().tileBounds), Direction::North, level, level});
        for (const auto& door : room->Layout().doors) {
            const TileRect& c = door.corridorTiles;
            if (door.sealed || c.width <= 0 || c.height <= 0) continue;
            float targetLevel = level;
            auto target = lookup.find(door.targetCoords);
            if (target != lookup.end()) targetLevel = AmbientLevel(target->second->GetBiome());
            zones.push_back({TilesToPixels(c), door.direction, level, targetLevel});
        }
    }
    return zones;
}

std::optional<float> AmbientLevelAt(const std::vector<AmbientZone>& zones, Vector2 point) {
    std::optional<float> roomLevel;
    for (const auto& zone : zones) {
        if (!Contains(zone.area, point)) continue;
        // Corredores de nível constante equivalem a uma sala: só o gradiente precisa vencer a borda.
        if (IsCorridor(zone)) {
            const float t = CorridorProgress(zone, point);
            return zone.startLevel + (zone.endLevel - zone.startLevel) * t;
        }
        if (!roomLevel) roomLevel = zone.startLevel;
    }
    return roomLevel;
}

} // namespace LightingRules
