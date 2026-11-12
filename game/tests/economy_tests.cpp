// Testes headless da economia que precisam do catálogo real de itens (InitializeInventoryUIDummyData).
// Não abrem janela: só montam o estado do inventário e do jogador.
#include "inventory_rules.h"
#include "player.h"
#include "ui_inventory.h"

#include <cassert>
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

} // namespace

int main() {
    TestKnightStartsWithoutCharacterBonuses();
    TestStartingStateWithoutDebug();
    TestStartingStateWithDebug();
    std::cout << "Economy tests passed\n";
    return 0;
}
