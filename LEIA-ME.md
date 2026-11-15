<h1 align="center">Endless Rooms</h1>

<p align="center">
  <strong>Abra caminho por salas de masmorra geradas proceduralmente, equipe-se e veja até onde você chega.</strong><br>
  Roguelite de ação 2D feito em C++17 com raylib. Jogue a demo no navegador ou baixe para Windows.
</p>

<p align="center">
  <a href="https://tiagojsaraiva.github.io/Endless-Rooms/"><strong>▶ Jogar agora</strong></a>
  &nbsp;·&nbsp;
  <a href="https://github.com/TiagoJSaraiva/Endless-Rooms/releases/latest"><strong>⬇ Baixar para Windows</strong></a>
  &nbsp;·&nbsp;
  <a href="README.md">Read in English</a>
</p>

## Como jogar

| Entrada | Ação |
|---|---|
| W, A, S, D | Mover |
| Clique esquerdo | Atacar com a arma da mão esquerda |
| Clique direito | Atacar com a arma da mão direita |
| E | Abrir portas, usar lojas, forjas e baús |
| Tab ou I | Inventário |
| 1 a 5 | Habilidades dos itens equipados |

Você começa no lobby, ao lado de um boneco de treino. Atravesse uma porta para entrar na masmorra: cada sala é gerada conforme você avança, e as portas só abrem quando todos os inimigos da sala forem derrotados. Inimigos deixam bolsas de recompensa com moedas e itens. As salas ficam mais perigosas à medida que você as limpa.

A morte encerra a run. Um resumo mostra o tempo sobrevivido, as salas visitadas, os inimigos derrotados e o que você coletou, e leva você de volta ao lobby para uma nova run com um mapa novo.

### Inimigos

| Inimigo | Comportamento |
|---|---|
| Victim | Rápido e frágil; arremessa ossos de longe |
| Guard | Esqueleto de armadura com lança; carga curta e depois estocada |
| Executioner | Lento e resistente; prepara o golpe e gira um machado enorme ao redor de si. Aparece menos que os outros |

### Salas especiais

| Sala | O que faz |
|---|---|
| Loja | Compre poções e equipamentos, ou venda itens por 30% do valor |
| Forja | Invista moedas para ter chance de forjar um item a partir de uma receita; quanto mais moedas, maior a chance |
| Baú | Itens encontrados pelo caminho |

### Armas

Cada mão segura uma arma, então dá para combinar uma arma corpo a corpo com uma de longo alcance. Armas melhores são mais raras.

| Arma | Raridade | Estilo |
|---|---|---|
| Espada Curta | 2 | Cortes rápidos |
| Machadinha | 2 | Golpes mais lentos e pesados |
| Arco Simples | 2 | Flechas rápidas à distância |
| Cajado de Carvalho | 3 | Rajadas curtas de laser |
| Broquel | 3 | Golpe de escudo; também concede defesa |
| Espada Rúnica | 5 | Gira ao seu redor, acertando tudo o que estiver perto |

A demo cobre o bioma Dungeon, com o cavaleiro como único personagem jogável, e não tem chefe.

## Rodando localmente

Windows, com [MinGW-w64](https://www.mingw-w64.org) (`g++`, `gcc`, `ar`) e GNU Make (`mingw32-make`) no `PATH`. A raylib 5.5 está incluída em `game/vendor/raylib` e é compilada junto com o jogo.

```powershell
cd game
mingw32-make game
.\game.exe
```

Execute o jogo de dentro de `game/`: os caminhos dos assets são relativos a essa pasta.

| Comando (dentro de `game/`) | O que faz |
|---|---|
| `mingw32-make game` | Compila o jogo (`game.exe`) |
| `mingw32-make game PORTABLE=TRUE` | Build autocontido, sem DLLs do MinGW e sem console, como o das releases |
| `mingw32-make test` | Compila e executa os testes headless |
| `mingw32-make clean` | Remove as saídas de build |
| `make game PLATFORM=PLATFORM_WEB` | Build web em `build-web/` (exige o [Emscripten SDK](https://emscripten.org) ativado) |

Na raiz do repositório, `scripts/check.ps1` executa toda a verificação: toolchain, build, testes headless e testes de renderização. Mais detalhes em [INSTRUCTION.md](INSTRUCTION.md).

## Estrutura

| Caminho | Conteúdo |
|---|---|
| `game/src/main.cpp` | Loop do jogo: conecta todos os sistemas |
| `game/src/game_window.cpp` | Abertura da janela e ritmo dos quadros no desktop e no navegador |
| `game/src/*_rules.cpp` | Regras de jogo (inimigos, armas, economia, luz, salas), testáveis sem janela |
| `game/src/*_renderer.cpp`, `ui_*.cpp`, `hud.cpp` | Desenho e interface |
| `game/tests/` | Um executável de teste headless por domínio, mais os testes de renderização |
| `game/assets/` | Sprites, tiles e fonte |
| `game/web/shell.html`, `game/web/assets/` | A página web que hospeda o jogo e a arte da tela de carregamento |
| `game/vendor/raylib/`, `game/inc/` | Código de terceiros (raylib, raygui) |
| `tools/sprites/` | Scripts Python que geram os sprites do cavaleiro e dos inimigos |
| `docs/` | Arquitetura, notas de sistemas e decisões de projeto |

O jogo sempre renderiza em 1920×1080; a versão para navegador escala o canvas para caber na janela.

## Deploy

[`.github/workflows/deploy-web.yml`](.github/workflows/deploy-web.yml) compila o jogo para WebAssembly com o Emscripten a cada push na `main` e publica o resultado no GitHub Pages. O loop bloqueante do jogo roda no navegador via ASYNCIFY do Emscripten, devolvendo o controle à página a cada quadro de animação.

[`.github/workflows/release-windows.yml`](.github/workflows/release-windows.yml) roda quando uma tag `v*` é enviada: compila o `EndlessRooms.exe` autocontido, executa os testes headless e publica uma release com um zip contendo o executável e os assets.

## Créditos

Desenvolvido por [TiagoJSaraiva](https://github.com/TiagoJSaraiva) com [raylib](https://www.raylib.com) e [raygui](https://github.com/raysan5/raygui), de Ramon Santamaria, ambas sob a licença zlib/libpng (veja os avisos em `game/vendor/raylib` e `game/inc`).

**Fonte:** Alagard, de Hewett Tsoi.

**Sprites do cavaleiro e dos inimigos:** gerados pelos scripts de `tools/sprites/`.

<!-- TODO: origem e licença da arte restante (chão, paredes, props, itens) e licença do projeto. -->
