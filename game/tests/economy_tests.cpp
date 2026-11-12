// Testes headless da economia que precisam do catálogo real de itens (InitializeInventoryUIDummyData).
// Não abrem janela: só montam o estado do inventário e do jogador.
#include "inventory_rules.h"
#include "player.h"
#include "chest.h"
#include "ui_inventory.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

constexpr float kStartingHealth = 100.0f;
constexpr int kKitItemId = 159;
constexpr int kEquipmentSlotCount = 4;

bool EquipmentHasKit(const InventoryUIState& state) {
    for (int itemId : state.equipmentSlotIds) {
        if (itemId == kKitItemId) { return true; }
    }
    return false;
}

void TestKnightStartsWithoutCharacterBonuses() {
    PlayerCharacter knight = CreateKnightCharacter();
    assert(knight.derivedStats.maxHealth == kStartingHealth);
    assert(knight.currentHealth == kStartingHealth);
    assert(knight.totalAttributes.primary.defesa == 0);
    assert(knight.totalAttributes.primary.vigor == 0);
    assert(knight.totalAttributes.secondary.reducaoDano == 0.0f);
    assert(knight.derivedStats.damageMitigation == 0.0f);
}

void TestStartingStateWithoutDebug() {
    InventoryUIState state{};
    InitializeInventoryUIDummyData(state, false);

    PlayerCharacter knight = CreateKnightCharacter();
    SyncEquipmentBonuses(state, knight);
    assert(knight.derivedStats.maxHealth == kStartingHealth);
    assert(knight.totalAttributes.primary.defesa == 0);
    assert(!EquipmentHasKit(state));
    for (int slot = 0; slot < kEquipmentSlotCount; ++slot) {
        assert(state.equipmentSlotIds[slot] == 0);
    }
    assert(state.weaponSlotIds.size() >= 2);
    assert(state.weaponSlotIds[0] == 1 && state.weaponSlotIds[1] == 3); // Espada Curta + Arco Simples
    assert(state.coins == 0);
}

void TestStartingStateWithDebug() {
    InventoryUIState state{};
    InitializeInventoryUIDummyData(state, true);
    for (int slot = 0; slot < kEquipmentSlotCount; ++slot) {
        assert(state.equipmentSlotIds[slot] == kKitItemId);
    }
    PlayerCharacter knight = CreateKnightCharacter();
    SyncEquipmentBonuses(state, knight);
    assert(knight.totalAttributes.primary.defesa == 20);
}

constexpr int kPotionItemId = 150;
constexpr int kCommonEquipmentValue = 50;
constexpr int kCommonEquipmentBuyPrice = 65;
constexpr int kSampleSeeds = 3000;

void TestCatalogValues() {
    InventoryUIState state{};
    InitializeInventoryUIDummyData(state, false);
    const ItemDefinition* potion = GetItemDefinition(state, kPotionItemId);
    const ItemDefinition* tunic = GetItemDefinition(state, 110);   // Túnica (equipamento comum)
    const ItemDefinition* leather = GetItemDefinition(state, 101); // Tira de couro (material)
    assert(potion != nullptr && tunic != nullptr && leather != nullptr);
    assert(potion->value == 35);
    assert(tunic->value == kCommonEquipmentValue);
    assert(leather->value == 20);
    assert(InventoryRules::BuyPrice(tunic->value) == kCommonEquipmentBuyPrice);
    assert(state.sellPriceMultiplier == InventoryRules::kBaseSellMultiplier);
}

// A loja real, sorteada pelo caminho de produção (semente da loja), sempre tem poção e nunca o Kit sem debug.
void TestShopThroughInventoryState() {
    for (bool debug : {false, true}) {
        InventoryUIState state{};
        InitializeInventoryUIDummyData(state, debug);
        bool kitSeen = false;
        for (int seed = 1; seed <= kSampleSeeds; ++seed) {
            ShopInstance shop{};
            shop.baseSeed = static_cast<std::uint64_t>(seed);
            RollShopInventory(state, &shop);

            assert(state.shopItemIds.size() >= static_cast<size_t>(InventoryRules::kShopSlotCount));
            assert(state.shopItemIds[0] == kPotionItemId);
            assert(state.shopStock[0] >= 2 && state.shopStock[0] <= 4);
            assert(state.shopPrices[0] == InventoryRules::BuyPrice(35));
            const ItemDefinition* premium = GetItemDefinition(state, state.shopItemIds[1]);
            assert(premium != nullptr && premium->rarity >= 2);
            assert(shop.items.size() == static_cast<size_t>(InventoryRules::kShopSlotCount));

            for (int slot = 0; slot < InventoryRules::kShopSlotCount; ++slot) {
                kitSeen = kitSeen || state.shopItemIds[slot] == kKitItemId;
                for (int other = slot + 1; other < InventoryRules::kShopSlotCount; ++other) {
                    assert(state.shopItemIds[slot] != state.shopItemIds[other]);
                }
            }
        }
        assert(kitSeen == debug);
    }
}

// O baú real (EnsureCommonChestLoot) preenche 2 a 3 slots e só inclui o Kit no modo debug.
void TestChestThroughInventoryState() {
    for (bool debug : {false, true}) {
        InventoryUIState state{};
        InitializeInventoryUIDummyData(state, debug);
        bool kitSeen = false;
        for (int seed = 1; seed <= kSampleSeeds; ++seed) {
            CommonChest chest(0.0f, 0.0f, 100.0f, Rectangle{0.0f, 0.0f, 10.0f, 10.0f}, 8, static_cast<std::uint64_t>(seed));
            EnsureCommonChestLoot(chest, state);
            int filled = 0;
            for (const Chest::Slot& slot : chest.GetSlots()) {
                if (slot.itemId == 0) { continue; }
                ++filled;
                kitSeen = kitSeen || slot.itemId == kKitItemId;
                assert(slot.quantity >= 1);
            }
            assert(filled >= 2 && filled <= 3);
        }
        assert(kitSeen == debug);
    }
}

// A chance da forja usa o valor do item produzido pela receita (Farrapo + Farrapo = Túnica, valor 50).
void TestForgeChanceUsesProductValue() {
    InventoryUIState state{};
    InitializeInventoryUIDummyData(state, false);

    constexpr int kRagId = 100;
    constexpr int kLeatherId = 101;
    constexpr int kBronzeId = 102;
    constexpr int kBronzeHelmetId = 112;
    ForgeInstance forge{};
    forge.contents.inputs[0] = ForgeInstance::Slot{kRagId, 1};
    forge.contents.inputs[1] = ForgeInstance::Slot{kRagId, 1};

    state.forgeBaseCost = 40;
    LoadForgeContents(state, forge);
    assert(std::abs(state.forgeSuccessChance - 1.0f) < 1e-6f);

    state.forgeBaseCost = 20;
    LoadForgeContents(state, forge);
    assert(std::abs(state.forgeSuccessChance - 0.5f) < 1e-6f);

    // Sem receita para a combinação (Tira de couro + Elmo de bronze): 0%.
    forge.contents.inputs[0] = ForgeInstance::Slot{kLeatherId, 1};
    forge.contents.inputs[1] = ForgeInstance::Slot{kBronzeHelmetId, 1};
    state.forgeBaseCost = 9999;
    LoadForgeContents(state, forge);
    assert(state.forgeSuccessChance == 0.0f);

    // Receita existente com investimento zero: 0%.
    forge.contents.inputs[0] = ForgeInstance::Slot{kLeatherId, 1};
    forge.contents.inputs[1] = ForgeInstance::Slot{kBronzeId, 1};
    state.forgeBaseCost = 0;
    LoadForgeContents(state, forge);
    assert(state.forgeSuccessChance == 0.0f);
}

} // namespace

int main() {
    TestKnightStartsWithoutCharacterBonuses();
    TestStartingStateWithoutDebug();
    TestStartingStateWithDebug();
    TestCatalogValues();
    TestForgeChanceUsesProductValue();
    TestShopThroughInventoryState();
    TestChestThroughInventoryState();
    std::cout << "Economy tests passed\n";
    return 0;
}
