#include "room_collision_rules.h"

#include "room.h"

#include <algorithm>

namespace RoomCollisionRules {
namespace {

// Valores independentes para permitir ajuste fino do limite inferior por bioma.
constexpr int kLobbyBottomInsetPixels = 15;
constexpr int kCaveBottomInsetPixels = 15;
constexpr int kMansionBottomInsetPixels = 15;
constexpr int kDungeonBottomInsetPixels = 15;
constexpr int kDefaultBottomInsetPixels = 15;

} // namespace

int BottomInsetPixels(BiomeType biome) {
    switch (biome) {
        case BiomeType::Lobby: return kLobbyBottomInsetPixels;
        case BiomeType::Cave: return kCaveBottomInsetPixels;
        case BiomeType::Mansion: return kMansionBottomInsetPixels;
        case BiomeType::Dungeon: return kDungeonBottomInsetPixels;
        case BiomeType::Unknown: return kDefaultBottomInsetPixels;
    }
    return kDefaultBottomInsetPixels;
}

Rectangle RoomBoundsForCollision(const TileRect& tileBounds, BiomeType biome) {
    const float tileSize = static_cast<float>(TILE_SIZE);
    const float height = static_cast<float>(tileBounds.height) * tileSize;
    const float bottomInset = static_cast<float>(BottomInsetPixels(biome));

    return Rectangle{
        static_cast<float>(tileBounds.x) * tileSize,
        static_cast<float>(tileBounds.y) * tileSize,
        static_cast<float>(tileBounds.width) * tileSize,
        std::max(0.0f, height - bottomInset)
    };
}

namespace {
Rectangle TilesToPixels(const TileRect& tiles) {
    const float tileSize = static_cast<float>(TILE_SIZE);
    return Rectangle{
        static_cast<float>(tiles.x) * tileSize,
        static_cast<float>(tiles.y) * tileSize,
        static_cast<float>(tiles.width) * tileSize,
        static_cast<float>(tiles.height) * tileSize
    };
}
} // namespace

std::vector<Rectangle> ProjectilePassableAreas(const std::vector<const Room*>& rooms) {
    std::vector<Rectangle> areas;
    for (const Room* room : rooms) {
        if (room == nullptr) continue;
        const RoomLayout& layout = room->Layout();
        areas.push_back(TilesToPixels(layout.tileBounds));
        for (const Doorway& door : layout.doors) {
            if (door.sealed || !door.doorState || !door.doorState->open) continue;
            areas.push_back(TilesToPixels(door.corridorTiles));
        }
    }
    return areas;
}

} // namespace RoomCollisionRules
