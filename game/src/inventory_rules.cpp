#include "inventory_rules.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

namespace InventoryRules {

namespace {

constexpr int kConsumableMaxStack = 10;
constexpr int kMaterialMaxStack = 99;

// Tabela de valores. Ajuste a economia da demo aqui.
constexpr int kMaterialValue = 20;
constexpr int kConsumableValue = 35;
// Equipamento (arma, armadura e resultado de forja) por raridade: índice 0 = raridade 1.
constexpr std::array<int, 5> kEquipmentValueByRarity{50, 150, 250, 370, 600};

} // namespace

// Define se o item pode empilhar no inventário ou no baú.
bool IsStackableCategory(ItemCategory category) {
    return category == ItemCategory::Consumable || category == ItemCategory::Material;
}

// Quantidade máxima por pilha em função da categoria.
int MaxStackForCategory(ItemCategory category) {
    if (category == ItemCategory::Consumable) {
        return kConsumableMaxStack;
    }
    if (category == ItemCategory::Material) {
        return kMaterialMaxStack;
    }
    return 1;
}

int ItemValue(ItemCategory category, int rarity) {
    if (category == ItemCategory::Material) {
        return kMaterialValue;
    }
    if (category == ItemCategory::Consumable) {
        return kConsumableValue;
    }
    const int index = std::clamp(rarity, 1, static_cast<int>(kEquipmentValueByRarity.size())) - 1;
    return kEquipmentValueByRarity[static_cast<size_t>(index)];
}

int BuyPrice(int itemValue) {
    if (itemValue <= 0) {
        return 0;
    }
    // Conta em inteiros (valor x 13 = décimos de moeda) para evitar erro de ponto flutuante no meio-termo.
    constexpr int kTenths = 10;
    const int buyTenths = itemValue * static_cast<int>(std::lround(kBuyMultiplier * kTenths));
    const int stepTenths = kBuyPriceStep * kTenths;
    return (buyTenths + stepTenths / 2) / stepTenths * kBuyPriceStep;
}

int SaleValue(int itemValue, int quantity, float sellMultiplier) {
    if (itemValue <= 0 || quantity <= 0) {
        return 0;
    }
    // Epsilon cobre o meio-termo (ex.: 35 x 0,3 = 10,5), que deve arredondar para cima.
    constexpr double kRoundingEpsilon = 1e-6;
    const double total = static_cast<double>(itemValue) * quantity * std::max(0.0f, sellMultiplier);
    return static_cast<int>(std::floor(total + 0.5 + kRoundingEpsilon));
}

int ForgeFullChanceCost(int productValue) {
    if (productValue <= 0) {
        return 0;
    }
    return static_cast<int>(std::lround(productValue * kForgeFullChanceValueFraction));
}

float ForgeSuccessChance(int investedCoins, int productValue) {
    if (productValue <= 0) {
        return 0.0f;
    }
    const double fullCost = productValue * kForgeFullChanceValueFraction;
    const double chance = std::max(0, investedCoins) / fullCost;
    return static_cast<float>(std::clamp(chance, 0.0, 1.0));
}

bool IsLootEligible(const ItemDefinition& item, bool debugMode) {
    if (item.id <= 0 || item.category == ItemCategory::None) {
        return false;
    }
    return !item.debugOnly || debugMode;
}

const ItemDefinition* PickByRarityWeight(const std::vector<const ItemDefinition*>& candidates,
                                         std::mt19937_64& rng) {
    if (candidates.empty()) {
        return nullptr;
    }

    // Só entram no sorteio as raridades da tabela que têm ao menos um candidato.
    std::array<int, kRarityWeights.size()> weights{};
    for (const ItemDefinition* item : candidates) {
        const int rarity = item->rarity;
        if (rarity >= 1 && rarity <= static_cast<int>(kRarityWeights.size())) {
            weights[static_cast<size_t>(rarity - 1)] = kRarityWeights[static_cast<size_t>(rarity - 1)];
        }
    }
    const int totalWeight = std::accumulate(weights.begin(), weights.end(), 0);
    if (totalWeight <= 0) {
        // Nenhum candidato com raridade da tabela: sorteio uniforme evita loja/baú vazios.
        std::uniform_int_distribution<size_t> any(0, candidates.size() - 1);
        return candidates[any(rng)];
    }

    int roll = std::uniform_int_distribution<int>(0, totalWeight - 1)(rng);
    int chosenRarity = 1;
    for (size_t i = 0; i < weights.size(); ++i) {
        if (roll < weights[i]) {
            chosenRarity = static_cast<int>(i) + 1;
            break;
        }
        roll -= weights[i];
    }

    std::vector<const ItemDefinition*> sameRarity;
    for (const ItemDefinition* item : candidates) {
        if (item->rarity == chosenRarity) {
            sameRarity.push_back(item);
        }
    }
    std::uniform_int_distribution<size_t> pick(0, sameRarity.size() - 1);
    return sameRarity[pick(rng)];
}

namespace {

void RemoveCandidate(std::vector<const ItemDefinition*>& candidates, const ItemDefinition* item) {
    candidates.erase(std::remove(candidates.begin(), candidates.end(), item), candidates.end());
}

std::vector<const ItemDefinition*> EligibleItems(const std::vector<ItemDefinition>& catalog, bool debugMode) {
    std::vector<const ItemDefinition*> eligible;
    eligible.reserve(catalog.size());
    for (const ItemDefinition& item : catalog) {
        if (IsLootEligible(item, debugMode)) {
            eligible.push_back(&item);
        }
    }
    return eligible;
}

} // namespace

std::vector<ShopSlotRoll> RollShopSlots(const std::vector<ItemDefinition>& catalog,
                                        bool debugMode,
                                        std::mt19937_64& rng) {
    std::vector<const ItemDefinition*> pool = EligibleItems(catalog, debugMode);
    std::vector<ShopSlotRoll> slots;
    slots.reserve(kShopSlotCount);

    auto addSlot = [&](const ItemDefinition* item) {
        ShopSlotRoll slot{};
        slot.itemId = item->id;
        slot.stock = 1;
        if (item->category == ItemCategory::Consumable) {
            slot.stock = std::uniform_int_distribution<int>(kShopConsumableMinStock, kShopConsumableMaxStock)(rng);
        }
        slots.push_back(slot);
        RemoveCandidate(pool, item);
    };

    // 1) Slot garantido de consumível (a poção de cura), uniforme entre os consumíveis.
    std::vector<const ItemDefinition*> consumables;
    for (const ItemDefinition* item : pool) {
        if (item->category == ItemCategory::Consumable) {
            consumables.push_back(item);
        }
    }
    if (!consumables.empty()) {
        std::uniform_int_distribution<size_t> pick(0, consumables.size() - 1);
        addSlot(consumables[pick(rng)]);
    }

    // 2) Slot premium: raridade >= 2 pelo peso de raridade.
    std::vector<const ItemDefinition*> premium;
    for (const ItemDefinition* item : pool) {
        if (item->rarity >= kShopMinHighRarity) {
            premium.push_back(item);
        }
    }
    if (const ItemDefinition* item = PickByRarityWeight(premium, rng)) {
        addSlot(item);
    }

    // 3) Demais slots: peso de raridade entre os itens restantes.
    while (static_cast<int>(slots.size()) < kShopSlotCount) {
        const ItemDefinition* item = PickByRarityWeight(pool, rng);
        if (item == nullptr) {
            break;
        }
        addSlot(item);
    }
    return slots;
}

std::vector<ChestLootEntry> RollChestLoot(const std::vector<ItemDefinition>& catalog,
                                          bool debugMode,
                                          std::mt19937_64& rng) {
    std::vector<const ItemDefinition*> pool = EligibleItems(catalog, debugMode);
    const int slotCount = std::uniform_int_distribution<int>(kChestMinSlots, kChestMaxSlots)(rng);

    std::vector<ChestLootEntry> loot;
    for (int i = 0; i < slotCount; ++i) {
        const ItemDefinition* item = PickByRarityWeight(pool, rng);
        if (item == nullptr) {
            break;
        }

        int quantity = 1;
        if (item->category == ItemCategory::Consumable) {
            quantity = std::uniform_int_distribution<int>(kChestConsumableMinQuantity, kChestConsumableMaxQuantity)(rng);
        } else if (item->category == ItemCategory::Material) {
            quantity = std::uniform_int_distribution<int>(kChestMaterialMinQuantity, kChestMaterialMaxQuantity)(rng);
        }
        quantity = std::min(quantity, MaxStackForCategory(item->category));
        loot.push_back(ChestLootEntry{item->id, quantity});
        RemoveCandidate(pool, item);
    }
    return loot;
}

ItemCategory ItemCategoryFromId(const InventoryUIState& state, int id) {
    if (id <= 0) { return ItemCategory::None; }
    for (const ItemDefinition& candidate : state.items) {
        if (candidate.id == id) { return candidate.category; }
    }
    return ItemCategory::None;
}

} // namespace InventoryRules
