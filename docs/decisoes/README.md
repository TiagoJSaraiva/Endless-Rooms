# Decisões

Uma nota por decisão estrutural, numerada em sequência: `NNNN-titulo-curto.md`.
Decisão aceita não é editada para mudar de ideia: crie uma nova e marque a antiga como "substituída por NNNN".

| Nº | Decisão |
|---|---|
| [0001](0001-cpp17-raylib-vendorizada.md) | C++17 com raylib 5.5 vendorizada |
| [0002](0002-build-com-makefile.md) | Build com Makefile e MinGW-w64 |
| [0003](0003-instrucoes-sem-plugins.md) | Instruções em AGENTS.md, sem plugins de agente |
| [0004](0004-memoria-em-docs.md) | Memória do projeto em docs/ |
| [0005](0005-pipeline-de-verificacao.md) | Pipeline de verificação |
| [0006](0006-testes-sem-framework.md) | Testes sem framework, um executável por domínio |
| [0007](0007-regras-operacoes-renderizacao.md) | Separar regras, operações e renderização |
| [0008](0008-commits-pelo-agente.md) | Commits feitos pelo agente, por módulo, na main |
| [0009](0009-escuridao-em-tela-cheia.md) | Escuridão como filtro de tela cheia |

Modelo:

```markdown
# NNNN — Título

- **Data**: AAAA-MM
- **Status**: aceita | substituída por NNNN

## Contexto
O problema e as restrições.

## Decisão
O que foi escolhido.

## Alternativas descartadas
Cada uma com o motivo.

## Consequências
O que isso facilita e o que dificulta.
```
