#include "inventory_rules.h"

#include <algorithm>
#include <array>
#include <cmath>

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

ItemCategory ItemCategoryFromId(const InventoryUIState& state, int id) {
    if (id <= 0) { return ItemCategory::None; }
    for (const ItemDefinition& candidate : state.items) {
        if (candidate.id == id) { return candidate.category; }
    }
    return ItemCategory::None;
}

} // namespace InventoryRules
