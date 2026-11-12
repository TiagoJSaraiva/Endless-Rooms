# Status

## Onde paramos
- Etapa 0 concluída: fluxo de agentes adaptado do `godot-template` (fluxos `/inicio` `/planejar` `/revisar` `/fim`, `scripts/check.ps1`, hook de toolchain, memória em `docs/`) e regra de commits pelo agente (decisão 0008).
- Etapa 1 em andamento: 1.1 concluída; 1.2 a 1.5 implementadas e aguardando validação jogando; 1.6 concluída (metas aprovadas em `docs/sistemas/balanceamento.md`). Onda 1 de subagentes (1.7, 1.8, 1.9a, 1.9b, 1.9c) planejada.
- Jogo jogável: lobby, salas geradas por bioma, combate melee/ranged, props e tochas com iluminação, reward pouches, inventário, loja, forja e baú.

## Roadmap até a demo
Cada etapa é planejada com `/planejar` (levantamento de requisitos junto com o desenvolvedor) e quebrada em subtarefas aqui antes da execução.

- [x] **0 — Fluxo de agentes**: instruções, fluxos `/inicio` `/planejar` `/revisar` `/fim`, check e memória em `docs/`, no modelo do `godot-template`.
- [ ] **1 — Escopo da demo**: definir o conteúdo da demo e garantir que tudo nesse escopo esteja completamente funcional.
- [ ] **2 — Menu principal** do jogo.
- [ ] **3 — Deploy**: decidir entre embed no navegador e itch.io, e as decisões auxiliares (CI, build web).
- [ ] **4 — README** final, no estilo do README do projeto `kill-em-clankers-game`.

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
- [ ] **1.7 Inimigos da Dungeon** (onda 1). Victim, Guard e Executioner substituem `dungeon_melee` e `dungeon_ranged`, com padrões de ataque próprios (nenhuma arma do player), crítico zero e atributos, bolsas e pesos da tabela de metas. Inclui inimigos por sala (`round(área × 0,02)`, mín. 2), teto de 1 Executioner a cada 4 inimigos e o escalonamento temporário de dificuldade. Sprites placeholders gerados pelo agente. Entregável: testes de faixa headless dos atributos, da composição da sala e do escalonamento.
  - [ ] **Victim**: esqueleto sem roupas que arremessa ossos de longa distância; pouca vida, rápido.
  - [ ] **Guard**: esqueleto com peitoral de ferro e lança; estocada como a da machadinha, precedida por carga curta; rápido, dano e vida moderados.
  - [ ] **Executioner**: esqueleto executor com machadão; ataque giratório como a espada giratória do jogador, parado num frame de carga antes do giro; vida e dano altos; aparece menos.
- [ ] **1.8 Visual definitivo do cavaleiro** (onda 1): sprite e animações gerados pelo agente, sem mudanças em atributos ou habilidades.
- [ ] **1.9 Balanceamento aplicado**:
  - [ ] **1.9a Armas** (onda 1): `poder` aplicado ao dano (`× (1 + poder/100)`) e DPS por tier (Cajado, Broquel, Espada Rúnica). Entregável: testes de DPS por tier e de `poder`; nota `sistemas/armas.md` com os pontos de ajuste manual.
  - [ ] **1.9b Economia e modo debug** (onda 1): `debug.txt` (Kit do Testador nos 4 slots e nos pools só quando ligado), cavaleiro sem bônus de personagem, tabela de valores por raridade, venda a 30%, loja com poção garantida e peso por raridade, baú por peso de raridade, custo da forja = 80% do valor do produto. Entregável: testes headless dessas regras.
  - [ ] **1.9c Salas** (onda 1): simulação de seeds medindo salas especiais visíveis até a 10ª sala visitada e comentário em `kRoomTypeWeights` sobre reintroduzir a chance de boss crescente.
  - [ ] **1.9d Playtest** pelo desenvolvedor, com o roteiro de `balanceamento.md`. Depende de 1.7 a 1.9c.
- [ ] **1.10 UI**: revisão visual do HUD e das telas de loja, forja, baú e inventário, sem regras novas e validada com o desenvolvedor. Depois da onda 1.
- [ ] **1.11 Equipamentos de atributos de ataque**: equipamentos que concedem Força, Foco, Conhecimento (e talvez Constituição e Misticismo) e as receitas de forja correspondentes. Escopo a definir pelo desenvolvedor em sessão futura.
- [ ] **1.12 Fechamento**: `check.ps1` completo passando, `arquitetura.md` e notas de sistemas atualizadas, e roteiro de validação jogando.

### Execução da onda 1

Cada subtarefa da onda 1 é implementada por um subagente em branch própria (`feat/1.7-inimigos`, `feat/1.8-cavaleiro`, `feat/1.9a-armas`, `feat/1.9b-economia`, `test/1.9c-salas`), em paralelo: nenhuma depende de outra, e os conflitos esperados (Makefile, `.gitignore`, `check.ps1`, `arquitetura.md`, `CreateKnightCharacter`) são resolvidos no merge. O agente principal revisa os diffs, faz o merge na `main` (ordem: 1.9c, 1.9a, 1.8, 1.9b, 1.7), roda o check completo, faz push e apaga as branches. Subagentes não editam `status.md` nem `balanceamento.md`.

## Pós-demo
- Habilidades e atributos próprios do cavaleiro.
- **Bônus de personagem**: voltam com a pluralidade de personagens. Hoje `SyncEquipmentBonuses` sobrescreve `equipmentBonuses`, onde o cavaleiro guardava seus bônus; ao reintroduzir, separar bônus de personagem e de equipamento.
- **Chance de sala de boss**: quando o boss voltar, reintroduzir uma chance que cresça com o avanço da run, de forma melhor que o antigo `peso − 0,5 por sala`.
- **Escalonamento de dificuldade definitivo**: substituir o escalonamento temporário da demo (`balanceamento.md`).
- Refatorar as imagens das armas aos poucos.
- Retomar os biomas caverna e mansão.

## Próximo passo
- Agente: executar a onda 1 (subagentes), revisar, fazer o merge e o push.
- Desenvolvedor: validar 1.2 a 1.5 jogando; depois da onda 1, playtest (1.9d) e revisão de UI (1.10).

## Bloqueios
- Nenhum.
