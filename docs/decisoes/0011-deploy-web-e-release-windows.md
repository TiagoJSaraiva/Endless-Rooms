# 0011 — Deploy web no GitHub Pages e release para Windows

- **Data**: 2026-11
- **Status**: aceita

## Contexto
A demo precisa ser jogável sem instalar nada e também baixável para Windows. O jogo tem um único `main()` com um `while` bloqueante e muitos locais capturados por lambdas, o que não combina com o `emscripten_set_main_loop`.

## Decisão
- **Web**: o jogo é compilado com Emscripten (`make game PLATFORM=PLATFORM_WEB`, saída em `game/build-web/`) e publicado no GitHub Pages pelo workflow `deploy-web.yml` a cada push na `main`.
- O loop bloqueante roda via **ASYNCIFY**: `GameWindow::NextFrame` (`game_window.cpp`) aguarda o `requestAnimationFrame` e devolve o controle ao navegador a cada quadro. `-sASYNCIFY_IGNORE_INDIRECT=1` limita a instrumentação às chamadas diretas `main` → `NextFrame`. Por isso `NextFrame` é `noexcept`: com `-fexceptions`, uma chamada que pode lançar sai de `main` por um `invoke_*` em JS, o ASYNCIFY não instrumenta `main` e ela recomeça do zero a cada quadro (tela preta após o clique).
- Diferenças de plataforma ficam em `game_window.cpp` (janela e ritmo do loop); o resto do jogo não tem `#ifdef` de plataforma. Na web não há `SetTargetFPS`, porque a espera da raylib é ativa e travaria a página.
- `game/assets/` é empacotado com `--preload-file`, mantendo os mesmos caminhos relativos do desktop. A página é `game/web/shell.html`.
- **Windows**: tags `v*` disparam `release-windows.yml`, que compila com `PORTABLE=TRUE` (runtime estático, sem console), roda `mingw32-make test` e publica uma release com `EndlessRooms-windows.zip` (exe + `assets/`).

## Alternativas descartadas
- **itch.io**: exige conta e upload manual; o GitHub Pages fica no mesmo repositório e se atualiza sozinho.
- **`emscripten_set_main_loop`**: exigiria mover todo o estado de `main()` para uma estrutura de vida longa, uma refatoração grande do ponto de integração.
- **`WindowShouldClose` da raylib web** (dorme 16 ms fixos): ritmo irregular e somado ao tempo do quadro.
- **Release a cada push**: geraria uma release por commit; a tag marca quando uma versão está pronta para jogadores.

## Consequências
- Todo push na `main` vai direto para os jogadores da versão web.
- O build web não roda no `check.ps1` (exige o emsdk); quem o valida é o CI. Mudanças em `game_window.cpp`, no `Makefile` web ou em `shell.html` precisam de teste no navegador.
- Objetos web ficam em `game/obj-web/`, mas a raylib vendorizada compila os objetos no mesmo lugar para desktop e web: para alternar localmente, rode `clean` antes.
