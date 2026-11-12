#include "run_stats.h"
#include "room.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace RunStatsRules;

namespace {

bool Near(float a, float b) {
    return std::fabs(a - b) < 0.001f;
}

Room MakeRoom(RoomType type, bool visited) {
    RoomLayout layout{};
    layout.widthTiles = 4;
    layout.heightTiles = 4;
    layout.tileBounds = TileRect{0, 0, 4, 4};
    Room room(RoomCoords{0, 0}, RoomSeedData{type, BiomeType::Cave, 1}, layout);
    room.SetVisited(visited);
    return room;
}

void TestTime() {
    RunStats stats;
    AddTime(stats, 0.5f);
    AddTime(stats, 1.25f);
    AddTime(stats, -3.0f);
    assert(Near(stats.elapsedSeconds, 1.75f));
}

void TestFormatDuration() {
    assert(FormatDuration(0.0f) == "00:00");
    assert(FormatDuration(-5.0f) == "00:00");
    assert(FormatDuration(59.9f) == "00:59");
    assert(FormatDuration(60.0f) == "01:00");
    assert(FormatDuration(3599.0f) == "59:59");
    assert(FormatDuration(3600.0f) == "1:00:00");
    assert(FormatDuration(36000.0f + 125.0f) == "10:02:05");
}

void TestEnemies() {
    RunStats stats;
    AddEnemyDefeated(stats);
    AddEnemyDefeated(stats);
    assert(stats.enemiesDefeated == 2);
}

void TestCollection() {
    RunStats stats;
    AddCollection(stats, 0, {});
    assert(stats.coinsCollected == 0 && stats.items.empty());

    AddCollection(stats, 5, {{7, 2}, {3, 1}});
    AddCollection(stats, 4, {{3, 2}, {9, 1}, {7, 1}, {11, 0}});
    assert(stats.coinsCollected == 9);
    assert(stats.items.size() == 3);
    assert(stats.items[0].itemId == 7 && stats.items[0].quantity == 3);
    assert(stats.items[1].itemId == 3 && stats.items[1].quantity == 3);
    assert(stats.items[2].itemId == 9 && stats.items[2].quantity == 1);
}

void TestVisitedRooms() {
    Room lobby = MakeRoom(RoomType::Lobby, true);
    Room a = MakeRoom(RoomType::Normal, true);
    Room b = MakeRoom(RoomType::Shop, true);
    Room c = MakeRoom(RoomType::Normal, false);
    assert(CountVisitedRooms({&lobby, &a, &b, &c, nullptr}) == 2);
    assert(CountVisitedRooms({}) == 0);
}

void TestItemLines() {
    RunStats stats;
    AddCollection(stats, 0, {{1, 2}, {2, 1}});
    auto lookup = [](int id) -> const char* { return id == 1 ? "Pocao" : nullptr; };
    auto lines = BuildItemLines(stats, lookup);
    assert(lines.size() == 2);
    assert(lines[0].name == "Pocao" && lines[0].quantity == 2);
    assert(lines[1].name == kUnknownItemName && lines[1].quantity == 1);
    assert(BuildItemLines(stats, nullptr)[0].name == kUnknownItemName);
}

}

int main() {
    TestTime();
    TestFormatDuration();
    TestEnemies();
    TestCollection();
    TestVisitedRooms();
    TestItemLines();
    std::cout << "Run stats tests passed\n";
    return 0;
}
