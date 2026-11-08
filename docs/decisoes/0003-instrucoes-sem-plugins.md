# 0003 — Instruções em AGENTS.md, sem plugins de agente

- **Data**: 2026-11
- **Status**: aceita

## Contexto
O projeto deve funcionar com Claude Code e Codex. Plugins de agente adicionam contexto, custo e instruções concorrentes, e são específicos de uma ferramenta. O modelo adotado vem do `godot-template`.

## Decisão
`AGENTS.md` é a fonte única de instruções; `CLAUDE.md` apenas o importa. Fluxos ficam em `.agents/fluxos/` como markdown comum, com comandos finos em `.claude/commands/` apontando para eles. Gemini CLI não é suportado por ora.

## Alternativas descartadas
- **Plugins de processo/memória (Superpowers, claude-mem)**: específicos do Claude, opacos ou rígidos demais para mecânicas de jogo.
- **Comandos para Gemini CLI**: não usado no projeto; adicionar depois é só criar `GEMINI.md` e `.gemini/commands/`.

## Consequências
A disciplina de memória depende do fluxo `fim` e da revisão do desenvolvedor.
