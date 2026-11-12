#include "room.h"
#include "room_collision_rules.h"

#include <cassert>
#include <iostream>

namespace {

Room MakeRoom(RoomCoords coords, TileRect bounds) {
    RoomLayout layout{};
    layout.widthTiles = bounds.width;
    layout.heightTiles = bounds.height;
    layout.tileBounds = bounds;
    return Room(coords, RoomSeedData{RoomType::Normal, BiomeType::Dungeon, 1}, layout);
}

Doorway MakeDoor(TileRect corridor, bool open, bool sealed) {
    Doorway door{};
    door.direction = Direction::East;
    door.corridorTiles = corridor;
    door.sealed = sealed;
    door.doorState = std::make_shared<DoorInstance>();
    door.doorState->open = open;
    return door;
}

bool Contains(const std::vector<Rectangle>& areas, Rectangle expected) {
    for (const Rectangle& area : areas) {
        if (area.x == expected.x && area.y == expected.y &&
            area.width == expected.width && area.height == expected.height) {
            return true;
        }
    }
    return false;
}

void CheckProjectilePassableAreas() {
    const float tile = static_cast<float>(TILE_SIZE);
    const TileRect corridor{10, 3, 2, 2};
    const Rectangle corridorPixels{10 * tile, 3 * tile, 2 * tile, 2 * tile};

    Room room = MakeRoom({0, 0}, TileRect{0, 0, 10, 8});
    room.Layout().doors.push_back(MakeDoor(corridor, true, false));
    auto areas = RoomCollisionRules::ProjectilePassableAreas({&room, nullptr});
    assert(areas.size() == 2);
    assert(Contains(areas, Rectangle{0.0f, 0.0f, 10 * tile, 8 * tile}));
    assert(Contains(areas, corridorPixels));

    room.Layout().doors[0].doorState->open = false;
    areas = RoomCollisionRules::ProjectilePassableAreas({&room});
    assert(areas.size() == 1);
    assert(!Contains(areas, corridorPixels));

    room.Layout().doors[0].doorState->open = true;
    room.Layout().doors[0].sealed = true;
    areas = RoomCollisionRules::ProjectilePassableAreas({&room});
    assert(areas.size() == 1);

    room.Layout().doors[0].sealed = false;
    room.Layout().doors[0].doorState.reset();
    assert(RoomCollisionRules::ProjectilePassableAreas({&room}).size() == 1);
}

} // namespace

int main() {
    const TileRect tiles{3, 5, 8, 6};
    const BiomeType biomes[] = {
        BiomeType::Lobby,
        BiomeType::Cave,
        BiomeType::Mansion,
        BiomeType::Dungeon
    };

    for (BiomeType biome : biomes) {
        const int inset = RoomCollisionRules::BottomInsetPixels(biome);
        const Rectangle bounds = RoomCollisionRules::RoomBoundsForCollision(tiles, biome);
        assert(inset > 0);
        assert(bounds.x == 3.0f * TILE_SIZE);
        assert(bounds.y == 5.0f * TILE_SIZE);
        assert(bounds.width == 8.0f * TILE_SIZE);
        assert(bounds.height == 6.0f * TILE_SIZE - inset);
        assert(bounds.y + bounds.height == 11.0f * TILE_SIZE - inset);
    }

    const Rectangle shortRoom = RoomCollisionRules::RoomBoundsForCollision(
        TileRect{0, 0, 1, 0}, BiomeType::Lobby);
    assert(shortRoom.height == 0.0f);

    CheckProjectilePassableAreas();

    std::cout << "Room collision rule tests passed.\n";
    return 0;
}
