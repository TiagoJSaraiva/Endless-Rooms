#include "room_type_rules.h"

#include <algorithm>

namespace RoomTypeRules {

const std::vector<RoomTypeWeight>& Weights() {
    // Boss fica fora até a sala de boss ser retomada. Antes, ele tinha chance crescente por sala
    // (peso − 0,5 por sala), removida junto com a sala de boss na demo. Quando o boss voltar, será
    // preciso reintroduzir uma chance que cresça com o avanço da run, de forma melhor que a antiga.
    static const std::vector<RoomTypeWeight> weights{
        {RoomType::Normal, 71.0},
        {RoomType::Forge, 12.0},
        {RoomType::Shop, 12.0},
        {RoomType::Chest, 5.0},
    };
    return weights;
}

bool IsSpecial(RoomType type) {
    return type == RoomType::Shop || type == RoomType::Forge || type == RoomType::Chest;
}

RoomType Pick(double roll, const std::vector<RoomType>& neighborTypes, const std::vector<RoomTypeWeight>& weights) {
    auto allowed = [&](RoomType type) {
        return !IsSpecial(type) ||
               std::find(neighborTypes.begin(), neighborTypes.end(), type) == neighborTypes.end();
    };

    double total = 0.0;
    for (const RoomTypeWeight& entry : weights) {
        if (allowed(entry.type) && entry.weight > 0.0) total += entry.weight;
    }
    if (total <= 0.0) return RoomType::Normal;

    double pick = std::clamp(roll, 0.0, 1.0) * total;
    RoomType last = RoomType::Normal;
    for (const RoomTypeWeight& entry : weights) {
        if (!allowed(entry.type) || entry.weight <= 0.0) continue;
        last = entry.type;
        if (pick < entry.weight) return entry.type;
        pick -= entry.weight;
    }
    // Só alcançado com roll = 1 ou por arredondamento no limite superior.
    return last;
}

}  // namespace RoomTypeRules
