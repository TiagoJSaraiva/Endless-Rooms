#include "room_manager.h"
#include "biome_config.h"
#include "room_type_rules.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

void CheckGeneratedWorld(std::uint64_t seed) {
    RoomManager manager(seed);
    assert(manager.Rooms().size() > 1);

    const Room& lobby = manager.GetRoom(RoomCoords{0, 0});
    const Doorway* lobbyNorth = lobby.FindDoor(Direction::North);
    assert(lobbyNorth != nullptr);
    assert(lobbyNorth->offset == 4);

    BiomeType selectedBiome = BiomeType::Unknown;
    for (const auto& [coords, roomPointer] : manager.Rooms()) {
        (void)coords;
        const Room& room = *roomPointer;
        if (room.GetType() == RoomType::Lobby) {
            assert(room.GetBiome() == BiomeType::Lobby);
        } else {
            // Caverna e mansão estão congeladas na demo: o sorteio só produz dungeon.
            assert(room.GetBiome() == BiomeType::Dungeon);
            if (selectedBiome == BiomeType::Unknown) {
                selectedBiome = room.GetBiome();
            } else {
                assert(room.GetBiome() == selectedBiome);
            }
        }

        const RoomLayout& layout = room.Layout();
        assert(layout.widthTiles > 0 && layout.widthTiles % 2 == 0);
        assert(layout.heightTiles > 0 && layout.heightTiles % 2 == 0);

        for (const Doorway& door : layout.doors) {
            assert(door.offset >= 0 && door.offset % 2 == 0);

            const TileRect& corridor = door.corridorTiles;
            if (corridor.width <= 0 || corridor.height <= 0) continue;
            if (door.corridorLength < 4 || door.corridorLength > 6 ||
                door.corridorLength % 2 != 0) {
                std::cerr << "Unexpected corridor for seed " << seed << " at room ("
                          << room.GetCoords().x << ", " << room.GetCoords().y << "): direction="
                          << static_cast<int>(door.direction) << " length=" << door.corridorLength
                          << " bounds=" << corridor.width << "x" << corridor.height << '\n';
            }
            assert(door.corridorLength >= 4 && door.corridorLength <= 6);
            assert(door.corridorLength % 2 == 0);
            if (door.direction == Direction::North || door.direction == Direction::South) {
                assert(corridor.width == door.width);
                assert(corridor.height % 2 == 0);
            } else {
                assert(corridor.height == door.width);
                assert(corridor.width % 2 == 0);
            }
        }
    }
}

// O gerador não produz mais salas de boss; Normal e ao menos um tipo especial aparecem.
void CheckNoBossRooms(std::uint64_t seed) {
    constexpr std::size_t kTargetRoomCount = 8;
    constexpr int kMaxExpansionRadius = 3;
    RoomManager manager(seed);
    for (int radius = 2; radius <= kMaxExpansionRadius && manager.Rooms().size() < kTargetRoomCount; ++radius) {
        manager.EnsureNeighborsGenerated(manager.GetCurrentCoords(), radius);
    }
    assert(manager.Rooms().size() >= kTargetRoomCount);

    for (const auto& [coords, roomPointer] : manager.Rooms()) {
        (void)coords;
        assert(roomPointer->GetType() != RoomType::Boss);
    }
}

void CheckSpecialRoomsAppear() {
    constexpr std::uint64_t kSeedCount = 20;
    bool sawNormal = false;
    bool sawSpecial = false;
    for (std::uint64_t seed = 1; seed <= kSeedCount; ++seed) {
        RoomManager manager(seed);
        manager.EnsureNeighborsGenerated(manager.GetCurrentCoords(), 3);
        for (const auto& [coords, roomPointer] : manager.Rooms()) {
            (void)coords;
            const RoomType type = roomPointer->GetType();
            if (type == RoomType::Normal) sawNormal = true;
            if (type == RoomType::Forge || type == RoomType::Shop || type == RoomType::Chest) sawSpecial = true;
        }
    }
    assert(sawNormal && sawSpecial);
}

void CheckForcedBiome(std::uint64_t seed, BiomeType forcedBiome) {
    RoomManager manager(seed, forcedBiome);
    assert(manager.GetRoom(RoomCoords{0, 0}).GetBiome() == BiomeType::Lobby);
    assert(manager.Rooms().size() > 1);

    for (const auto& [coords, roomPointer] : manager.Rooms()) {
        (void)coords;
        const Room& room = *roomPointer;
        if (room.GetType() == RoomType::Lobby) continue;
        assert(room.GetBiome() == forcedBiome);
    }
}

void CheckWallPropRegistry() {
    RoomManager manager(1234, BiomeType::Dungeon);
    assert(!manager.WallProps().empty());
    const auto before = manager.WallProps();
    const auto revision = manager.GetLayoutRevision();
    const auto* storage = manager.WallProps().data();
    assert(revision == manager.GetWallPropsRevision());
    for (int frame = 0; frame < 120; ++frame) {
        manager.EnsureNeighborsGenerated(manager.GetCurrentCoords());
        assert(manager.GetLayoutRevision() == revision);
        assert(manager.GetWallPropsRevision() == revision);
        assert(manager.WallProps().data() == storage);
    }
    // Door animation, lock state, and visits affect exposure, not structural placement.
    for (const auto& entry : manager.Rooms()) {
        entry.second->SetVisited(true);
        for (auto& door : entry.second->Layout().doors) if (door.doorState) {
            door.doorState->opening = true;
            door.doorState->open = true;
            door.doorState->fadeProgress = 0.5f;
            door.doorState->interactionState = DoorInteractionState::Locked;
        }
    }
    manager.EnsureNeighborsGenerated(manager.GetCurrentCoords());
    assert(manager.GetWallPropsRevision() == revision && manager.WallProps().data() == storage);
    const auto& after = manager.WallProps();
    assert(before.size() == after.size());
    for (std::size_t i = 0; i < before.size(); ++i) {
        assert(before[i].key == after[i].key);
        assert(before[i].animationPhase == after[i].animationPhase);
    }
    RoomManager replay(1234, BiomeType::Dungeon);
    assert(before.size() == replay.WallProps().size());
    for (std::size_t i = 0; i < before.size(); ++i)
        assert(before[i].key == replay.WallProps()[i].key);

    // Expanding the map configures new rooms/doors and must refresh the registry.
    const auto roomCount = manager.Rooms().size();
    manager.EnsureNeighborsGenerated(manager.GetCurrentCoords(), 3);
    assert(manager.Rooms().size() > roomCount);
    assert(manager.GetLayoutRevision() > revision);
    assert(manager.GetWallPropsRevision() == manager.GetLayoutRevision());
    std::vector<WallRules::RoomView> views;
    for (const auto& entry : manager.Rooms()) views.push_back({entry.second.get(), 1});
    const auto expected = WallPropRules::Generate(views, manager.GetWorldSeed());
    assert(expected.size() == manager.WallProps().size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        assert(expected[i].key == manager.WallProps()[i].key);
        assert(expected[i].animationPhase == manager.WallProps()[i].animationPhase);
    }
    const auto expandedRevision = manager.GetWallPropsRevision();
    storage = manager.WallProps().data();
    manager.EnsureNeighborsGenerated(manager.GetCurrentCoords(), 3);
    assert(manager.GetWallPropsRevision() == expandedRevision && manager.WallProps().data() == storage);
    manager = RoomManager(5678, BiomeType::Cave);
    assert(manager.WallProps().empty());
}

void CheckBiomeConfigParsing() {
    assert(BiomeConfig::ParseBiomeOverride("cave") == BiomeType::Cave);
    assert(BiomeConfig::ParseBiomeOverride("  MANSION \n") == BiomeType::Mansion);
    assert(BiomeConfig::ParseBiomeOverride("\tDungeon  ") == BiomeType::Dungeon);
    assert(!BiomeConfig::ParseBiomeOverride(""));
    assert(!BiomeConfig::ParseBiomeOverride("   \n\t"));
    assert(!BiomeConfig::ParseBiomeOverride("lobby"));
    assert(!BiomeConfig::ParseBiomeOverride("unknown-biome"));
    assert(!BiomeConfig::LoadBiomeOverride("missing-biome-config-for-test.txt"));
}

} // namespace

// Conta os tipos sorteados por `Pick` em rolagens uniformes de [0, 1).
std::vector<int> PickCounts(const std::vector<RoomType>& neighbors, int samples) {
    std::vector<int> counts(static_cast<std::size_t>(RoomType::Unknown) + 1, 0);
    for (int i = 0; i < samples; ++i) {
        const double roll = (i + 0.5) / samples;
        ++counts[static_cast<std::size_t>(RoomTypeRules::Pick(roll, neighbors))];
    }
    return counts;
}

int Count(const std::vector<int>& counts, RoomType type) {
    return counts[static_cast<std::size_t>(type)];
}

void CheckRoomTypeRules() {
    constexpr int kSamples = 10000;
    // Sem vizinhos especiais: os pesos valem como porcentagem (71/12/12/5).
    const auto free = PickCounts({RoomType::Normal, RoomType::Lobby}, kSamples);
    assert(Count(free, RoomType::Normal) == 7100);
    assert(Count(free, RoomType::Forge) == 1200);
    assert(Count(free, RoomType::Shop) == 1200);
    assert(Count(free, RoomType::Chest) == 500);

    // Forja vizinha: forja sai do sorteio e o peso dela se redistribui (71/12/5 de 88).
    const auto nearForge = PickCounts({RoomType::Forge}, kSamples);
    assert(Count(nearForge, RoomType::Forge) == 0);
    assert(std::abs(Count(nearForge, RoomType::Normal) - 8068) <= 1);
    assert(std::abs(Count(nearForge, RoomType::Shop) - 1364) <= 1);
    assert(std::abs(Count(nearForge, RoomType::Chest) - 568) <= 1);

    // Os três especiais vizinhos: só sobra sala normal.
    const auto surrounded = PickCounts({RoomType::Shop, RoomType::Forge, RoomType::Chest}, kSamples);
    assert(Count(surrounded, RoomType::Normal) == kSamples);

    // Limites da rolagem.
    assert(RoomTypeRules::Pick(0.0, {}) == RoomType::Normal);
    assert(RoomTypeRules::Pick(1.0, {}) == RoomType::Chest);
    assert(RoomTypeRules::Pick(1.0, {RoomType::Chest}) == RoomType::Shop);
}

int main() {
    CheckRoomTypeRules();
    CheckBiomeConfigParsing();
    CheckWallPropRegistry();
    for (std::uint64_t seed = 1; seed <= 16; ++seed) CheckGeneratedWorld(seed);
    for (std::uint64_t seed = 1; seed <= 20; ++seed) CheckNoBossRooms(seed);
    CheckSpecialRoomsAppear();
    CheckForcedBiome(1234, BiomeType::Dungeon);
    CheckForcedBiome(5678, BiomeType::Cave);
    CheckForcedBiome(9012, BiomeType::Mansion);
    std::cout << "Room generation tests passed\n";
}
