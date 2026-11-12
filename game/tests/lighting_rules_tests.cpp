#include "lighting_rules.h"
#include "room.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace LightingRules;

namespace {

bool Near(float a, float b) {
    return std::fabs(a - b) < 0.001f;
}

bool LevelIs(const std::vector<AmbientZone>& zones, Vector2 point, float expected) {
    auto level = AmbientLevelAt(zones, point);
    return level.has_value() && Near(*level, expected);
}

Room MakeRoom(RoomCoords coords, TileRect bounds, BiomeType biome) {
    RoomLayout layout{};
    layout.widthTiles = bounds.width;
    layout.heightTiles = bounds.height;
    layout.tileBounds = bounds;
    return Room(coords, RoomSeedData{RoomType::Normal, biome, 1}, layout);
}

// Conecta duas salas pelo corredor informado, com portas nos dois lados, como o RoomManager faz.
void Connect(Room& from, Room& to, Direction direction, TileRect corridor) {
    Doorway door{};
    door.direction = direction;
    door.targetCoords = to.GetCoords();
    door.corridorTiles = corridor;
    door.targetGenerated = true;
    from.Layout().doors.push_back(door);
    door.direction = Opposite(direction);
    door.targetCoords = from.GetCoords();
    to.Layout().doors.push_back(door);
}

float Px(int tiles) {
    return static_cast<float>(tiles * TILE_SIZE);
}

void CheckTable() {
    assert(Near(AmbientLevel(BiomeType::Lobby), 1.0f));
    assert(Near(AmbientLevel(BiomeType::Dungeon), 0.65f));
    assert(Near(AmbientLevel(BiomeType::Cave), 0.75f));
    assert(Near(AmbientLevel(BiomeType::Mansion), 0.85f));
    assert(Near(AmbientLevel(BiomeType::Unknown), 1.0f));
}

void CheckRoomsAndOutside() {
    auto room = MakeRoom({0, 0}, {0, 0, 10, 8}, BiomeType::Dungeon);
    auto zones = BuildAmbientZones({&room});
    assert(zones.size() == 1);
    assert(LevelIs(zones, {Px(5), Px(4)}, 0.65f));
    assert(!AmbientLevelAt(zones, {Px(20), Px(4)}).has_value());
    assert(!AmbientLevelAt({}, {0, 0}).has_value());
}

// Lobby à esquerda (x 0..10), Dungeon à direita (x 14..24), corredor x 10..14.
void CheckEastCorridorGradient() {
    auto lobby = MakeRoom({0, 0}, {0, 0, 10, 8}, BiomeType::Lobby);
    auto dungeon = MakeRoom({1, 0}, {14, 0, 10, 8}, BiomeType::Dungeon);
    Connect(lobby, dungeon, Direction::East, {10, 3, 4, 2});
    auto zones = BuildAmbientZones({&lobby, &dungeon});
    assert(zones.size() == 4);
    const float y = Px(4);
    assert(LevelIs(zones, {Px(10), y}, 1.0f));
    assert(LevelIs(zones, {Px(12), y}, 0.825f));
    assert(LevelIs(zones, {Px(14), y}, 0.65f));
    assert(LevelIs(zones, {Px(5), y}, 1.0f));
    assert(LevelIs(zones, {Px(20), y}, 0.65f));
    // O lado do corredor visto pela Dungeon (West) dá o mesmo valor.
    assert(LevelIs(zones, {Px(11), y}, 1.0f - 0.35f * 0.25f));
}

// Corredores verticais: Dungeon embaixo (y 10..18) e Mansão em cima (y 0..6), corredor y 6..10.
void CheckVerticalCorridorGradient() {
    auto bottom = MakeRoom({0, 1}, {0, 10, 8, 8}, BiomeType::Dungeon);
    auto top = MakeRoom({0, 0}, {0, 0, 8, 6}, BiomeType::Mansion);
    Connect(bottom, top, Direction::North, {3, 6, 2, 4});
    const float x = Px(4);
    for (const auto& zones : {BuildAmbientZones({&bottom, &top}), BuildAmbientZones({&top, &bottom})}) {
        assert(LevelIs(zones, {x, Px(10)}, 0.65f));
        assert(LevelIs(zones, {x, Px(8)}, 0.75f));
        assert(LevelIs(zones, {x, Px(6)}, 0.85f));
        assert(LevelIs(zones, {x, Px(9)}, 0.65f + 0.2f * 0.25f));
    }
}

void CheckSameBiomeAndMissingTarget() {
    auto a = MakeRoom({0, 0}, {0, 0, 10, 8}, BiomeType::Dungeon);
    auto b = MakeRoom({1, 0}, {14, 0, 10, 8}, BiomeType::Dungeon);
    Connect(a, b, Direction::East, {10, 3, 4, 2});
    auto zones = BuildAmbientZones({&a, &b});
    assert(LevelIs(zones, {Px(12), Px(4)}, 0.65f));
    // Sem a sala de destino na lista, o corredor mantém o nível da origem.
    auto lobby = MakeRoom({0, 0}, {0, 0, 10, 8}, BiomeType::Lobby);
    auto dungeon = MakeRoom({1, 0}, {14, 0, 10, 8}, BiomeType::Dungeon);
    Connect(lobby, dungeon, Direction::East, {10, 3, 4, 2});
    zones = BuildAmbientZones({&lobby});
    assert(LevelIs(zones, {Px(13), Px(4)}, 1.0f));
}

void CheckSealedAndEmptyDoors() {
    auto room = MakeRoom({0, 0}, {0, 0, 10, 8}, BiomeType::Dungeon);
    Doorway sealed{};
    sealed.direction = Direction::East;
    sealed.corridorTiles = {10, 3, 4, 2};
    sealed.sealed = true;
    room.Layout().doors.push_back(sealed);
    Doorway empty{};
    empty.direction = Direction::West;
    room.Layout().doors.push_back(empty);
    auto zones = BuildAmbientZones({&room});
    assert(zones.size() == 1);
    assert(!AmbientLevelAt(zones, {Px(12), Px(4)}).has_value());
}

// O gradiente vence a zona da sala mesmo quando as duas se sobrepõem.
void CheckCorridorPrecedence() {
    std::vector<AmbientZone> zones{
        {{0, 0, 100, 100}, Direction::North, 1.0f, 1.0f},
        {{40, 0, 20, 100}, Direction::South, 1.0f, 0.5f},
    };
    assert(LevelIs(zones, {50, 50}, 0.75f));
    assert(LevelIs(zones, {10, 50}, 1.0f));
}

} // namespace

int main() {
    CheckTable();
    CheckRoomsAndOutside();
    CheckEastCorridorGradient();
    CheckVerticalCorridorGradient();
    CheckSameBiomeAndMissingTarget();
    CheckSealedAndEmptyDoors();
    CheckCorridorPrecedence();
    std::cout << "Lighting rules tests passed" << std::endl;
    return 0;
}
