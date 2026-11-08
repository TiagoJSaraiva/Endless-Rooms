# 0002 — Build com Makefile e MinGW-w64

- **Data**: 2026-11 (registro de decisão anterior)
- **Status**: aceita

## Contexto
O projeto nasceu do template de Makefile da raylib e é desenvolvido no Windows com MinGW-w64.

## Decisão
`game/Makefile` (GNU Make, `mingw32-make` no Windows) é o único sistema de build. Ele inclui automaticamente todos os `.cpp` de `game/src/`, compila a raylib vendorizada e define um alvo por executável de teste. `COMPILER_PATH` é opcional para toolchains fora do `PATH`. `clean` remove apenas saídas do projeto e da raylib vendorizada.

## Alternativas descartadas
- **CMake + Ninja**: mais portável, mas a migração não traz ganho imediato para um projeto de um desenvolvedor só no Windows.

## Consequências
Cada novo executável de teste exige alvo, lista de fontes e entrada em `clean` no Makefile. Um build web (etapa 3 do roadmap) pode exigir revisar esta decisão.
