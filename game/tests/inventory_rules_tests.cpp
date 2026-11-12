#include "inventory_rules.h"

#include <array>
#include <cassert>

namespace {

struct ValueCase {
    ItemCategory category;
    int rarity;
    int value;
    int buy;
    int sell;
};

// Tabela aprovada em docs/sistemas/balanceamento.md.
void TestValueBuyAndSellTable() {
    const std::array<ValueCase, 7> cases{{
        {ItemCategory::Material, 1, 20, 25, 6},
        {ItemCategory::Consumable, 1, 35, 45, 11},
        {ItemCategory::Armor, 1, 50, 65, 15},
        {ItemCategory::Weapon, 2, 150, 195, 45},
        {ItemCategory::Armor, 3, 250, 325, 75},
        {ItemCategory::Armor, 4, 370, 480, 111},
        {ItemCategory::Weapon, 5, 600, 780, 180},
    }};
    for (const ValueCase& c : cases) {
        const int value = InventoryRules::ItemValue(c.category, c.rarity);
        assert(value == c.value);
        assert(InventoryRules::BuyPrice(value) == c.buy);
        assert(InventoryRules::SaleValue(value, 1, InventoryRules::kBaseSellMultiplier) == c.sell);
    }

    // Raridade fora da tabela é limitada às pontas; valor inválido não gera preço.
    assert(InventoryRules::ItemValue(ItemCategory::Armor, 0) == 50);
    assert(InventoryRules::ItemValue(ItemCategory::Armor, 99) == 600);
    assert(InventoryRules::BuyPrice(0) == 0);
    assert(InventoryRules::SaleValue(50, 0, 0.3f) == 0);
    // Venda de pilha: 3 materiais = round(60 x 0,3) = 18.
    assert(InventoryRules::SaleValue(20, 3, InventoryRules::kBaseSellMultiplier) == 18);
}

} // namespace

int main() {
    InventoryUIState state{};
    state.items = {
        ItemDefinition{1, "weapon", ItemCategory::Weapon},
        ItemDefinition{2, "consumable", ItemCategory::Consumable},
        ItemDefinition{3, "material", ItemCategory::Material}
    };

    assert(InventoryRules::ItemCategoryFromId(state, 1) == ItemCategory::Weapon);
    assert(InventoryRules::ItemCategoryFromId(state, 999) == ItemCategory::None);
    assert(!InventoryRules::IsStackableCategory(ItemCategory::Weapon));
    assert(InventoryRules::IsStackableCategory(ItemCategory::Consumable));
    assert(InventoryRules::IsStackableCategory(ItemCategory::Material));
    assert(InventoryRules::MaxStackForCategory(ItemCategory::Consumable) == 10);
    assert(InventoryRules::MaxStackForCategory(ItemCategory::Material) == 99);
    assert(InventoryRules::MaxStackForCategory(ItemCategory::Armor) == 1);

    TestValueBuyAndSellTable();

    return 0;
}
