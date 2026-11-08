# Fluxo: início de sessão

1. Confirme o toolchain:
   `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-toolchain.ps1`
   (no Claude Code, um hook já roda isso ao abrir a sessão). Se der ERRO, pare e avise.
2. Rode `git status --short`. Se houver alterações locais, liste-as: elas não são suas e devem ser preservadas.
3. Leia `docs/status.md` e as notas que ele citar.
4. Responda em até 5 linhas: etapa atual do roadmap, estado, próximo passo, bloqueios, dúvidas abertas.
5. Não escreva nem altere código neste fluxo.
