#pragma once

#include "ui_inventory.h"

namespace InventoryRules {

ItemCategory ItemCategoryFromId(const InventoryUIState& state, int id);
bool IsStackableCategory(ItemCategory category);
int MaxStackForCategory(ItemCategory category);

// --- Economia: valor, compra e venda (ver docs/sistemas/economia.md) ---

// Multiplicador de compra sobre o valor do item (a loja cobra 30% a mais).
constexpr double kBuyMultiplier = 1.3;
// Múltiplo em que o preço de compra é arredondado.
constexpr int kBuyPriceStep = 5;
// Fração do valor paga ao jogador na venda (InventoryUIState::sellPriceMultiplier começa nela).
constexpr float kBaseSellMultiplier = 0.3f;

// Valor base do item, vindo da tabela por categoria/raridade. Moeda de troca de todo o resto:
// compra, venda e custo de forja derivam deste número.
int ItemValue(ItemCategory category, int rarity);
// Preço cobrado pela loja: valor x 1,3 arredondado ao múltiplo de 5 mais próximo.
int BuyPrice(int itemValue);
// Quanto o jogador recebe ao vender `quantity` unidades: round(valor x quantidade x multiplicador).
int SaleValue(int itemValue, int quantity, float sellMultiplier);

} // namespace InventoryRules
