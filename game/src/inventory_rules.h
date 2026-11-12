#pragma once

#include "ui_inventory.h"

#include <array>
#include <random>
#include <vector>

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

// --- Forja ---

// Fração do valor do item PRODUZIDO que garante 100% de chance de sucesso.
constexpr double kForgeFullChanceValueFraction = 0.8;

// Moedas necessárias para 100% de chance: 80% do valor do produto (equipamento comum, valor 50: 40).
int ForgeFullChanceCost(int productValue);
// Chance de sucesso (0 a 1) = investido / custo de 100%. Sem produto válido (valor <= 0), 0.
float ForgeSuccessChance(int investedCoins, int productValue);

// --- Loja e baú: sorteio por peso de raridade ---

// Peso de sorteio de cada raridade na loja e no baú (índice 0 = raridade 1). Raridade sem item
// disponível sai do sorteio e os pesos restantes são renormalizados.
constexpr std::array<int, 5> kRarityWeights{60, 25, 10, 4, 1};

// Itens que podem aparecer em loja e baú: id válido, categoria definida e, se for "só debug",
// apenas com o modo debug ligado.
bool IsLootEligible(const ItemDefinition& item, bool debugMode);

// Sorteia um item pelo peso de raridade (depois, uniforme dentro da raridade sorteada).
// Retorna nullptr se `candidates` estiver vazio.
const ItemDefinition* PickByRarityWeight(const std::vector<const ItemDefinition*>& candidates,
                                         std::mt19937_64& rng);

constexpr int kShopSlotCount = 4;
constexpr int kShopMinHighRarity = 2;       // O slot "premium" da loja sai de raridade >= 2
constexpr int kShopConsumableMinStock = 2;  // Estoque das poções (1 unidade para os demais itens)
constexpr int kShopConsumableMaxStock = 4;

struct ShopSlotRoll {
    int itemId{0};
    int stock{0};
};

// Monta a vitrine: 1 slot de consumível (poção) garantido, 1 slot de raridade >= 2 e o restante
// sorteado pelo peso de raridade, sem repetir item. Retorna até kShopSlotCount slots (menos se o
// catálogo for pequeno demais).
std::vector<ShopSlotRoll> RollShopSlots(const std::vector<ItemDefinition>& catalog,
                                        bool debugMode,
                                        std::mt19937_64& rng);

constexpr int kChestMinSlots = 2;
constexpr int kChestMaxSlots = 3;
constexpr int kChestConsumableMinQuantity = 1;
constexpr int kChestConsumableMaxQuantity = 2;
constexpr int kChestMaterialMinQuantity = 2;
constexpr int kChestMaterialMaxQuantity = 6;

struct ChestLootEntry {
    int itemId{0};
    int quantity{0};
};

// Sorteia o conteúdo de um baú comum: 2 a 3 itens distintos pelo peso de raridade, com quantidades
// por categoria. Moedas nunca saem de baú (não existem como item).
std::vector<ChestLootEntry> RollChestLoot(const std::vector<ItemDefinition>& catalog,
                                          bool debugMode,
                                          std::mt19937_64& rng);

} // namespace InventoryRules
