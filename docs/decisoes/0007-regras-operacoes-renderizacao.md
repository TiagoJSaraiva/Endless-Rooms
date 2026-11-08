# 0007 — Separar regras, operações e renderização

- **Data**: 2026-11 (registro de decisão anterior)
- **Status**: aceita

## Contexto
`main.cpp` concentrava gameplay, estado e desenho, o que tornava regras difíceis de testar e mudanças arriscadas.

## Decisão
Cada domínio se divide em até três camadas: **dados/regras** (`<dominio>.h` e `<dominio>_rules`: funções puras ou quase puras, testáveis sem janela), **operações** (`<dominio>_operations`: aplicam regras ao estado de jogo) e **apresentação** (`<dominio>_renderer`, `ui_*`: texturas e desenho). Regras não chamam funções de desenho da raylib; renderizadores não decidem regra de jogo. Funções de regra ficam em `namespace <Dominio>Rules`.

## Alternativas descartadas
- **ECS**: reescrita grande sem ganho proporcional para o escopo da demo.

## Consequências
Regras novas ganham teste headless. Mais arquivos por domínio; `main.cpp` continua como ponto de integração e deve encolher aos poucos.
