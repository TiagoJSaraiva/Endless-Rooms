#include "run_stats.h"

#include "room.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerHour = 3600;
constexpr size_t kDurationBufferSize = 32;
}

namespace RunStatsRules {
const char* const kUnknownItemName = "Item desconhecido";

void AddTime(RunStats& stats, float deltaSeconds) {
    if (deltaSeconds > 0.0f) {
        stats.elapsedSeconds += deltaSeconds;
    }
}

void AddEnemyDefeated(RunStats& stats) {
    ++stats.enemiesDefeated;
}

void AddCollection(RunStats& stats, int coins, const std::vector<RewardPouchItem>& items) {
    if (coins > 0) {
        stats.coinsCollected += coins;
    }
    for (const auto& item : items) {
        if (item.quantity <= 0) {
            continue;
        }
        auto found = std::find_if(stats.items.begin(), stats.items.end(),
                                  [&](const RunStatsItem& entry) { return entry.itemId == item.itemId; });
        if (found != stats.items.end()) {
            found->quantity += item.quantity;
        } else {
            stats.items.push_back(RunStatsItem{item.itemId, item.quantity});
        }
    }
}

int CountVisitedRooms(const std::vector<const Room*>& rooms) {
    int count = 0;
    for (const Room* room : rooms) {
        if (room != nullptr && room->IsVisited() && room->GetType() != RoomType::Lobby) {
            ++count;
        }
    }
    return count;
}

std::string FormatDuration(float seconds) {
    const int total = seconds > 0.0f ? static_cast<int>(std::floor(seconds)) : 0;
    const int hours = total / kSecondsPerHour;
    const int minutes = (total % kSecondsPerHour) / kSecondsPerMinute;
    const int secs = total % kSecondsPerMinute;
    char buffer[kDurationBufferSize];
    if (hours > 0) {
        std::snprintf(buffer, sizeof(buffer), "%d:%02d:%02d", hours, minutes, secs);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, secs);
    }
    return buffer;
}

std::vector<RunSummaryItemLine> BuildItemLines(const RunStats& stats, const ItemNameLookup& lookup) {
    std::vector<RunSummaryItemLine> lines;
    lines.reserve(stats.items.size());
    for (const auto& item : stats.items) {
        const char* name = lookup ? lookup(item.itemId) : nullptr;
        lines.push_back(RunSummaryItemLine{(name != nullptr && name[0] != '\0') ? name : kUnknownItemName,
                                           item.quantity});
    }
    return lines;
}
}
