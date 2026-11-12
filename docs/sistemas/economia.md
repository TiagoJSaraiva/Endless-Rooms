# Economia

## Propósito
Define quanto vale cada item, como loja, baú e forja sorteiam e cobram, e o modo debug que libera o Kit do Testador. As metas vêm de `balanceamento.md`; esta nota descreve o que o código faz.

## Componentes
- `inventory_rules.h/.cpp`: regras puras (sem janela): tabela de valores, compra, venda, chance de forja, sorteio de loja e baú.
- `ui_inventory.cpp`: monta o catálogo (`InitializeInventoryUIDummyData`), chama as regras e aplica o resultado ao estado (`RollShopInventory`, `EnsureCommonChestLoot`, `RefreshForgeChance`, `CalculateSaleValue`).
- `debug_config.h/.cpp`: lê `debug.txt` (namespace `DebugConfig`).
- `main.cpp`: lê `debug.txt` junto de `biome.txt` e passa o resultado a `InitializeInventoryUIDummyData(state, debugMode)` a cada nova run.

## Regras

### Valor, compra e venda
Tabela única em `InventoryRules::ItemValue(categoria, raridade)` (`inventory_rules.cpp`): material 20, poção 35 e equipamento (arma, armadura, resultado de forja) por raridade 1 a 5: 50, 150, 250, 370, 600. Não há valor por item: `ItemDefinition::value` vem sempre da tabela.

| Categoria / raridade | Valor | Compra | Venda |
|---|---:|---:|---:|
| Material | 20 | 25 | 6 |
| Poção de cura | 35 | 45 | 11 |
| Equipamento 1 | 50 | 65 | 15 |
| Equipamento 2 | 150 | 195 | 45 |
| Equipamento 3 | 250 | 325 | 75 |
| Equipamento 4 | 370 | 480 | 111 |
| Equipamento 5 | 600 | 780 | 180 |

- Compra (`BuyPrice`): `valor × kBuyMultiplier (1,3)` arredondado ao múltiplo de `kBuyPriceStep (5)` mais próximo. A loja grava o preço no momento do sorteio.
- Venda (`SaleValue`): `round(valor × quantidade × multiplicador)`. O multiplicador vive em `InventoryUIState::sellPriceMultiplier`, que começa em `kBaseSellMultiplier (0,3)` (a meta-progressão poderá escalar).

### Loja
`RollShopSlots` (`inventory_rules.cpp`), 4 slots (`kShopSlotCount`), sem repetir item:
1. Slot garantido de consumível (hoje, a Poção de cura), estoque de `kShopConsumableMinStock` a `kShopConsumableMaxStock` (2 a 4).
2. Slot premium: item de raridade >= `kShopMinHighRarity` (2), pelo peso de raridade.
3. Dois slots pelo peso de raridade entre os itens restantes.

Os demais itens têm estoque 1. A semente é `ShopInstance::CurrentSeed()`: o mesmo estado da loja gera a mesma vitrine e o re-roll muda a semente.

### Baú comum
`RollChestLoot`: de `kChestMinSlots` a `kChestMaxSlots` (2 a 3) itens distintos pelo peso de raridade. Poções: 1 a 2 unidades; materiais: 2 a 6; demais itens: 1. Nunca moedas (moeda não é item). `EnsureCommonChestLoot` escolhe em quais slots do baú cada item cai, pela semente do baú.

### Peso de raridade (loja e baú)
`kRarityWeights`: raridade 1: 60, 2: 25, 3: 10, 4: 4, 5: 1. Sorteia-se a raridade e depois um item uniforme dentro dela. Raridade sem item disponível sai do sorteio (os pesos restantes são renormalizados). Itens com raridade fora da tabela só aparecem se nenhum candidato estiver na tabela.

### Forja
- Chance = `investido / (kForgeFullChanceValueFraction (0,8) × valor do item produzido)`, limitada a [0, 1]. Para equipamento comum (valor 50), 40 moedas dão 100%.
- Sem receita válida para os dois insumos, 0%.
- Falhar quebra a bigorna (comportamento inalterado). Receitas: as 5 atuais, todas com produto de raridade 1 (`forgeRecipes`, em `InitializeInventoryUIDummyData`).

### Modo debug
- `game/debug.txt` (não versionado), lido na abertura do jogo. Valores que ligam: `true`, `1`, `on`, `yes` (sem diferenciar maiúsculas); ausente, vazio ou qualquer outro valor = desligado.
- Ligado: Kit do Testador nos 4 slots de equipamento e elegível em loja e baú.
- Desligado: equipamento inicial vazio (só Espada Curta + Arco Simples nas mãos); o Kit não sai de loja nem de baú e não está em nenhuma receita.
- O item é marcado `ItemDefinition::debugOnly`; `IsLootEligible` aplica a regra. A flag fica em `InventoryUIState::debugMode`.

### Jogador inicial
`CreateKnightCharacter` não tem bônus de personagem: 100 PV, Defesa 0, mitigação 0%. Bônus de personagem (pós-demo) precisarão de campo próprio, pois `SyncEquipmentBonuses` sobrescreve `equipmentBonuses`.

## Onde ajustar
| O quê | Onde |
|---|---|
| Valor por categoria/raridade | `kMaterialValue`, `kConsumableValue`, `kEquipmentValueByRarity` em `inventory_rules.cpp` |
| Multiplicador e arredondamento de compra | `kBuyMultiplier`, `kBuyPriceStep` em `inventory_rules.h` |
| Taxa de venda | `kBaseSellMultiplier` em `inventory_rules.h` |
| Pesos de raridade | `kRarityWeights` em `inventory_rules.h` |
| Slots e estoque da loja | `kShopSlotCount`, `kShopMinHighRarity`, `kShopConsumable*Stock` em `inventory_rules.h` |
| Slots e quantidades do baú | `kChest*` em `inventory_rules.h` |
| Custo de forja | `kForgeFullChanceValueFraction` em `inventory_rules.h` |
| Receitas de forja | `forgeRecipes` em `InitializeInventoryUIDummyData` |
| Itens, raridades e atributos | `InitializeInventoryUIDummyData` (`addItem`) |

## Contratos
- Regras de sorteio recebem o catálogo e um `std::mt19937_64&`: mesma semente, mesmo resultado. Não dependem de raylib.
- `InitializeInventoryUIDummyData(state, debugMode)` zera o estado e define `debugMode`; deve ser chamado a cada nova run.
- Os valores dos itens são derivados da tabela no `addItem`; nenhum outro lugar define valor.

## Testes
- `tests/inventory_rules_tests.cpp`: tabela valor/compra/venda, custo e chance da forja, loja (formato, poção, premium, sem repetição, distribuição por raridade, Kit só no debug) e baú (slots, quantidades, distribuição, Kit só no debug) com catálogo sintético.
- `tests/economy_tests.cpp` (liga `ui_inventory.cpp`, sem janela): catálogo real, estado inicial do jogador com e sem debug, loja e baú pelo caminho de produção, chance da forja pelo valor do produto.
- `tests/debug_config_tests.cpp`: interpretação e leitura de `debug.txt`.
- Validar jogando: preços na loja e no inventário, vitrine sempre com poção, quantidade de itens do baú, custo da forja (40 moedas para 100% numa peça comum), e o modo debug com e sem `debug.txt`.
