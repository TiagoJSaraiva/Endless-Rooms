// Simulação headless de runs: mede quantas salas especiais ficam visíveis ao jogador
// nas primeiras salas visitadas (metas de balanceamento). Usa o RoomManager real, sem janela.
#include "room_manager.h"

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

namespace {

constexpr std::uint64_t kFirstSeed = 1;
constexpr int kSeedCount = 500;
constexpr int kRoomsToVisit = 10;
// Limite de travessias de porta por run, para a caminhada nunca ficar presa em laço.
constexpr int kMaxMoves = 200;
// Meta aprovada: runs com ao menos uma loja visível até a 10ª sala visitada.
constexpr double kShopVisibleTargetPercent = 70.0;
constexpr double kPercentScale = 100.0;

struct RunResult {
    bool shopVisible{false};
    bool forgeVisible{false};
    bool chestVisible{false};
    int roomsGenerated{0};
    int shops{0};
    int forges{0};
    int chests{0};
};

// Portas utilizáveis da sala atual: não seladas e com sala de destino existente.
std::vector<RoomCoords> ReachableNeighbors(const RoomManager& manager, const Room& room) {
    std::vector<RoomCoords> result;
    for (const Doorway& door : room.Layout().doors) {
        if (door.sealed || !door.targetGenerated) continue;
        if (manager.TryGetRoom(door.targetCoords) == nullptr) continue;
        result.push_back(door.targetCoords);
    }
    return result;
}

// Uma sala é visível quando foi gerada e é a própria sala visitada ou vizinha por porta de uma visitada.
void MarkVisible(const RoomManager& manager, const Room& room, RunResult& result) {
    auto mark = [&result](RoomType type) {
        if (type == RoomType::Shop) result.shopVisible = true;
        if (type == RoomType::Forge) result.forgeVisible = true;
        if (type == RoomType::Chest) result.chestVisible = true;
    };
    mark(room.GetType());
    for (const RoomCoords& coords : ReachableNeighbors(manager, room)) {
        mark(manager.TryGetRoom(coords)->GetType());
    }
}

RunResult SimulateRun(std::uint64_t seed) {
    RoomManager manager(seed);
    // Props não influenciam tipo nem layout das salas e dominam o custo da geração.
    manager.SetPropsEnabled(false);
    std::mt19937_64 rng(seed * 7919u + 13u);
    std::unordered_set<RoomCoords, RoomCoordsHash> visited;

    RunResult result;
    visited.insert(manager.GetCurrentCoords());
    MarkVisible(manager, manager.GetCurrentRoom(), result);

    for (int move = 0; move < kMaxMoves && static_cast<int>(visited.size()) < kRoomsToVisit; ++move) {
        const Room& current = manager.GetCurrentRoom();
        std::vector<RoomCoords> options = ReachableNeighbors(manager, current);
        if (options.empty()) break;

        // Prefere salas ainda não visitadas; sem elas, volta por uma porta qualquer.
        std::vector<RoomCoords> unvisited;
        for (const RoomCoords& coords : options) {
            if (visited.count(coords) == 0) unvisited.push_back(coords);
        }
        const std::vector<RoomCoords>& pool = unvisited.empty() ? options : unvisited;
        const RoomCoords target = pool[rng() % pool.size()];

        bool moved = false;
        for (const Doorway& door : current.Layout().doors) {
            if (door.sealed || !door.targetGenerated || !(door.targetCoords == target)) continue;
            moved = manager.MoveToNeighbor(door.direction);
            break;
        }
        assert(moved);
        visited.insert(manager.GetCurrentCoords());
        MarkVisible(manager, manager.GetCurrentRoom(), result);
    }
    assert(static_cast<int>(visited.size()) == kRoomsToVisit);

    for (const auto& [coords, room] : manager.Rooms()) {
        (void)coords;
        ++result.roomsGenerated;
        if (room->GetType() == RoomType::Shop) ++result.shops;
        if (room->GetType() == RoomType::Forge) ++result.forges;
        if (room->GetType() == RoomType::Chest) ++result.chests;
    }
    return result;
}

} // namespace

int main() {
    int shopRuns = 0, forgeRuns = 0, chestRuns = 0;
    long generated = 0, shops = 0, forges = 0, chests = 0;

    for (int i = 0; i < kSeedCount; ++i) {
        const RunResult r = SimulateRun(kFirstSeed + i);
        shopRuns += r.shopVisible;
        forgeRuns += r.forgeVisible;
        chestRuns += r.chestVisible;
        generated += r.roomsGenerated;
        shops += r.shops;
        forges += r.forges;
        chests += r.chests;
    }

    const double shopPercent = kPercentScale * shopRuns / kSeedCount;
    std::cout << std::fixed << std::setprecision(1)
              << "Runs: " << kSeedCount << ", salas visitadas por run: " << kRoomsToVisit << '\n'
              << "Runs com loja visivel: " << shopPercent << "%\n"
              << "Runs com forja visivel: " << kPercentScale * forgeRuns / kSeedCount << "%\n"
              << "Runs com bau visivel: " << kPercentScale * chestRuns / kSeedCount << "%\n"
              << "Salas geradas por sala visitada: "
              << static_cast<double>(generated) / kSeedCount / kRoomsToVisit << '\n'
              << "Media por run de lojas/forjas/baus gerados: "
              << static_cast<double>(shops) / kSeedCount << " / "
              << static_cast<double>(forges) / kSeedCount << " / "
              << static_cast<double>(chests) / kSeedCount << '\n';

    assert(shopPercent >= kShopVisibleTargetPercent);
    std::cout << "Room simulation tests passed\n";
}
