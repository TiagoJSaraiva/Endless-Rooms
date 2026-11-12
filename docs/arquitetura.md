# Arquitetura

Visão do projeto como ele está **agora**. Atualize no mesmo passo de qualquer mudança em módulos, contratos de header, organização de pastas ou build.

## Visão geral

Endless Rooms é um roguelite 2D de ação e exploração: o jogador parte de um lobby, atravessa salas geradas por bioma (caverna, mansão, dungeon), enfrenta inimigos e coleta recursos para lojas, forjas e baús. C++17 com raylib 5.5.

O jogo usa módulos C++ organizados por domínio. `main.cpp` compõe os sistemas e executa o loop; os headers em `game/src/` são os contratos entre as áreas.

## Organização de pastas

```
AGENTS.md, CLAUDE.md    instruções para agentes
.agents/fluxos/         fluxos inicio / planejar / revisar / fim
.claude/                comandos e hook do Claude Code
scripts/                check.ps1 (verificação) e check-toolchain.ps1
docs/                   memória do projeto (este arquivo, status, sistemas, decisões)
tools/sprites/         geradores de sprites placeholder (Python + Pillow), um script por conjunto
anotacoes/              notas pessoais, ignoradas pelo Git
game/
  Makefile              compila raylib vendorizada, jogo e testes
  src/                  código do jogo (todo .cpp entra no build)
  tests/                um executável de teste por domínio
  assets/               sprites e fonte (caminhos relativos a game/)
  inc/                  raygui.h e wrappers rlcpp (terceiros)
  vendor/raylib/        raylib 5.5 (terceiros; preservar licença)
  obj/                  objetos de build (local, não versionado)
```

## Módulos

| Área | Responsabilidade |
|---|---|
| Salas | `room` guarda o estado; `room_manager` coordena navegação e geração; `room_features` prepara forja, loja e baús; `room_renderer` desenha a cena das salas; `room_types` define tipos comuns (`TileRect`, `BiomeType`, `TILE_SIZE`). |
| Biomas, chão e paredes | `biome_config` define os biomas sorteáveis (só dungeon na demo; caverna e mansão congeladas) e o override por `biome.txt`, que ainda aceita os três; `floor_rules`/`floor_tiles` e `wall_rules`/`wall_tiles` separam regras e desenho de chão e paredes; `door_visual_rules` define o visual das portas; `room_collision_rules` define os limites de movimento por bioma. |
| Props e luz | `prop` define o catálogo e o estado por instância; `prop_rules` isola geração, colisão, navegação e dano ambiental; `prop_renderer` desenha sprites e compõe iluminação; `lighting_rules` define a luminosidade por bioma e o nível da escuridão em tela cheia conforme a posição (decisão 0009). `wall_prop`/`wall_prop_renderer` fazem o mesmo para props de parede (tochas). |
| Jogador e combate | `player` guarda atributos; `enemy`, `enemy_common` e `enemy_spawner` cuidam dos inimigos; `enemy_rules` (regras puras: tabela dos inimigos da Dungeon, composição da sala, escalonamento temporário e máquina de ataque) e `enemy_attacks` (traduz a definição de ataque em blueprint de projétil) ficam fora da apresentação; `projectile` coordena ciclo de vida e criação; `projectile_melee` e `projectile_ranged` implementam as famílias (contratos internos em `projectile_internal.h`); `combat_geometry` fornece interseções, trajetos e a saída de trajetos das áreas livres (paredes param projéteis via `SetPassableAreas`); `weapon`/`weapon_blueprints` definem as armas. |
| Recompensas | `reward_pouch` declara pools e isola sorteios; `reward_pouch_operations` integra mortes/destruição ao spawn e entrega recompensas; `reward_pouch_renderer` administra texturas e desenho. |
| Inventário | `inventory_rules` contém políticas de categorias, pilhas e preços; `inventory_operations` manipula slots e armazenamento; `ui_inventory` liga essas operações aos controles e desenha as telas de inventário, loja, forja e baú. |
| Interface e recursos | `hud` desenha informações de jogo; `font_manager` administra a fonte; `chest` define os tipos de armazenamento; `debug_console` implementa comandos de depuração; `run_stats` acumula as estatísticas da run (regras puras) e `ui_run_summary` desenha o resumo de fim de run. |
| Integração | `main.cpp` conecta os módulos e processa o ciclo do jogo; `station_persistence` sincroniza forjas e lojas entre UI e salas. |

Notas detalhadas por sistema ficam em [`sistemas/`](sistemas/README.md).

## Estado por partida

- Salas guardam props, bolsas e RNG de loot durante toda a run; tudo é recriado em uma nova partida. Não há persistência em disco.
- Instâncias de props não são adicionadas, removidas nem reordenadas durante o combate: seus endereços identificam alvos de ataques.
- Projéteis que seguem um dono dinâmico usam `ownerLifetime` (ver [diagnóstico de crashes](sistemas/diagnostico-de-crashes.md)).

## Build, testes e execução

- `game/Makefile` compila a raylib vendorizada e todos os `.cpp` de `game/src/`. Flags: `-Wall -std=c++17`; `BUILD_MODE=DEBUG` adiciona `-g -O0 -DENDLESS_DEBUG_DIAGNOSTICS`.
- `mingw32-make test` compila e executa os testes headless; os testes de renderização (`test-floor-render`, `test-wall-render`, `test-prop-render`, `test-wall-prop-render`, `test-reward-pouch-render`) abrem uma janela oculta.
- `scripts/check.ps1` encadeia tudo isso (decisão [0005](decisoes/0005-pipeline-de-verificacao.md)).
- Os assets usam caminhos relativos a `assets/`; execute o binário com `game/` como diretório de trabalho.

## Direção de evolução

Mantenha a diferença entre dados/regras, operações e apresentação (decisão [0007](decisoes/0007-regras-operacoes-renderizacao.md)). `main.cpp` concentra integração e ainda desenha parte da cena; extraia responsabilidades dele uma por vez, preservando as APIs dos headers existentes e validando os fluxos afetados antes de alterar gameplay.
