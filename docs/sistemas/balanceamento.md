# Balanceamento da demo

**Status: metas aprovadas pelo desenvolvedor em 2026-11-12.** A aplicação no código está dividida entre as subtarefas 1.7 (inimigos), 1.9a (armas), 1.9b (economia e modo debug) e 1.9c (salas), descritas em `docs/status.md`.

Cenário: só a Dungeon, sem boss, a run termina na morte, personagem único (cavaleiro).

Premissas das contas: o jogador começa com 100 de vida e 0% de mitigação (o cavaleiro não tem bônus de personagem na demo). Nenhum item da demo concede atributos de ataque (ver "Atributos de escalonamento de arma"). Todos os números "por segundo" desconsideram mira e esquiva.

## Valores de partida (código em 2026-11-12)

Referência do que existia antes da aplicação das metas. Não atualize esta seção; os valores vigentes passam a estar nas notas de cada sistema.

### Jogador (`game/src/player.cpp`, `player.h`)

| Item | Valor |
|---|---|
| Vida | 100 + 12 por ponto de Vigor |
| Velocidade | 250 px/s, +3% por ponto de Velocidade (teto ×1,9); tile = 64 px |
| Mitigação (Defesa) | `0,6 · ln(def+1) / ln(61)`: def 2 = 16%, 5 = 26%, 10 = 35%, 20 = 44% (teto 60%) |
| Redução fixa | `reducaoDano` subtraída do golpe antes da mitigação; todo golpe causa no mínimo 1 (`main.cpp`, ~l.1613) |
| Esquiva | 1% por ponto de Desvio, teto 60% |
| Cavaleiro | `equipmentBonuses`: Defesa 2, Vigor 2, reducaoDano 5 (sobrescritos na primeira sincronização de equipamento) |
| Kit inicial | `InitializeInventoryUIDummyData` equipa 4× Kit do Testador (cada: Vigor 3, Defesa 5, Força 5, Foco 5, Vampirismo 10) e as armas Espada Curta + Arco Simples |
| Moedas iniciais | 0 |
| Inventário | 24 slots; poção empilha 10, material 99; 2 slots de arma (as duas mãos atacam) |
| Cura | Poção de cura: +50 de vida, sem cooldown |

### Armas (`game/src/weapon_blueprints.cpp`)

DPS = dano × ataques/s × (1 + chance crítica × (mult − 1)), contando só os passivos da própria arma e atributos zerados. Armas de golpe múltiplo estão em "alvo único".

| Arma | Raridade | Dano | Atq/s | Crítico (chance, mult) | DPS | Observação |
|---|---:|---:|---:|---|---:|---|
| Espada Curta | 2 | 12 | 1,4 (+0,12/Destreza) → 1,52 | 8% (+0,6%/Let.), ×1,3 | 18,7 | arco 110°, alcance 110 px |
| Machadinha | 2 | 16 | 1,1 | 10%, ×1,45 | 18,5 | estocada 118 px, 0,47 s |
| Arco Simples | 2 | 8,5 | 2,2 → 2,48 | 12%, ×1,45 | 22,6 | flecha 560 px/s, 860 px |
| Cajado de Carvalho | 3 | 2 por tick | 1,6 | 10%, ×1,55 | ~10 | ~3 ticks por pulso |
| Broquel | 3 | 10 | 0,6 | 5%, ×1,2 | 6,1 | +5 Defesa passiva |
| Espada Rúnica | 5 | 20 | 0,55 | 14%, ×1,65 | 25 a 37 | giro de 1,2 s, 2 a 3 acertos por giro |

### Inimigos da Dungeon (`game/src/enemy_spawner.cpp`)

| Inimigo | Vida | Vel. | Peso | Raio | Range | Arma | DPS bruto |
|---|---:|---:|---:|---:|---:|---|---:|
| `dungeon_ranged` | 27,5 | 85 | 1,0 | 22 | 560 | Cajado | ~10 |
| `dungeon_melee` | 47,5 | 92,5 | 1,3 | 26 | 150 | Machadinha | 17,6 |

### Salas, bolsas e economia

| Item | Valor |
|---|---|
| Sala normal | lados pares de 10 a 20 tiles (média 225 tiles); especiais 8×8 |
| Inimigos por sala normal | `round(área × 0,03)`, mínimo 1: média 6,8 |
| Tipos de sala | Normal 79, Forge 5, Shop 5, Chest 5 (fixos desde a 1.3) |
| Bolsa do inimigo da Dungeon | small/medium/big 50/20/5%: ~17 moedas por inimigo |
| Vaso | 20% de bolsa small (3,2 moedas) |
| Valor do item | `20 × raridade + baseValue`; compra `round(valor × 1,3)`; venda `round(valor × 0,2)` |
| Loja | 4 slots sorteados uniformemente entre os 19 itens (79% das lojas sem poção) |
| Baú comum | 1 a 3 itens sorteados uniformemente entre os 19 |
| Forja | 5 receitas de raridade 1; chance = `moedas / (valor A + valor B)`; falha quebra a bigorna |

Bolsas: small 12–20 moedas (20% de 1 couro); medium 24–36 (50% de 1–2 couro, 20% de 1 lingote); big 48–72 (100% de 2–3 couro, 50% de 1–2 lingotes).

## Metas aprovadas

### Ritmo

| Meta | Valor | Justificativa |
|---|---|---|
| Duração de uma run típica | 12 a 15 min | Demo curta; a pessoa vê loja, forja e baú e ainda morre dentro de uma sessão |
| Salas visitadas | 12 a 15: ~9 a 11 normais + 3 a 4 especiais | Sala normal ≈ 60 a 70 s; especial ≈ 40 s |
| Inimigos por sala normal | `round(área × 0,02)`, mín. 2: média 4,5, de 2 a 8 | 6,8 de média com inimigos mais resistentes tornaria a sala cansativa |
| Vida perdida por sala normal | ~30 PV (30% da vida inicial) | Com ~350 PV efetivos na run (100 + ~5 poções + armadura) dá ~10 salas normais |
| Fim da run | 90% das runs terminam antes de 20 min | Garantido pelo escalonamento temporário de dificuldade (abaixo) |

### Escalonamento de dificuldade (temporário)

**Solução temporária da demo.** Sem boss, um jogador bom nunca morreria. A versão final do jogo não usará este escalonamento; ele sai quando a progressão definitiva (bosses, biomas, metas) existir.

- `k = max(0, salas normais concluídas − 5)`: conta a partir da 6ª sala normal concluída.
- Inimigos que nascem depois disso: vida × (1 + 0,05·k), dano × (1 + 0,03·k).
- Regra pura e testável, isolada num único ponto, para ser removida sem tocar no resto.

### Combate: jogador

| Meta | Valor | Justificativa |
|---|---|---|
| Vida inicial | 100 | Cavaleiro sem bônus de personagem na demo |
| Mitigação inicial | 0%, sem redução fixa | Idem; a Defesa vem só de equipamento |
| Velocidade | 250 px/s, sem mudar | Mais rápido que qualquer inimigo (máx. 170): fuga e kite sempre possíveis |
| Vida no fim da run | 125 a 160 | Equipamento comum soma Vigor 2 a 3 e Defesa 1 a 2 por peça |
| `poder` | dano final × (1 + poder/100) | Já é o que a UI mostra; o cálculo passa a aplicar |
| DPS por tier de arma | raridade 2: ~20; raridade 3: ~30; raridade 4: ~40; raridade 5: ~50 | Cada tier rende mais que o anterior |

Ajustes de armas (1.9a), em alvo único e atributos zerados:

| Arma | DPS alvo | Regra |
|---|---:|---|
| Espada Curta, Machadinha, Arco Simples | sem mudança (18,5 a 22,6) | Já estão na curva de raridade 2 |
| Cajado de Carvalho | ~30 | Raridade 3 |
| Broquel | ~26 | Acima de todas as armas de raridade 2 (máx. 22,6); fica abaixo das outras de raridade 3 por causa da Defesa +5 |
| Espada Rúnica | ~50 | Média de 2 e 3 acertos por giro; com 2 acertos, ≥ 40 |

Os pontos de ajuste manual de cada arma ficam documentados em `docs/sistemas/armas.md` (criada na 1.9a).

### Atributos de escalonamento de arma

São cinco (`WeaponAttributeKey` em `player.h`), cada arma escala com um:

| Atributo | Armas atuais |
|---|---|
| Constituição | Broquel |
| Força | Espada Curta, Machadinha |
| Foco | Arco Simples |
| Misticismo | Espada Rúnica |
| Conhecimento | Cajado de Carvalho |

Na demo, nenhum equipamento concede esses atributos (fora o Kit do Testador, só no modo debug); o dano das armas vem só do valor base e de `poder`. Os equipamentos que concedem atributos de ataque e as novas receitas de forja ficam para uma subtarefa própria, definida pelo desenvolvedor (ver `status.md`).

### Combate: inimigos novos (1.7)

O jogador começa sem mitigação: dano bruto = dano recebido. "Arma inicial" = 20 DPS (Espada Curta); "tier médio" = 30 DPS (Cajado reajustado). TTK = vida ÷ DPS, sem tempo de mira e aproximação.

| Meta | Victim | Guard | Executioner |
|---|---:|---:|---:|
| Vida | 20 | 45 | 130 |
| TTK arma inicial | 1,0 s | 2,3 s | 6,5 s |
| TTK tier médio | 0,7 s | 1,5 s | 4,3 s |
| Golpes da Espada Curta para matar | 2 | 4 | 11 |
| Dano por golpe | 5 | 10 | 16 por acerto (até 2 por giro, 32) |
| DPS recebido (se todos acertam) | 3,1 | 5,6 | 8,0 |
| Golpes para matar o jogador (100 PV) | 20 | 10 | 7 acertos (~4 giros) |
| Carga antes do ataque | nenhuma | 0,35 s | 0,8 s |
| Peso de spawn | 1,0 | 1,0 | 0,35 |
| Crítico | 0% | 0% | 0% |

Por que:
- Victim é isca de pressão à distância: TTK de 1 s e dano baixo; o perigo é acumular vários.
- Guard: 0,35 s de carga dá reação só para quem está atento, e o intervalo de 1,8 s deixa espaço para contra-atacar.
- Executioner: 0,8 s de carga a 250 px/s permite sair do raio do giro (~130 px) caminhando (≥ 200 px); morre devagar de propósito.
- Spawn: pesos 1,0/1,0/0,35 (42%/42%/15% por inimigo), com **teto de 1 Executioner a cada 4 inimigos da sala** (`floor(n / 4)`): salas com menos de 4 inimigos não têm Executioner, salas de 8 têm no máximo 2. Um sorteio de Executioner acima do teto vira Victim ou Guard (pesos 1:1).
- Mistura típica de sala (4,5 inimigos): ~2 Victim, ~2 Guard, ~0,5 Executioner ≈ 200 de vida total (~10 s de DPS contínuo, 35 a 45 s com deslocamento).

### Economia

Moedas por origem. Tamanhos de bolsa não mudam; mudam as chances por inimigo.

| Origem | small | medium | big | Nada | Moedas esperadas |
|---|---:|---:|---:|---:|---:|
| Victim | 35% | 5% | 0% | 60% | 7,1 |
| Guard | 35% | 20% | 0% | 45% | 11,6 |
| Executioner | 0% | 40% | 40% | 20% | 36 |
| Vaso | 20% | 0% | 0% | 80% | 3,2 (sem mudar) |

| Meta | Valor |
|---|---|
| Moedas por inimigo (média pelos pesos) | ~13 |
| Moedas por sala normal | ~70 (60 de inimigos + ~10 de vasos; estimativa a medir) |
| Moedas por run | ~650 (9 a 11 salas normais), mais ~75 vendendo saque |
| Itens de bolsa | sem mudança (couro e lingote alimentam a forja) |
| Salas especiais por run | ≥ 70% das runs com ao menos uma loja visível até a 10ª sala visitada |

Tabela de valores por raridade (uma única tabela de dados). Compra = `valor × 1,3` arredondada a 5; venda = `round(valor × 0,3)`. Margem compra/venda de ~4,3 para 1.

| Categoria / raridade | Valor | Compra | Venda |
|---|---:|---:|---:|
| Material (1) | 20 | 25 | 6 |
| Poção de cura (1) | 35 | 45 | 11 |
| Equipamento comum (1) | 50 | 65 | 15 |
| Incomum (2) | 150 | 195 | 45 |
| Raro (3) | 250 | 325 | 75 |
| Épico (4) | 370 | 480 | 111 |
| Lendário (5) | 600 | 780 | 180 |

O que cabe numa run (~650 moedas, ~1,6 lojas visíveis):
- Cura: 1 a 3 poções por loja, 45 cada.
- 1 arma ou peça de raridade 2 a 3 (195 a 325).
- Raridade 4 só juntando moedas ou achando em baú; raridade 5 (780) praticamente só em baú.

Loja, baú e forja:
- Peso de raridade (loja e baú): 1: 60%, 2: 25%, 3: 10%, 4: 4%, 5: 1%. Dentro da raridade sorteada, item uniforme. Quanto maior a raridade, menor a chance de a arma ou o equipamento aparecer.
- Loja: 4 slots, sem repetição. **1 slot sempre poção** (estoque 2 a 4), 1 slot sorteado entre itens de raridade ≥ 2 (pelo peso), 2 slots sorteados pelo peso entre os demais itens.
- Baú comum: 2 a 3 slots sorteados pelo peso; poções 1 a 2 unidades, materiais 2 a 6.
- Forja: custo para 100% de chance = **80% do valor do item produzido** (equipamento comum, valor 50: 40 moedas). Chance linear: `moedas investidas / (0,8 × valor do produto)`, limitada a 100%; sem receita, 0%. Falhar quebra a bigorna. Receitas: só as 5 atuais de raridade 1, até a subtarefa de equipamentos de atributos de ataque.
- Kit do Testador: só existe no modo debug (abaixo); fora dele, não entra em loja, baú, forja nem no equipamento inicial.

### Modo debug

- Ligado por um arquivo local `debug.txt` no diretório de trabalho do jogo (como `biome.txt`): não versionado; arquivo ausente ou com valor diferente de ligado = desligado.
- Ligado: o jogador começa com o Kit do Testador nos 4 slots de equipamento, e o Kit entra nos pools de loja e baú.
- Desligado: o cavaleiro começa só com Espada Curta + Arco Simples, 100 PV e Defesa 0, e o Kit fica fora do jogo.

### Pesos de tipos de sala

Mantidos: Normal 79, Forge 5, Shop 5, Chest 5 (84% / 5,3% cada). Cada porta gera uma sala na hora (~2,5 salas novas por sala visitada; ~30 geradas em 12 visitas): P(ao menos uma Shop entre 30) ≈ 80%. A simulação de seeds (1.9c) confere a meta de 70% de runs com loja visível até a 10ª sala visitada. Se ficar abaixo, subir para Normal 70 / Shop 10 / Forge 10 / Chest 10.

## Atributos dos inimigos (1.7)

Todos sem arma do player e sem crítico. `Range` é a distância de início do ataque.

| Atributo | Victim | Guard | Executioner |
|---|---|---|---|
| Vida | 20 | 45 | 130 |
| Velocidade (px/s) | 170 | 140 | 90 |
| Raio de colisão (px) | 22 | 24 | 30 |
| Dano por golpe | 5 | 10 | 16 por acerto |
| Padrão de ataque | osso arremessado em linha reta | estocada tipo machadinha | giro 1,6 volta (480°/s, 1,2 s) |
| Alcance | 480 px de início; osso a 380 px/s, até 600 px | 120 px | 130 px de raio |
| Intervalo entre ataques (início a início) | 1,6 s | 1,8 s | 4,0 s |
| Carga (parado antes do ataque) | 0 | 0,35 s | 0,8 s, parado num frame de carga |
| Imunidade por alvo entre acertos | n/a | 0,6 s | 0,6 s (no máximo 2 acertos por giro) |
| Peso de spawn (`spawnRate`) | 1,0 | 1,0 | 0,35 (teto de 1 a cada 4 inimigos) |
| Bolsa (small/medium/big) | 35/5/0 | 35/20/0 | 0/40/40 |
| Moedas esperadas | 7,1 | 11,6 | 36 |
| Crítico / mitigação própria | 0 / 0 | 0 / 0 | 0 / 0 |

## Como validar

Testes de faixa headless em `game/tests/`, escritos junto com cada subtarefa:

- 1.7: atributos de cada inimigo dentro das faixas, valor esperado de moedas por tabela de bolsa, inimigos por sala (`round(área × 0,02)`, mín. 2) para todos os tamanhos, teto de Executioner, escalonamento temporário.
- 1.9a: DPS por tier com atributos zerados e efeito de `poder`.
- 1.9b: tabela de valor/compra/venda por raridade, pools de loja e baú (poção garantida, distribuição por raridade em muitas seeds, Kit só no modo debug), custo de forja, estado inicial do jogador com e sem debug.
- 1.9c: simulação de seeds medindo salas especiais visíveis até a 10ª sala visitada.

Exigem playtest (desenvolvedor jogando):

- Duração de 12 a 15 min e salas visitadas por run.
- Vida perdida por sala (~30 PV) e poções usadas.
- Legibilidade da carga do Guard (0,35 s) e do Executioner (0,8 s).
- Se o Victim é perigoso em grupo sem ser injusto.
- Sensação de progressão: o que o jogador compra e se a forja vale o risco.
- Se o escalonamento temporário encerra as runs em tempo razoável.

## Inconsistências encontradas e destino

| Inconsistência | Decisão | Onde |
|---|---|---|
| `InitializeInventoryUIDummyData` equipa 4× Kit do Testador e o Kit entra no pool de loja e baú | Modo debug por `debug.txt`; fora dele o Kit sai do jogo | 1.9b |
| `CreateKnightCharacter` define `equipmentBonuses`, mas `SyncEquipmentBonuses` os substitui | Bônus de personagem removidos na demo; o conflito entre bônus de personagem e de equipamento fica registrado em "Pós-demo" (`status.md`) | 1.9b |
| `poder` aparece na UI mas não entra no cálculo | Aplicar `× (1 + poder/100)` ao dano | 1.9a |
| Loja sem poção em 79% das vezes; baú uniforme entre todos os itens | Poção garantida e peso por raridade | 1.9b |
| Forja: 100% custa mais que comprar o produto | Custo de 100% = 80% do valor do produto | 1.9b |
| `dungeon_melee` com `range` maior que o alcance da estocada | Sem correção: o inimigo é substituído | 1.7 |
| Armas de raridade 3 rendem menos que as de raridade 2 | DPS por tier (tabela acima) | 1.9a |
| `PickRoomType` aumentava a chance de boss por sala | Já removido na 1.3; comentário em `kRoomTypeWeights` e item em "Pós-demo" para reintroduzir de forma melhor quando o boss voltar | 1.9c |
