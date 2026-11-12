# Status

## Onde paramos
- Etapa 0 concluída: fluxo de agentes adaptado do `godot-template` (fluxos `/inicio` `/planejar` `/revisar` `/fim`, `scripts/check.ps1`, hook de toolchain, memória em `docs/`) e regra de commits pelo agente (decisão 0008).
- Etapa 1 planejada: escopo da demo definido e quebrado em subtarefas (abaixo).
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
- Um único personagem jogável, o cavaleiro.
- Três inimigos na Dungeon.

Subtarefas, em ordem de execução. Cada uma é planejada no modo B antes do código.

- [ ] **1.1 Filtro de escuridão em tela cheia** (decisão 0009). Entregável: regra de alpha por ambiente com interpolação no corredor, testada sem janela, e filtro sobre o mundo com recorte de tochas e velas, deixando a interface fora dele. Corrige a escuridão que vaza para fora das paredes. Validação manual: o lobby fica claro e a tela escurece ao longo do corredor até a primeira sala da Dungeon.
- [ ] **1.2 Projéteis colidem com paredes**, tanto os do jogador quanto os dos inimigos. Entregável: teste headless de projétil parando na parede.
- [ ] **1.3 Remover a sala de boss do gerador**. Entregável: teste em várias seeds sem nenhuma sala `Boss`.
- [ ] **1.4 Congelar caverna e mansão**. Entregável: o sorteio retorna apenas Dungeon, e `biome.txt` continua forçando o bioma para testes.
- [ ] **1.5 Tela de fim de run**. Entregável: estatísticas da run (moedas, salas visitadas, inimigos derrotados, itens obtidos) contadas por regras testáveis, e a tela de resumo com botão para voltar ao lobby.
- [ ] **1.6 Metas de balanceamento**, propostas pelo agente e aprovadas pelo desenvolvedor antes de qualquer ajuste. Metas de economia: duração e moedas por run, preços de loja e forja por tier, chances de drop de vasos, inimigos e baús. Metas de combate: vida e dano do jogador, tempo para matar cada inimigo, dano por golpe e por segundo, e pesos de spawn. Entregável: tabela de metas em `docs/sistemas/`.
- [ ] **1.7 Inimigos da Dungeon**, implementados por completo pelo agente. Atributos seguem as metas da 1.6, os assets são placeholders com potencial de ficar definitivos, e nenhum inimigo usa armas do player. Os atuais `dungeon_melee` e `dungeon_ranged` são substituídos. Depende de 1.2 e 1.6.
  - [ ] **Victim**: esqueleto humano sem roupas que arremessa ossos de longa distância na direção do jogador. Tem pouca vida e se move rápido.
  - [ ] **Guard**: esqueleto com peitoral de ferro e lança. O ataque funciona como a machadinha do jogador, precedido por uma carga curta. É rápido, com dano e vida moderados.
  - [ ] **Executioner**: esqueleto executor com machadão. Ataca girando o machado, como a espada giratória do jogador. Antes do giro, fica parado num frame que sinaliza a carga e volta a andar depois que o ataque ativa. Tem vida e dano altos, e a carga dá ao jogador tempo de reagir. Aparece menos que os outros dois.
- [ ] **1.8 Visual definitivo do cavaleiro**: sprite e animações finais, sem mudanças em atributos ou habilidades.
- [ ] **1.9 Balanceamento aplicado**: metas aplicadas nas tabelas de dados, com testes de faixa que travam os valores, e depois um playtest. Depende de 1.4, 1.5, 1.6 e 1.7.
- [ ] **1.10 UI**: revisão visual do HUD e das telas de loja, forja, baú e inventário, sem regras novas e validada com o desenvolvedor.
- [ ] **1.11 Fechamento**: `check.ps1` completo passando, `arquitetura.md` e notas de sistemas atualizadas, e roteiro de validação jogando.

## Pós-demo
- Habilidades e atributos próprios do cavaleiro.
- Refatorar as imagens das armas aos poucos.
- Retomar os biomas caverna e mansão.

## Próximo passo
- Subtarefa 1.1 implementada; falta a validação jogando (roteiro em `docs/sistemas/props-e-iluminacao.md`, item 6). Validada, marcar 1.1 e seguir para 1.2 com `/planejar` no modo B.

## Bloqueios
- Nenhum.
