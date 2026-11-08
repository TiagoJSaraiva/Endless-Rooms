# 0005 — Pipeline de verificação

- **Data**: 2026-11
- **Status**: aceita

## Contexto
Agentes precisam de feedback objetivo para saber se uma mudança está correta antes de declará-la pronta.

## Decisão
`scripts/check.ps1` encadeia: toolchain (`check-toolchain.ps1`) → build do jogo (`mingw32-make game`) → testes headless (`mingw32-make test`) → testes de renderização com janela oculta. `-Fast` pula os testes de renderização. Check completo é obrigatório antes de concluir uma tarefa. O hook `SessionStart` do Claude Code roda `check-toolchain.ps1`.

## Alternativas descartadas
- **Só rodar o jogo**: pega erros apenas no caminho executado.
- **MCP para rodar o jogo**: não há equivalente maduro para raylib; o desenvolvedor valida jogando quando necessário.

## Consequências
Game feel e fluxos visuais continuam dependendo de verificação manual, descrita no resumo de cada tarefa.
