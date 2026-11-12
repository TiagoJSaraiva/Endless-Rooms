#include "inventory_rules.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

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

// Catálogo sintético com todas as raridades, uma poção, materiais e um item só de debug.
constexpr int kPotionId = 100;
constexpr int kKitId = 900;
constexpr int kSamples = 20000;
constexpr double kDistributionTolerance = 0.02;

std::vector<ItemDefinition> MakeCatalog() {
    std::vector<ItemDefinition> catalog;
    auto add = [&](int id, ItemCategory category, int rarity) {
        ItemDefinition def{};
        def.id = id;
        def.name = "item" + std::to_string(id);
        def.category = category;
        def.rarity = rarity;
        def.value = InventoryRules::ItemValue(category, rarity);
        catalog.push_back(def);
    };
    add(kPotionId, ItemCategory::Consumable, 1);
    add(101, ItemCategory::Material, 1);
    add(102, ItemCategory::Material, 1);
    add(110, ItemCategory::Armor, 1);
    add(111, ItemCategory::Armor, 1);
    add(1, ItemCategory::Weapon, 2);
    add(2, ItemCategory::Weapon, 2);
    add(3, ItemCategory::Weapon, 3);
    add(4, ItemCategory::Weapon, 3);
    add(5, ItemCategory::Armor, 4);
    add(6, ItemCategory::Weapon, 5);
    add(kKitId, ItemCategory::Armor, 5);
    catalog.back().debugOnly = true;
    return catalog;
}

const ItemDefinition* Find(const std::vector<ItemDefinition>& catalog, int id) {
    for (const ItemDefinition& def : catalog) {
        if (def.id == id) { return &def; }
    }
    return nullptr;
}

int RarityOf(const std::vector<ItemDefinition>& catalog, int id) {
    const ItemDefinition* def = Find(catalog, id);
    return def != nullptr ? def->rarity : 0;
}

void ExpectRarityDistribution(const std::map<int, int>& counts, int total, const std::array<double, 5>& expected) {
    for (int rarity = 1; rarity <= 5; ++rarity) {
        const auto it = counts.find(rarity);
        const double observed = (it == counts.end()) ? 0.0 : static_cast<double>(it->second) / total;
        assert(std::abs(observed - expected[static_cast<size_t>(rarity - 1)]) < kDistributionTolerance);
    }
}

void TestShopShape() {
    const std::vector<ItemDefinition> catalog = MakeCatalog();
    for (bool debug : {false, true}) {
        for (std::uint64_t seed = 1; seed <= 2000; ++seed) {
            std::mt19937_64 rng(seed);
            const auto slots = InventoryRules::RollShopSlots(catalog, debug, rng);
            assert(static_cast<int>(slots.size()) == InventoryRules::kShopSlotCount);

            // Slot 0: poção com estoque 2 a 4; slot 1: raridade >= 2; sem repetição.
            assert(slots[0].itemId == kPotionId);
            assert(slots[0].stock >= InventoryRules::kShopConsumableMinStock);
            assert(slots[0].stock <= InventoryRules::kShopConsumableMaxStock);
            assert(RarityOf(catalog, slots[1].itemId) >= 2);

            std::set<int> unique;
            bool hasKit = false;
            for (const auto& slot : slots) {
                unique.insert(slot.itemId);
                hasKit = hasKit || slot.itemId == kKitId;
                if (slot.itemId != kPotionId) { assert(slot.stock == 1); }
            }
            assert(unique.size() == slots.size());
            if (!debug) { assert(!hasKit); }
        }
    }
}

void TestShopSeedIsDeterministicAndVaries() {
    const std::vector<ItemDefinition> catalog = MakeCatalog();
    std::mt19937_64 a(42);
    std::mt19937_64 b(42);
    const auto first = InventoryRules::RollShopSlots(catalog, false, a);
    const auto second = InventoryRules::RollShopSlots(catalog, false, b);
    assert(first.size() == second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        assert(first[i].itemId == second[i].itemId && first[i].stock == second[i].stock);
    }

    std::set<int> premiumItems;
    for (std::uint64_t seed = 1; seed <= 200; ++seed) {
        std::mt19937_64 rng(seed);
        premiumItems.insert(InventoryRules::RollShopSlots(catalog, false, rng)[1].itemId);
    }
    assert(premiumItems.size() > 1);
}

void TestShopPremiumSlotDistribution() {
    const std::vector<ItemDefinition> catalog = MakeCatalog();
    std::map<int, int> counts;
    for (int seed = 1; seed <= kSamples; ++seed) {
        std::mt19937_64 rng(static_cast<std::uint64_t>(seed));
        const auto slots = InventoryRules::RollShopSlots(catalog, false, rng);
        ++counts[RarityOf(catalog, slots[1].itemId)];
    }
    // Pesos 25/10/4/1 renormalizados sobre as raridades >= 2 (total 40).
    ExpectRarityDistribution(counts, kSamples, {0.0, 25.0 / 40.0, 10.0 / 40.0, 4.0 / 40.0, 1.0 / 40.0});
}

void TestShopDebugIncludesKit() {
    const std::vector<ItemDefinition> catalog = MakeCatalog();
    bool seen = false;
    for (std::uint64_t seed = 1; seed <= 20000 && !seen; ++seed) {
        std::mt19937_64 rng(seed);
        for (const auto& slot : InventoryRules::RollShopSlots(catalog, true, rng)) {
            seen = seen || slot.itemId == kKitId;
        }
    }
    assert(seen);
}

void TestShopWithSmallCatalog() {
    std::vector<ItemDefinition> catalog = MakeCatalog();
    catalog.resize(2); // poção + 1 material
    std::mt19937_64 rng(7);
    const auto slots = InventoryRules::RollShopSlots(catalog, false, rng);
    assert(slots.size() == 2);
    assert(slots[0].itemId == kPotionId);
}

void TestChestLoot() {
    const std::vector<ItemDefinition> catalog = MakeCatalog();
    std::map<int, int> firstRarityCounts;
    std::map<int, int> slotCountHistogram;
    for (int seed = 1; seed <= kSamples; ++seed) {
        std::mt19937_64 rng(static_cast<std::uint64_t>(seed));
        const auto loot = InventoryRules::RollChestLoot(catalog, false, rng);
        ++slotCountHistogram[static_cast<int>(loot.size())];
        assert(static_cast<int>(loot.size()) >= InventoryRules::kChestMinSlots);
        assert(static_cast<int>(loot.size()) <= InventoryRules::kChestMaxSlots);

        std::set<int> unique;
        for (const auto& entry : loot) {
            unique.insert(entry.itemId);
            assert(entry.itemId != kKitId);
            const ItemDefinition* def = Find(catalog, entry.itemId);
            assert(def != nullptr);
            if (def->category == ItemCategory::Consumable) {
                assert(entry.quantity >= InventoryRules::kChestConsumableMinQuantity);
                assert(entry.quantity <= InventoryRules::kChestConsumableMaxQuantity);
            } else if (def->category == ItemCategory::Material) {
                assert(entry.quantity >= InventoryRules::kChestMaterialMinQuantity);
                assert(entry.quantity <= InventoryRules::kChestMaterialMaxQuantity);
            } else {
                assert(entry.quantity == 1);
            }
        }
        assert(unique.size() == loot.size());
        ++firstRarityCounts[RarityOf(catalog, loot[0].itemId)];
    }
    assert(slotCountHistogram[2] > 0 && slotCountHistogram[3] > 0);
    // O primeiro item sai direto do peso de raridade 60/25/10/4/1.
    ExpectRarityDistribution(firstRarityCounts, kSamples, {0.60, 0.25, 0.10, 0.04, 0.01});
}

void TestChestDebugIncludesKit() {
    const std::vector<ItemDefinition> catalog = MakeCatalog();
    bool seen = false;
    for (std::uint64_t seed = 1; seed <= 20000 && !seen; ++seed) {
        std::mt19937_64 rng(seed);
        for (const auto& entry : InventoryRules::RollChestLoot(catalog, true, rng)) {
            seen = seen || entry.itemId == kKitId;
        }
    }
    assert(seen);
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
    TestShopShape();
    TestShopSeedIsDeterministicAndVaries();
    TestShopPremiumSlotDistribution();
    TestShopDebugIncludesKit();
    TestShopWithSmallCatalog();
    TestChestLoot();
    TestChestDebugIncludesKit();

    return 0;
}
