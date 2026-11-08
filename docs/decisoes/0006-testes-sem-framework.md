# 0006 — Testes sem framework, um executável por domínio

- **Data**: 2026-11 (registro de decisão anterior)
- **Status**: aceita

## Contexto
O projeto precisa de testes reproduzíveis sem trazer dependências extras para o build.

## Decisão
Cada domínio tem um executável em `game/tests/<dominio>_tests.cpp`, com `main()`, `assert` e mensagem de sucesso no fim. O alvo no Makefile lista só as fontes necessárias. Testes que não abrem janela entram em `mingw32-make test`; testes de renderização usam janela oculta e asserções sobre pixels offscreen, com alvo próprio `test-<dominio>-render`.

## Alternativas descartadas
- **doctest/Catch2/GoogleTest**: relatórios melhores, mas outra dependência a vendorizar sem necessidade atual.

## Consequências
Um `assert` falho aborta o executável inteiro, sem relatório por caso. `NDEBUG` nunca deve ser definido nos testes.
