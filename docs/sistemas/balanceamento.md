# Balanceamento da demo

**Status: proposta (aguardando aprovação do desenvolvedor).**

Nada aqui altera código ou valores do jogo. Depois de aprovadas, as metas orientam a subtarefa 1.7 (atributos dos inimigos) e a 1.9 (aplicação e testes de faixa). Cenário: só a Dungeon, sem boss, a run termina na morte, personagem único (cavaleiro).

Premissas das contas: jogador com 124 de vida e 16% de mitigação (ver dúvida 1), sem fonte de atributos de ataque (`forca`, `foco` etc. só vêm do Kit do Testador, que sai da demo), todos os números "por segundo" sem contar mira e esquiva.

## Valores atuais

### Jogador (`game/src/player.cpp`, `player.h`)

| Item | Valor |
|---|---|
| Vida | 100 + 12 por ponto de Vigor |
| Velocidade | 250 px/s, +3% por ponto de Velocidade (teto ×1,9); tile = 64 px |
| Mitigação (Defesa) | `0,6 · ln(def+1) / ln(61)`: def 2 = 16%, 5 = 26%, 10 = 35%, 20 = 44% (teto 60%) |
| Redução fixa | `reducaoDano` subtraída do golpe antes da mitigação; todo golpe causa no mínimo 1 (`main.cpp`, ~l.1613) |
| Esquiva | 1% por ponto de Desvio, teto 60% |
| Cavaleiro | `equipmentBonuses`: Defesa 2, Vigor 2, reducaoDano 5 (sobrescritos na primeira sincronização, ver inconsistências) |
| Kit inicial real | `InitializeInventoryUIDummyData` equipa **4× Kit do Testador** (cada: Vigor 3, Defesa 5, Força 5, Foco 5, Vampirismo 10) e as armas Espada Curta + Arco Simples. Resultado: 244 de vida, 44% de mitigação, Força/Foco 20 |
| Moedas iniciais | 0 (`ui_inventory.cpp`, `state.coins = 0`) |
| Inventário | 24 slots; poção empilha 10, material 99; 2 slots de arma (as duas mãos atacam) |
| Cura | Poção de cura: +50 de vida, sem cooldown |

### Armas (`game/src/weapon_blueprints.cpp`)

DPS = dano × ataques/s × (1 + chance crítica × (mult − 1)), contando só os passivos da própria arma e atributos zerados. Armas de golpe múltiplo estão em "alvo único".

| Arma | Raridade | Dano | Atq/s | Crítico (chance, mult) | DPS atual | Observação |
|---|---:|---:|---:|---|---:|---|
| Espada Curta | 2 | 12 | 1,4 (+0,12/Destreza) → 1,52 | 8% (+0,6%/Let.), ×1,3 | 18,7 | arco 110°, alcance 110 px |
| Machadinha | 2 | 16 | 1,1 → 1,1 | 10%, ×1,45 | 18,5 | estocada 118 px, 0,47 s |
| Arco Simples | 2 | 8,5 | 2,2 → 2,48 | 12%, ×1,45 | 22,6 | flecha 560 px/s, 860 px |
| Cajado de Carvalho | 3 | 2 por tick | 1,6 | 10%, ×1,55 | ~10 | ~3 ticks por pulso; escala 1,9/Conhecimento |
| Broquel | 3 | 10 | 0,6 | 5%, ×1,2 | 6,1 | +5 Defesa passiva |
| Espada Rúnica | 5 | 20 | 0,55 | 14%, ×1,65 | 25 a 37 | giro de 1,2 s, até 3 acertos (0,4 s por alvo) |

Escalonamento por atributo de ataque (ex.: Espada Curta +1,5 por Força) existe, mas nenhum item da demo concede esses atributos além do Kit do Testador. O atributo `poder` aparece na UI como multiplicador de dano mas não entra no cálculo (ver inconsistências).

### Inimigos da Dungeon atuais (`game/src/enemy_spawner.cpp`, `enemy.h`, `enemy_common.cpp`)

`MakeConfig(id, nome, bioma, maxHealth, speed, spawnRate, collisionRadius)`: vida máxima; velocidade em px/s; `spawnRate` é o **peso** relativo do sorteio (`discrete_distribution`); `collisionRadius` em px. `range` (no template) é a distância a que o inimigo para de andar e começa a atacar. O ataque usa o blueprint de arma: dano base, cadência e crítico da arma, sem escalar por atributo.

| Inimigo | Vida | Vel. | Peso | Raio | Range | Arma | DPS bruto | Chance (peso) |
|---|---:|---:|---:|---:|---:|---|---:|---:|
| `dungeon_ranged` | 27,5 | 85 | 1,0 | 22 | 560 | Cajado | ~10 | 43% |
| `dungeon_melee` | 47,5 | 92,5 | 1,3 | 26 | 150 | Machadinha | 17,6 | 57% |

### Salas e spawn

| Item | Valor | Fonte |
|---|---|---|
| Tamanho normal | lados pares de 10 a 20 tiles (média 225 tiles) | `room_manager.cpp` |
| Salas especiais | 8×8 tiles | idem |
| Corredor | 4 ou 6 tiles | `MIN/MAX_CORRIDOR_LENGTH_TILES` |
| Portas | até 4 por sala; cada porta gera a sala vizinha na hora (~2 a 3 salas novas por sala visitada, a confirmar por simulação) | `room_manager.cpp` |
| Inimigos por sala normal | `round(área × 0,03)`, mínimo 1: média 6,8, de 3 a 12 | `enemy_spawner.cpp` |
| Props de chão | orçamento de 8% a 15% da área; pesos: vaso 6, barril 1, bone-jail 3, mesa 1 | `prop.cpp`, `prop_rules.cpp` |
| Tipos de sala | hoje Normal 79−0,5·n (cai com salas sem boss), Forge/Shop/Chest 5, Boss 1+0,5·n | `PickRoomType` |

### Bolsas (`reward_pouch.cpp`, `docs/sistemas/reward-pouches.md`)

| Bolsa | Moedas | Itens |
|---|---|---|
| small | 12–20 (média 16) | 20% de 1 tira de couro |
| medium | 24–36 (30) | 50% de 1–2 couro; 20% de 1 lingote |
| big | 48–72 (60) | 100% de 2–3 couro; 50% de 1–2 lingotes |

Chances atuais: inimigo da Dungeon 50/20/5% (small/medium/big, 25% nada), média de 17 moedas por inimigo; vaso 20% de small (3,2 moedas por vaso). Média atual: 6,8 inimigos × 17 = ~115 moedas por sala normal, mais vasos.

### Preços, loja, forja e baú (`inventory_rules.cpp`, `ui_inventory.cpp`)

| Regra | Valor atual |
|---|---|
| Valor do item | `20 × raridade + baseValue` |
| Compra na loja | `round(valor × 1,3)` |
| Venda ao jogador-loja | `round(valor × 0,2)` (≈15% do preço de compra) |
| Loja | 4 slots sorteados **uniformemente entre os 19 itens**, sem repetição; equipamento estoque 1, poção 2–7; 1 re-roll |
| Baú comum | 4 slots, enche 1 a 3 com itens sorteados uniformemente entre os 19; material 2–6 un., poção 1–3 un.; nunca moedas |
| Forja | só 5 receitas, todas de raridade 1; chance de sucesso = `moedas investidas / (valor A + valor B)`; falha **quebra a bigorna** da sala e perde as moedas (insumos ficam) |
| Teto da UI de forja | 9999 moedas |

Valores de loja e venda dos itens atuais (compra / venda):

| Item | Raridade | Valor | Compra | Venda |
|---|---:|---:|---:|---:|
| Farrapo, Tira de couro, Lingote | 1 | 40 | 52 | 8 |
| Equipamentos comuns (Túnica, Calçados, Elmo, Colete, Espaldeira, Pingente) | 1 | 50 | 65 | 10 |
| Poção de cura | 1 | 40 | 52 | 8 |
| Machadinha / Espada Curta / Arco | 2 | 110 / 120 / 140 | 143 / 156 / 182 | 22 / 24 / 28 |
| Broquel / Cajado | 3 | 150 / 160 | 195 / 208 | 30 / 32 |
| Amuleto do Sorrateiro / Couraça do Caído | 4 | 200 / 220 | 260 / 286 | 40 / 44 |
| Espada Rúnica | 5 | 420 | 546 | 84 |
| Kit do Testador | 5 | 105 | 137 | 21 |

## Metas propostas

### Ritmo

| Meta | Valor | Justificativa |
|---|---|---|
| Duração de uma run típica | 12 a 15 min | Demo curta de itch.io/navegador; a pessoa vê loja, forja e baú e ainda morre dentro de uma sessão |
| Salas visitadas | 12 a 15: ~9 a 11 normais + 3 a 4 especiais | Sala normal ≈ 60 a 70 s (corredor 6 s, combate 35 a 45 s, vasos e saque 15 s); especial ≈ 40 s |
| Inimigos por sala normal | `round(área × 0,02)`, mín. 2: média 4,5, de 2 a 8 | Hoje 6,8 de média com inimigos de 140 de vida tornaria a sala cansativa |
| Vida perdida por sala normal | ~30% da vida máxima inicial (~40 PV) | Com ~400 PV efetivos na run (124 + ~5 poções + armadura) dá ~10 salas normais |
| Fim da run | 90% das runs terminam antes de 20 min | Sem boss, só morre se a dificuldade subir; ver dúvida 3 |

### Combate: jogador

| Meta | Valor | Justificativa |
|---|---|---|
| Vida inicial | 124 (100 + Vigor 2) | É o que o código do cavaleiro pretende; sem o Kit do Testador |
| Mitigação inicial | 16% (Defesa 2), sem redução fixa | Redução fixa de 5 anula os golpes pequenos do Victim; preferível só a percentual |
| Velocidade | 250 px/s, sem mudar | Mais rápido que qualquer inimigo (máx. 170): fuga e kite sempre possíveis |
| Vida no fim da run | 150 a 190 | Equipamento comum soma Vigor 2 a 3 e Defesa 1 a 2 por peça |
| DPS por tier de arma | raridade 2: ~20; raridade 3: ~30; raridade 4: ~40; raridade 5: ~50 | Hoje as armas de raridade 2 já estão em 18,5 a 22,6; Cajado (~10), Broquel (6) e Rúnica (25 a 37) fogem da curva |

Ajustes sugeridos às armas na 1.9 para entrar na curva (só os números): Cajado dano base 2 → 6 (~30 DPS); Broquel dano 10 → 14 e 0,6 → 1,0 atq/s (~15 DPS; função defensiva por +5 Defesa); Espada Rúnica dano 20 → 26 (~40 DPS, até 3 acertos por giro).

### Combate: inimigos novos

Dano recebido considera 16% de mitigação (×0,84). "Arma inicial" = 20 DPS (Espada Curta); "tier médio" = 30 DPS (Cajado reajustado). TTK = vida ÷ DPS, sem tempo de mira e aproximação.

| Meta | Victim | Guard | Executioner |
|---|---:|---:|---:|
| Vida | 20 | 45 | 130 |
| TTK arma inicial | 1,0 s | 2,3 s | 6,5 s |
| TTK tier médio | 0,7 s | 1,5 s | 4,3 s |
| Golpes da Espada Curta para matar | 2 | 4 | 11 |
| Dano por golpe (bruto) | 6 | 12 | 20 por acerto (até 2 por giro, 40) |
| DPS recebido (bruto, se todos acertam) | 3,8 | 6,7 | 10 |
| DPS recebido efetivo | 3,2 | 5,6 | 8,4 |
| Golpes para matar o jogador (124 PV) | ~25 | ~12 | ~7 acertos (~4 giros) |
| Carga antes do ataque | nenhuma | 0,35 s | 0,8 s |
| Peso de spawn | 1,0 | 1,0 | 0,35 |

Por que:
- Victim é isca de pressão à distância: TTK de 1 s e dano baixo; o perigo é acumular vários.
- Guard: 0,35 s de carga dá reação só para quem está atento, e o intervalo de 1,8 s deixa espaço para contra-atacar.
- Executioner: 0,8 s de carga a 250 px/s permite sair do raio do giro (~130 px) caminhando (≥ 200 px); morre devagar de propósito.
- Spawn: pesos 1,0/1,0/0,35 dão 42%/42%/15% por inimigo; com 4,5 inimigos por sala nascem em média 0,7 Executioner por sala. Teto proposto: 2 Executioners por sala.
- Mistura típica de sala (4,5 inimigos): 1,9 Victim, 1,9 Guard, 0,7 Executioner = ~220 de vida total (~11 s de DPS contínuo, 35 a 45 s com deslocamento).

### Economia

Moedas por origem (meta). Tamanhos de bolsa não mudam; mudam as chances por inimigo (valores esperados na tabela).

| Origem | small | medium | big | Nada | Moedas esperadas |
|---|---:|---:|---:|---:|---:|
| Victim | 35% | 5% | 0% | 60% | 7,1 |
| Guard | 35% | 20% | 0% | 45% | 11,6 |
| Executioner | 0% | 40% | 40% | 20% | 36 |
| Vaso | 20% | 0% | 0% | 80% | 3,2 (sem mudar) |

| Meta | Valor |
|---|---|
| Moedas por inimigo (média pelos pesos) | ~13 |
| Moedas por sala normal | ~70 (60 de inimigos + ~10 de vasos: ~3 vasos quebrados por sala; estimativa a medir) |
| Moedas por run | ~650 (9 a 11 salas normais), mais ~50 vendendo saque |
| Itens de bolsa | sem mudança (couro e lingote alimentam a forja) |
| Salas especiais por run | em média ≥ 1,5 de cada tipo visível; ≥ 70% das runs com ao menos uma loja visível até a 10ª sala visitada |

Tabela de valores por raridade (uma única tabela de dados; `baseValue = valor-alvo − 20 × raridade`). Compra = `valor × 1,3` (arredondada a 5); venda = `valor × 0,2`. Margem compra/venda de 6,5 para 1: vender saque rende pouco e não vira fonte de renda.

| Categoria / raridade | Valor-alvo | Compra | Venda | Hoje (compra) |
|---|---:|---:|---:|---:|
| Material (1) | 20 | 25 | 4 | 52 |
| Poção de cura (1) | 35 | 45 | 7 | 52 |
| Equipamento comum (1) | 50 | 65 | 10 | 65 |
| Incomum (2) | 150 | 195 | 30 | 143 a 182 |
| Raro (3) | 250 | 325 | 50 | 195 a 208 |
| Épico (4) | 370 | 480 | 74 | 260 a 286 |
| Lendário (5) | 600 | 780 | 120 | 546 |

O que cabe numa run (~650 moedas, ~1,6 lojas visíveis):
- Cura: 1 a 3 poções por loja, 45 cada. Curar é ~35% do gasto (≈0,8 poção por sala perdida).
- 1 arma ou peça de raridade 2 a 3 (195 a 325).
- Raridade 4 só juntando moedas ou achando em baú; raridade 5 (780) só em baú, com ~1% de chance por slot.

Regras de loja e baú propostas (implementar na 1.9):
- Loja: 4 slots; **1 slot sempre poção** (estoque 2 a 4), 1 slot sempre arma ou equipamento de raridade ≥ 2, 2 slots sorteados por peso de raridade. Nunca oferecer Kit do Testador.
- Baú: 2 a 3 slots (hoje 1 a 3), sorteio por peso de raridade 1: 60%, 2: 25%, 3: 10%, 4: 4%, 5: 1%; poções 1 a 2; sem Kit do Testador. Valor esperado ≈ 250 de valor-tabela (≈ 40% da renda de uma run).
- Forja: o custo de 100% é a soma dos valores dos insumos: 20 + 20 = 40 moedas (≈ 60% do preço de compra do equipamento comum, 65), mais os materiais. Chance linear mantida (20 moedas = 50%). Falhar quebra a bigorna: é o risco que torna investir menos que 100% uma escolha. Receitas: manter as 5 de raridade 1 na demo.

### Pesos de tipos de sala

Proposta: manter Normal 79, Forge 5, Shop 5, Chest 5 (total 94: 84% / 5,3% cada).

Cálculo: cada porta gera uma sala na hora, então ~2,5 salas novas por sala visitada; em 12 visitas, ~30 salas geradas. P(ao menos uma Shop entre 30 geradas) = 1 − 0,947³⁰ ≈ 80%; esperado 1,6 de cada tipo. Atende à meta de 70% se o jogador puder escolher o caminho. Se a simulação de seeds (1.9) mostrar menos de 70% de runs com loja visível até a 10ª sala visitada, subir para Normal 70 / Shop 10 / Forge 10 / Chest 10 (P ≈ 96% em 30 salas geradas).

## Atributos propostos dos inimigos

Para a 1.7. Todos sem arma do player e sem crítico (0%). `Range` é a distância de início do ataque.

| Atributo | Victim | Guard | Executioner |
|---|---|---|---|
| Vida | 20 | 45 | 130 |
| Velocidade (px/s) | 170 | 140 | 90 |
| Raio de colisão (px) | 22 | 24 | 30 |
| Dano por golpe | 6 | 12 | 20 por acerto |
| Padrão de ataque | osso arremessado em linha reta | estocada tipo machadinha | giro 1,6 volta (480°/s, 1,2 s) |
| Alcance | 480 px de início; osso a 380 px/s, até 600 px | 120 px | 130 px de raio |
| Intervalo entre ataques (início a início) | 1,6 s | 1,8 s | 4,0 s |
| Carga (parado antes do ataque) | 0 | 0,35 s | 0,8 s, parado num frame de carga |
| Imunidade por alvo entre acertos | n/a | 0,6 s | 0,6 s (no máximo 2 acertos por giro) |
| Peso de spawn (`spawnRate`) | 1,0 | 1,0 | 0,35 (teto de 2 por sala) |
| Bolsa (small/medium/big) | 35/5/0 | 35/20/0 | 0/40/40 |
| Moedas esperadas | 7,1 | 11,6 | 36 |
| Mitigação própria | 0 | 0 | 0 |

## Como validar

Viram teste de faixa em `game/tests/` (headless, subtarefa 1.9; os testes travam os valores aprovados):

- Atributos de cada inimigo dentro das faixas da tabela acima (vida, dano, carga, intervalo, peso).
- TTK = vida ÷ DPS das armas com atributos zerados, para as armas de raridade 2 a 5 (curva de DPS por tier).
- Valor esperado de moedas por tabela de bolsa de cada inimigo, vaso e média ponderada pelos pesos de spawn.
- Preços: tabela de valor/compra/venda por raridade e razão compra/venda.
- Inimigos por sala: faixa de `round(área × 0,02)` para todos os tamanhos 10 a 20; proporção esperada de Executioner.
- Receitas e custo de forja: custo de 100% = soma dos valores dos insumos.
- Pool de loja e baú: sem Kit do Testador; loja sempre com 1 poção; distribuição por raridade em muitas seeds.
- Pesos de salas: simulação de seeds medindo salas especiais visíveis até a 10ª sala visitada.
- Jogador inicial: vida 124, mitigação 16%, sem Kit do Testador equipado.

Exigem playtest (desenvolvedor jogando; sem como testar):

- Duração de 12 a 15 min e salas visitadas por run.
- Vida perdida por sala (~30%) e quantidade de poções usadas.
- Legibilidade da carga do Guard (0,35 s) e do Executioner (0,8 s), e se o giro dá tempo de reação.
- Se o Victim é perigoso em grupo sem ser injusto; velocidade de osso.
- Sensação de progressão: o que o jogador compra e se a forja vale o risco.
- Moedas reais por sala (inclui vasos quebrados) e se a quantidade de salas especiais é suficiente.
- Escalonamento de dificuldade (dúvida 3) e se as runs terminam em tempo razoável.

## Dúvidas para o desenvolvedor

1. **Estado inicial do cavaleiro.** O código equipa 4× Kit do Testador no início real da run (244 PV, 44% de mitigação). Assumi que sai da demo e o cavaleiro começa só com Espada Curta + Arco Simples, 124 PV e Defesa 2. Os bônus `equipmentBonuses` do cavaleiro (Defesa 2, Vigor 2, reducaoDano 5) são sobrescritos na primeira sincronização de equipamento: devem virar atributos base (sem a redução fixa de 5)?
2. **Atributos de ataque.** Nenhum item da demo dá Força/Foco/Conhecimento, e `poder` não afeta o dano. Mantenho o dano das armas sem escalonamento na demo, ou algum equipamento deve concedê-los?
3. **Fim da run.** Sem boss e sem escalonamento, um jogador bom nunca morre. Aceita um escalonamento simples (ex.: +5% de vida e +3% de dano dos inimigos por sala normal concluída a partir da 6ª) ou aceita que a run termine por abandono/atrito?
4. **Teto de Executioners.** Posso limitar a 2 por sala, ou prefere só por peso?
5. **Venda.** Manter venda a 20% do valor (6,5:1) ou subir para 30%?
6. **Forja.** Manter só as 5 receitas de raridade 1 na demo, ou incluir 2 a 3 receitas de raridade 2 para a forja continuar útil depois dos primeiros minutos?
7. **Crítico inimigo.** Zero para os três, ou o Executioner pode ter crítico?
8. **Tiers de armas.** Aceita que Cajado, Broquel e Espada Rúnica sejam reajustados (números acima)? Alguma delas fica fora da demo?
9. **Pesos de salas.** Mantém 79/5/5/5 até a simulação de seeds, como proposto?

## Inconsistências encontradas no código (não corrigidas)

- `InitializeInventoryUIDummyData` (usada em toda run) equipa 4× Kit do Testador e entra no pool de loja e baú (id 159, raridade 5).
- `CreateKnightCharacter` define `equipmentBonuses`, mas `SyncEquipmentBonuses` os substitui pelos itens equipados.
- `poder` aparece na UI (`ui_inventory.cpp`, ~l.511) como `× (1 + poder/100)`, mas `WeaponState::RecalculateDerivedStats` não aplica o multiplicador.
- A loja sorteia 4 entre 19 itens: 79% das lojas não têm poção; o baú pode dar Farrapo, Kit do Testador e Espada Rúnica com a mesma chance.
- Forja: 100% de chance custa 80 moedas (dois materiais de 40), mais que o preço de compra do produto (65). Farrapo não cai de nenhuma bolsa, então 3 das 5 receitas só funcionam com Farrapo de baú ou loja.
- `dungeon_melee` tem `range` 150, mas a estocada da Machadinha alcança ~118 px; ataca fora de alcance.
- Armas de raridade 3 (Cajado ~10 DPS, Broquel ~6) rendem menos que as de raridade 2 (~19 a 23).
- `PickRoomType`: o peso Normal cai 0,5 por sala enquanto não houver boss (a outra branch está removendo).
