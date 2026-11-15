# Status

## Onde paramos
- Etapa 0 concluída: fluxo de agentes adaptado do `godot-template` (fluxos `/inicio` `/planejar` `/revisar` `/fim`, `scripts/check.ps1`, hook de toolchain, memória em `docs/`) e regra de commits pelo agente (decisão 0008).
- Etapa 1 em andamento: 1.1 e 1.6 concluídas; 1.2 a 1.5 implementadas e aguardando validação jogando. Onda 1 (1.7, 1.8, 1.9a, 1.9b, 1.9c) implementada por subagentes, integrada na `main` com check completo passando; aguarda validação jogando.
- Sprites do cavaleiro e dos inimigos ampliados em 1,4× (`SPRITE_SCALE` nos geradores de `tools/sprites/`). A área de dano virou o retângulo do corpo de cada sprite e os ataques saem da altura das mãos (`body_rules`); o colisor de movimento não mudou.
- Ajustes de ambientação: mais tochas (raramente vizinhas) e tochas nas paredes laterais de salas normais e corredores; menos vasos; mais barris com vela, espalhados; salas especiais nunca vizinhas de outra do mesmo tipo. Tochas laterais são desenhadas acima de props de piso que as cubram.
- Rebalanceamento manual de armas pelo desenvolvedor (`4913d1d`): Cajado de Carvalho com 4,5 de dano por tick (DPS ~22,8) e Espada Rúnica com 36 de dano e giro de 150 × 40 (DPS médio ~55,9). As faixas de `weapon_rules_tests`, `armas.md` e `balanceamento.md` foram ajustadas a esses valores (Cajado 21 a 25; Rúnica com média 50 a 60).
- Correções visuais de luz e paredes: a luz das fontes termina na borda real da sala (sem faixa no vazio abaixo); a face da parede inferior fica na escuridão do bioma, sem luz de nenhuma fonte (nem das tochas presas a ela); o rodapé das paredes inferiores se dissolve no vazio em toda a extensão (corrigidas colunas sem esmaecer junto a corredores alinhados); a máscara de corredores laterais fechados cobre o corredor inteiro na vertical, com transição junto à porta. Aguardam validação jogando (roteiros 10 e 11 em `props-e-iluminacao.md`).
- Publicação (etapas 3 e 4, decisão 0011): versão web no GitHub Pages (https://tiagojsaraiva.github.io/Endless-Rooms/), recompilada e publicada pelo CI a cada push na `main`; tags `v*` geram a release com `EndlessRooms-windows.zip`. README em inglês e LEIA-ME em português, no estilo do `kill-em-clankers-game`. A página de carregamento usa o logo oficial, a barra de carregamento ilustrada, imagens de teclas e a fonte Alagard (arte em `game/web/assets/`). O menu principal (etapa 2) ficou para depois da publicação, a pedido do desenvolvedor.
- Jogo jogável: lobby, salas geradas por bioma, combate melee/ranged, props e tochas com iluminação, reward pouches, inventário, loja, forja e baú.

## Roadmap até a demo
Cada etapa é planejada com `/planejar` (levantamento de requisitos junto com o desenvolvedor) e quebrada em subtarefas aqui antes da execução.

- [x] **0 — Fluxo de agentes**: instruções, fluxos `/inicio` `/planejar` `/revisar` `/fim`, check e memória em `docs/`, no modelo do `godot-template`.
- [ ] **1 — Escopo da demo**: definir o conteúdo da demo e garantir que tudo nesse escopo esteja completamente funcional.
- [ ] **2 — Menu principal** do jogo.
- [x] **3 — Deploy**: GitHub Pages com build web pelo CI e release do Windows por tag (decisão 0011). *Falta validação jogando no navegador.*
- [x] **4 — README** em inglês e LEIA-ME em português, no estilo do `kill-em-clankers-game`. *Faltam captura de tela, créditos da arte e licença.*

## Etapa 1 — Escopo da demo

Conteúdo da demo:
- Só o bioma Dungeon. Caverna e mansão saem do sorteio, mas o código continua no repositório.
- Sem sala de boss e sem boss.
- A morte encerra a run e abre uma tela de resumo, com botão para voltar ao lobby.
- Um único personagem jogável, o cavaleiro, sem bônus de personagem (100 PV, Defesa 0).
- Três inimigos na Dungeon.

Subtarefas, em ordem de execução. Metas numéricas em [balanceamento.md](sistemas/balanceamento.md).

- [x] **1.1 Filtro de escuridão em tela cheia** (decisão 0009). Regra de alpha por ambiente com interpolação no corredor, testada sem janela, e filtro sobre o mundo com recorte de tochas e velas, deixando a interface fora dele.
- [ ] **1.2 Projéteis colidem com paredes**, do jogador e dos inimigos. *Implementada; falta validação jogando ([colisao-de-salas.md](sistemas/colisao-de-salas.md)).*
- [ ] **1.3 Remover a sala de boss do gerador**. *Implementada; falta validação jogando.*
- [ ] **1.4 Congelar caverna e mansão**: o sorteio retorna apenas Dungeon e `biome.txt` continua forçando o bioma. *Implementada; falta validação jogando.*
- [ ] **1.5 Tela de fim de run** com estatísticas e botão para voltar ao lobby. *Implementada; falta validação jogando ([fim-de-run.md](sistemas/fim-de-run.md)).*
- [x] **1.6 Metas de balanceamento**: aprovadas em [balanceamento.md](sistemas/balanceamento.md).
- [ ] **1.7 Inimigos da Dungeon** (onda 1). Victim, Guard e Executioner substituem `dungeon_melee` e `dungeon_ranged`, com padrões de ataque próprios (nenhuma arma do player), crítico zero e atributos, bolsas e pesos da tabela de metas. Inclui inimigos por sala (`round(área × 0,02)`, mín. 2), teto de 1 Executioner a cada 4 inimigos e o escalonamento temporário de dificuldade. Sprites placeholders gerados pelo agente. Entregável: testes de faixa headless dos atributos, da composição da sala e do escalonamento. *Implementada; falta validação jogando ([inimigos.md](sistemas/inimigos.md)).*
  - [ ] **Victim**: esqueleto sem roupas que arremessa ossos de longa distância; pouca vida, rápido.
  - [ ] **Guard**: esqueleto com peitoral de ferro e lança; estocada como a da machadinha, precedida por carga curta; rápido, dano e vida moderados.
  - [ ] **Executioner**: esqueleto executor com machadão; ataque giratório como a espada giratória do jogador, parado num frame de carga antes do giro; vida e dano altos; aparece menos.
- [ ] **1.8 Visual definitivo do cavaleiro** (onda 1): sprite e animações gerados pelo agente (`tools/sprites/knight.py`), sem mudanças em atributos ou habilidades. *Implementada; falta validação jogando.*
- [ ] **1.9 Balanceamento aplicado**:
  - [ ] **1.9a Armas** (onda 1): `poder` aplicado ao dano (`× (1 + poder/100)`) e DPS por tier (Cajado, Broquel, Espada Rúnica). Entregável: testes de DPS por tier e de `poder`; nota `sistemas/armas.md` com os pontos de ajuste manual. *Implementada; falta validação jogando ([armas.md](sistemas/armas.md)).*
  - [ ] **1.9b Economia e modo debug** (onda 1): `debug.txt` (Kit do Testador nos 4 slots e nos pools só quando ligado), cavaleiro sem bônus de personagem, tabela de valores por raridade, venda a 30%, loja com poção garantida e peso por raridade, baú por peso de raridade, custo da forja = 80% do valor do produto. Entregável: testes headless dessas regras. *Implementada; falta validação jogando ([economia.md](sistemas/economia.md)).*
  - [ ] **1.9c Salas** (onda 1): simulação de seeds medindo salas especiais visíveis até a 10ª sala visitada e comentário em `RoomTypeRules::Weights` sobre reintroduzir a chance de boss crescente. *Concluída: com os pesos 71/12/12/5 e sem especiais iguais vizinhas, loja visível em 90,6% das runs (ver balanceamento.md).*
  - [ ] **1.9d Playtest** pelo desenvolvedor, com o roteiro de `balanceamento.md`. Depende de 1.7 a 1.9c.
- [ ] **1.10 UI**: revisão visual do HUD e das telas de loja, forja, baú e inventário, sem regras novas e validada com o desenvolvedor. Depois da onda 1.
- [ ] **1.11 Equipamentos de atributos de ataque**: equipamentos que concedem Força, Foco, Conhecimento (e talvez Constituição e Misticismo) e as receitas de forja correspondentes. Escopo a definir pelo desenvolvedor em sessão futura.
- [ ] **1.12 Fechamento**: `check.ps1` completo passando, `arquitetura.md` e notas de sistemas atualizadas, e roteiro de validação jogando.

### Execução da onda 1

Concluída: cada subtarefa foi implementada por um subagente em branch própria, integrada na `main` (merges `466fa06`, `45e5b42`, `25cd5f4`, `a5683f3`, `8c77e58`) e as branches foram apagadas.

## Pós-demo
- Habilidades e atributos próprios do cavaleiro.
- **Bônus de personagem**: voltam com a pluralidade de personagens. Hoje `SyncEquipmentBonuses` sobrescreve `equipmentBonuses`, onde o cavaleiro guardava seus bônus; ao reintroduzir, separar bônus de personagem e de equipamento.
- **Chance de sala de boss**: quando o boss voltar, reintroduzir uma chance que cresça com o avanço da run, de forma melhor que o antigo `peso − 0,5 por sala`.
- **Escalonamento de dificuldade definitivo**: substituir o escalonamento temporário da demo (`balanceamento.md`).
- Refatorar as imagens das armas aos poucos.
- Retomar os biomas caverna e mansão.

## Próximo passo
- Desenvolvedor: jogar a versão web (https://tiagojsaraiva.github.io/Endless-Rooms/) e conferir carregamento, ritmo dos quadros, mouse (mira e clique direito), teclado (Tab, 1 a 5) e o escurecimento por tochas; reportar o que divergir do desktop.
- Desenvolvedor: informar origem e licença da arte (chão, paredes, props, itens) e a licença do projeto, e fornecer uma captura de tela de gameplay para os READMEs (TODO no fim de `README.md` e `LEIA-ME.md`).
- Desenvolvedor: validar jogando 1.2 a 1.5 e a onda 1 (roteiros em `inimigos.md`, `armas.md`, `economia.md` e `balanceamento.md`), depois o playtest 1.9d; validar a escala 1,4× dos sprites, a área de dano retangular e a saída dos ataques pelas mãos (roteiro em `inimigos.md`; conferir também `displayOffset` de arco, cajado e espada em `armas.md`); jogar com os pesos de salas 71/12/12/5 (mais lojas e forjas, menos salas normais) e dizer se a quantidade de moedas por run continua boa.
- Desenvolvedor: validar jogando os ajustes de ambientação (roteiro 9 em `props-e-iluminacao.md`): tamanho e posição das tochas laterais, quantidade de tochas, vasos e barris, luz geral das salas.
- Desenvolvedor: validar jogando as correções de luz, rodapé e máscara de corredores (roteiros 10 e 11 em `props-e-iluminacao.md`); se o rodapé (32 px) ou a transição da máscara (64 px) pedirem outro tamanho, os ajustes são `kFooterFadeHeight` em `wall_tiles.cpp` e `kHorizontalFadeWidth` em `door_visual_rules.cpp`.
- Depois da validação: 1.10 (UI) com o desenvolvedor.

## Bloqueios
- Nenhum.
