# Fim de run

## Propósito
Ao morrer, o jogador vê um resumo da run (tempo, salas, inimigos, moedas e itens) e um botão que inicia uma nova run no lobby.

## Componentes
- `run_stats.h/.cpp`: `RunStats` e `namespace RunStatsRules`, regras puras (sem janela, input ou `GetFrameTime`): acumular tempo, inimigos e recibos de coleta; contar salas visitadas; formatar duração; montar as linhas de itens.
- `ui_run_summary.h/.cpp`: `DrawRunSummary` desenha o painel e retorna `true` no clique em "Voltar ao lobby". Não decide regra.
- `main.cpp`: mantém o `RunStats` da run, alimenta-o e congela o resumo no momento da morte.

## Regras
- Tempo: soma o delta de cada frame enquanto o jogador está vivo; zera em `BeginNewRun`.
- Inimigos derrotados: cada `RewardPouchOperations::DamageEnemy` que retorna `true` (única via de morte de inimigo).
- Moedas e itens: só o que foi entregue pelos recibos de `RewardPouchOperations::Collect`. Compras, vendas, baú e forja não contam.
- Itens agregados por id, em ordem da primeira coleta. Id sem definição aparece como "Item desconhecido".
- Salas visitadas: `Room::IsVisited()`, excluindo `RoomType::Lobby`.
- Duração: `mm:ss`; a partir de 1 hora, `h:mm:ss`.
- Lista de itens exibe até 6 linhas; se houver mais, a última vira "+N outros".

## Contratos
- `RunStats` pertence ao `main.cpp`; nenhum outro módulo guarda referência a ele.
- Salas visitadas e linhas de itens são calculadas uma vez, no frame da morte (`playerDead` passa a `true`), e só lidas depois.
- O clique no botão chama `BeginNewRun(true)`: nova seed, lobby, estatísticas zeradas.

## Testes
- `game/tests/run_stats_tests.cpp` (`mingw32-make test`): tempo, formatação, inimigos, recibos (agregação, ordem, vazio), salas visitadas sem lobby e nomes com fallback.
- Verificação manual: morrer após derrotar inimigos, pegar bolsas e visitar salas; conferir os números e itens no painel, o botão "Voltar ao lobby" levando a uma run nova e o layout em janelas de tamanhos diferentes.
