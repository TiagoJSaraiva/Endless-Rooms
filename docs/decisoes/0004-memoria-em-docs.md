# 0004 — Memória do projeto em docs/

- **Data**: 2026-11
- **Status**: aceita

## Contexto
Agentes não lembram nada entre sessões. Reexplicar o projeto custa tempo e tokens.

## Decisão
Memória curada em markdown dentro de `docs/`, versionada com o código: `status.md` (incluindo o roadmap até a demo), `arquitetura.md`, `convencoes-cpp.md`, `sistemas/` e `decisoes/`. Atualizada no fluxo `fim`. Anotações pessoais, mockups e rascunhos ficam em `anotacoes/`, ignorada pelo Git; agentes não a alteram sem pedido explícito.

## Alternativas descartadas
- **Roadmap só em `anotacoes/todo.txt`**: não versionado, invisível para o histórico do projeto.
- **Memória automática do agente**: registra tentativas, não o que é verdade, e fica fora do repositório.

## Consequências
Código e documentação voltam juntos em qualquer commit. A memória exige revisão do diff de `docs/`.
