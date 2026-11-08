# 0001 — C++17 com raylib 5.5 vendorizada

- **Data**: 2026-11 (registro de decisão anterior)
- **Status**: aceita

## Contexto
O jogo precisa compilar da mesma forma em qualquer máquina, sem depender de uma instalação global da raylib com versão desconhecida.

## Decisão
C++17 e raylib 5.5, com o código-fonte da raylib copiado em `game/vendor/raylib` e compilado junto com o jogo. `RAYLIB_VERSION` no Makefile fixa a versão. Não se altera o código da raylib nem outras instalações dela.

## Alternativas descartadas
- **Raylib instalada no sistema**: versão varia por máquina e quebra builds silenciosamente.
- **Engine completa (Godot, Unity)**: o projeto é também um exercício de C++ e arquitetura própria.

## Consequências
Build reproduzível e offline. Atualizar a raylib exige uma nova decisão, substituir `vendor/raylib` e ajustar `RAYLIB_VERSION` no mesmo passo.
