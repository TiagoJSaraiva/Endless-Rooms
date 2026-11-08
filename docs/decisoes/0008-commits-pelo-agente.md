# 0008 — Commits feitos pelo agente, por módulo, na main

- **Data**: 2026-11
- **Status**: aceita

## Contexto
O `godot-template` proíbe o agente de fazer commits. Neste projeto, o desenvolvedor prefere um histórico granular, com um commit por parte concluída de cada implementação, sem precisar commitar manualmente.

## Decisão
O agente faz commits durante e ao fim da implementação, um por "módulo" concluído (uma parte coesa e verificada da tarefa), diretamente na `main`. Mensagens seguem Conventional Commits em português (`<tipo>: <verbo no infinitivo> ...`). Sem trailer de coautoria (`Co-Authored-By`) nem menção ao agente. O agente nunca faz `push`, nunca cria branches e nunca reescreve histórico (`amend`, `rebase`, `reset` de commits já feitos) sem pedido explícito.

## Alternativas descartadas
- **Agente nunca commita (modelo do template)**: o desenvolvedor teria que fatiar commits manualmente depois.
- **Branch por tarefa**: overhead desnecessário para um desenvolvedor só e sem CI ainda.

## Consequências
Cada commit deve compilar e passar `check.ps1 -Fast`. O desenvolvedor revisa pelo histórico e faz o push quando quiser.
