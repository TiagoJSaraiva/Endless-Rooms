@AGENTS.md

## Específico do Claude Code

- Fluxos disponíveis como comandos: `/inicio`, `/planejar`, `/revisar`, `/fim`.
- O hook `SessionStart` (`.claude/settings.json`) roda `scripts/check-toolchain.ps1` ao abrir a sessão; se ele acusar ERRO, avise antes de qualquer outra coisa.
- Abra a sessão na raiz do repositório (não em `game/`), para que o hook e os comandos encontrem os caminhos.
