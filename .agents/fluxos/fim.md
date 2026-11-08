# Fluxo: fim de sessão ou de tarefa

1. Rode o check completo:
   `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check.ps1`
   Se falhar, corrija ou reporte. Não encerre como concluído com check falhando.
2. Atualize a memória em `docs/`:
   - `status.md`: onde paramos, subtarefas concluídas da etapa, próximo passo, bloqueios.
   - `arquitetura.md`: se módulos, contratos de header, pastas ou build mudaram.
   - `sistemas/`: nota do sistema criado ou alterado.
   - `decisoes/`: nova nota para cada decisão estrutural tomada.
3. Faça o commit do que restar (código do último módulo e `docs/`), seguindo a seção "Commits" do `AGENTS.md`. Não faça push.
4. Responda com:
   - Resumo das mudanças de código (arquivos e propósito).
   - Arquivos de `docs/` alterados.
   - Commits feitos na tarefa (hash curto e mensagem).
   - O que o desenvolvedor deve validar jogando (game feel, visual, fluxos sem teste automatizado).
