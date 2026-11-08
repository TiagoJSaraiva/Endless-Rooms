# Fluxo: planejar

Há dois tipos de planejamento. Identifique qual é pelo pedido; na dúvida, pergunte.

## A. Etapa do roadmap (`docs/status.md`)

Etapas são amplas ("Menu principal", "Deploy") e precisam de requisitos levantados com o desenvolvedor antes de virar tarefas.

1. Leia a etapa em `docs/status.md`, `docs/arquitetura.md` e as notas de `docs/sistemas/` e `docs/decisoes/` relacionadas. Leia código só o suficiente para saber o que já existe.
2. Apresente o que já existe e o que falta para a etapa, em poucas linhas.
3. Levante requisitos com perguntas objetivas (até 4 por rodada, com opções e uma recomendação). Faça quantas rodadas forem necessárias; não suponha escopo.
4. Proponha a quebra em subtarefas: cada uma com objetivo em uma linha, entregável verificável e dependências. Ordene por risco/valor.
5. Após aprovação, registre as subtarefas como checklist sob a etapa em `docs/status.md` e as decisões estruturais em `docs/decisoes/`. Commit desse registro (`docs: ...`).
6. Cada subtarefa é planejada depois pelo modo B.

## B. Tarefa

Qualquer tarefa que crie sistema, altere contrato de header ou mexa em mais de ~3 arquivos.

1. Se o pedido for ambíguo, faça até 3 perguntas objetivas antes de planejar.
2. Leia `docs/arquitetura.md`, `docs/convencoes-cpp.md`, as notas de `docs/sistemas/` envolvidas e as decisões relevantes em `docs/decisoes/`.
3. Apresente o plano:
   - **Objetivo**: o que muda para quem joga.
   - **Arquivos**: criar / alterar, com uma linha de propósito cada.
   - **Módulos e contratos**: camada de cada parte (regras, operações, apresentação), headers públicos novos ou alterados, quem é dono de quê e tempo de vida de ponteiros.
   - **Testes**: executáveis em `game/tests/` novos ou ampliados (e alvos no Makefile); o que fica para verificação manual.
   - **Passos e commits**: sequência de passos pequenos, cada um verificável com `scripts/check.ps1 -Fast`, agrupados nos módulos que viram commits (com a mensagem prevista).
   - **Alternativas descartadas**: no máximo 2, com o motivo.
   - **Riscos**: o que pode quebrar ou ficar difícil de mudar depois.
4. Pare e aguarde aprovação. Não escreva código antes dela.
