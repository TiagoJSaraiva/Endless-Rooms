# Convenções de C++

Padrões que o código atual já segue. Código novo deve segui-los; código antigo que diverge não é corrigido por conta própria, só sugerido no resumo.

## Linguagem e build

- C++17 (`-Wall -std=c++17`). Código novo não deve introduzir avisos.
- Headers com `#pragma once`. Inclua `raylib.h` e os headers do projeto antes dos da biblioteca padrão, separados por linha em branco.
- Não use `using namespace` em headers.
- Todo `.cpp` em `game/src/` entra no build do jogo automaticamente; testes precisam de alvo explícito no Makefile (decisão [0006](decisoes/0006-testes-sem-framework.md)).

## Nomes

| Elemento | Padrão | Exemplo |
|---|---|---|
| Tipos (`class`, `struct`, `enum class`) | PascalCase | `RewardPouchInstance`, `BiomeType` |
| Funções e métodos | PascalCase | `RoomBoundsForCollision`, `Hitbox()` |
| Membros públicos de `struct` | camelCase | `chancePercent`, `hitboxSize` |
| Membros privados de `class` | camelCase com `_` final | `anchorX_`, `slots_` |
| Constantes locais a um `.cpp` | `k` + PascalCase | `kMinSpawnRate` |
| Constantes globais de configuração | MAIÚSCULAS | `TILE_SIZE`, `SCREEN_WIDTH` |
| Namespaces de regras | `<Dominio>Rules` | `RoomCollisionRules`, `RewardPouchRules` |
| Arquivos | snake_case | `room_collision_rules.cpp` |

- Prefira `enum class` a `enum`.
- Inicialize membros na declaração com chaves (`int coins{0};`).
- Helpers internos de um `.cpp` ficam em `namespace { }` anônimo.

## Organização por camadas

Decisão [0007](decisoes/0007-regras-operacoes-renderizacao.md):

- **Regras** (`<dominio>_rules`, ou funções no namespace `<Dominio>Rules`): sem desenho, sem estado global, sem `GetFrameTime()`/input; recebem tudo por parâmetro, inclusive RNG (`std::mt19937_64&`) e rolagens já sorteadas quando isso facilita o teste.
- **Operações** (`<dominio>_operations`): aplicam regras a salas, jogador e inventário.
- **Apresentação** (`<dominio>_renderer`, `ui_*`, `hud`): texturas, cache de assets e desenho. Não decidem regra de jogo.
- Valores de catálogo (pools, catálogos de props, configs de inimigo) ficam em tabelas de dados no `.cpp` do domínio, não espalhados em `if`/`switch`.
- Sem números mágicos: nomeie constantes; valores por bioma/tipo vão para a tabela correspondente.

## Headers como contratos

- Headers públicos em `game/src/` são contratos entre módulos. Mudança de assinatura é mudança de contrato: cite no plano e no resumo.
- Contratos internos de uma família de arquivos ficam em um header `_internal.h` (ex.: `projectile_internal.h`).
- Documente no header pré-condições e tempo de vida que não são óbvios pelo tipo (quem é dono, quando um ponteiro deixa de valer). Ponteiros para objetos de vida dinâmica precisam de um mecanismo de validade explícito, como `ownerLifetime` em projéteis.
- Prefira declarações antecipadas (`class Room;`) a includes em headers quando bastar.

## Memória e tempo de vida

- Sem `new`/`delete` manuais em código novo: use valores, `std::vector`, `std::unique_ptr` ou `std::shared_ptr`/`std::weak_ptr` quando a propriedade for compartilhada.
- Não guarde ponteiros ou referências para elementos de `std::vector` que possa realocar. Coleções cujos endereços identificam alvos (props de sala) não são reordenadas durante a partida.
- Texturas são carregadas uma vez e cacheadas pelo renderizador do domínio; descarregue-as no encerramento correspondente.

## Comentários e idioma

- Comentários explicam **por quê** e contratos, não o quê. Comentários em português.
- Identificadores e nomes de arquivo em inglês.
- Logs de diagnóstico ficam atrás de `ENDLESS_DEBUG_DIAGNOSTICS` (definido por `BUILD_MODE=DEBUG`).

## Testes

- Um executável por domínio: `game/tests/<dominio>_tests.cpp`, com `main()`, `assert` e uma linha `... tests passed.` no fim.
- Teste regras sem janela sempre que possível. Testes que precisam de contexto gráfico vão para `<dominio>_render_tests.cpp` com alvo `test-<dominio>-render`.
- Comparações de `float` resultantes de cálculo usam tolerância; igualdade exata só para valores construídos exatamente.
- RNG nos testes usa seed fixa.
- Novo executável: adicionar binário, alvo, dependência em `test` (ou alvo de render), entrada em `clean` e linha em `game/.gitignore`.
