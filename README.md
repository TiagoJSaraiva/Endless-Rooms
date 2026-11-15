<h1 align="center">Endless Rooms</h1>

<p align="center">
  <strong>Fight your way through procedurally generated dungeon rooms, gear up and see how far you get.</strong><br>
  A 2D action roguelite made with C++17 and raylib. Play the demo in your browser or download it for Windows.
</p>

<p align="center">
  <a href="https://tiagojsaraiva.github.io/Endless-Rooms/"><strong>▶ Play now</strong></a>
  &nbsp;·&nbsp;
  <a href="https://github.com/TiagoJSaraiva/Endless-Rooms/releases/latest"><strong>⬇ Download for Windows</strong></a>
  &nbsp;·&nbsp;
  <a href="LEIA-ME.md">Leia em português</a>
</p>

> The in-game text is in Brazilian Portuguese.

## How to play

| Input | Action |
|---|---|
| W, A, S, D | Move |
| Left click | Attack with the left-hand weapon |
| Right click | Attack with the right-hand weapon |
| E | Open doors, use shops, forges and chests |
| Tab or I | Inventory |
| 1 to 5 | Abilities of the equipped items |

You start in the lobby, next to a training dummy. Walk through a door to enter the dungeon: every room is generated as you go, and its doors stay shut until all of its enemies are defeated. Enemies drop reward pouches with coins and items. Rooms get more dangerous the more of them you clear.

Death ends the run. A summary shows the time survived, rooms visited, enemies defeated and the loot you collected, and takes you back to the lobby for a fresh run with a new map.

### Enemies

| Enemy | Behavior |
|---|---|
| Victim | Fast and fragile; throws bones from a distance |
| Guard | Armored skeleton with a spear; a short charge, then a thrust |
| Executioner | Slow and tough; winds up and spins a huge axe around itself. Rarer than the others |

### Special rooms

| Room | What it does |
|---|---|
| Shop | Buy potions and gear, or sell items for 30% of their value |
| Forge | Invest coins for a chance to forge an item from a recipe; more coins, better odds |
| Chest | Loot found along the way |

### Weapons

Each hand holds one weapon, so you can pair a melee weapon with a ranged one. Better weapons are rarer.

| Weapon | Rarity | Style |
|---|---|---|
| Short Sword | 2 | Quick slashes |
| Hatchet | 2 | Slower, heavier hits |
| Simple Bow | 2 | Fast arrows at range |
| Oak Staff | 3 | Short laser bursts |
| Buckler | 3 | Shield bash; also grants defense |
| Runic Sword | 5 | Spins around you, hitting everything close |

The demo covers the Dungeon biome with the knight as the only playable character, and it has no boss.

## Running locally

Windows, with [MinGW-w64](https://www.mingw-w64.org) (`g++`, `gcc`, `ar`) and GNU Make (`mingw32-make`) in the `PATH`. raylib 5.5 is bundled in `game/vendor/raylib` and is compiled along with the game.

```powershell
cd game
mingw32-make game
.\game.exe
```

Run the game from inside `game/`: asset paths are relative to that folder.

| Command (inside `game/`) | What it does |
|---|---|
| `mingw32-make game` | Builds the game (`game.exe`) |
| `mingw32-make game PORTABLE=TRUE` | Self-contained build with no MinGW DLLs and no console, as shipped in the releases |
| `mingw32-make test` | Builds and runs the headless tests |
| `mingw32-make clean` | Removes build outputs |
| `make game PLATFORM=PLATFORM_WEB` | Web build in `build-web/` (requires an active [Emscripten SDK](https://emscripten.org)) |

From the repository root, `scripts/check.ps1` runs the whole verification pipeline: toolchain, build, headless tests and render tests. More details in [INSTRUCTION.md](INSTRUCTION.md) (in Portuguese).

## Structure

| Path | Contents |
|---|---|
| `game/src/main.cpp` | Game loop: wires all systems together |
| `game/src/game_window.cpp` | Window setup and frame pacing for desktop and browser |
| `game/src/*_rules.cpp` | Game rules (enemies, weapons, economy, lighting, rooms), testable without a window |
| `game/src/*_renderer.cpp`, `ui_*.cpp`, `hud.cpp` | Drawing and user interface |
| `game/tests/` | One headless test executable per domain, plus render tests |
| `game/assets/` | Sprites, tiles and font |
| `game/web/shell.html`, `game/web/assets/` | The web page that hosts the game and its loading screen art |
| `game/vendor/raylib/`, `game/inc/` | Third-party code (raylib, raygui) |
| `tools/sprites/` | Python scripts that generate the knight and enemy sprites |
| `docs/` | Architecture, system notes and design decisions (in Portuguese) |

The game always renders at 1920×1080; the browser version scales the canvas to fit the window.

## Deploy

[`.github/workflows/deploy-web.yml`](.github/workflows/deploy-web.yml) compiles the game to WebAssembly with Emscripten on every push to `main` and publishes it to GitHub Pages. The blocking game loop runs in the browser through Emscripten's ASYNCIFY, yielding to the page on every animation frame.

[`.github/workflows/release-windows.yml`](.github/workflows/release-windows.yml) runs when a `v*` tag is pushed: it builds the self-contained `EndlessRooms.exe`, runs the headless tests and publishes a release with a zip containing the executable and its assets.

## Credits

Developed by [TiagoJSaraiva](https://github.com/TiagoJSaraiva) with [raylib](https://www.raylib.com) and [raygui](https://github.com/raysan5/raygui) by Ramon Santamaria, both under the zlib/libpng license (see the notices in `game/vendor/raylib` and `game/inc`).

**Font:** Alagard, by Hewett Tsoi.

**Knight and enemy sprites:** generated by the scripts in `tools/sprites/`.

<!-- TODO: origin and license of the remaining art (floor, walls, props, items) and the project license. -->
