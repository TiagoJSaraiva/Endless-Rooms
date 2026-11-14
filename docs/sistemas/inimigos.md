# Inimigos da Dungeon

## Propósito
Três inimigos (Victim, Guard, Executioner) substituem `dungeon_melee` e `dungeon_ranged`. Cada um tem um padrão de ataque próprio: nenhum usa armas (blueprints de itens) do jogador. Metas numéricas em [balanceamento.md](balanceamento.md), seção "Atributos dos inimigos (1.7)".

## Componentes
- `enemy_rules.h/.cpp` (namespace `EnemyRules`): regras puras e a tabela de dados. `DungeonEnemies()` (atributos, ataque, bolsas, sprite), `EnemyCountForArea`, `ExecutionerCap`, `ComposeRoom`, `ScaleForClearedRooms`, `ClearedRooms` e a máquina de estados `AdvanceAttack`/`CanMove`/`CancelAttack`.
- `enemy_attacks.h/.cpp`: traduz um `AttackDefinition` em `ProjectileBlueprint` reaproveitando as famílias existentes. Dano escalonado entra por parâmetro; crítico é sempre 0.
- `enemy_common.*`: `EnemyCommon` ganhou um segundo construtor (`EnemyDefinition` + `damageMultiplier`). Com ele, `Update` usa a máquina de estados; sem ele (caverna e mansão) segue o caminho antigo por arma.
- `enemy_spawner.*`: monta os templates da Dungeon a partir da tabela e usa `ComposeRoom` na Dungeon.
- `tools/sprites/dungeon_enemies.py`: gera os sprites (ver "Sprites").

## Os inimigos

| Atributo | Victim | Guard | Executioner |
|---|---|---|---|
| Vida | 20 | 45 | 130 |
| Velocidade (px/s) | 170 | 140 | 90 |
| Colisor de movimento (raio, px) | 22 | 24 | 30 |
| Área de dano (largura × altura, px) | 36×84 | 40×87 | 53×105 |
| Altura de saída do ataque (px) | 36 (mão) | 52 (peito, lança da carga) | 43 (mãos) |
| Ataque | osso em linha reta (família munição arremessada) | estocada (família `Spear`, como a Machadinha) | giro (família `FullCircleSwing`, como a Espada Rúnica) |
| Dano | 5 | 10 | 16 por acerto |
| Início do ataque (px) | 480 | 120 | 130 |
| Alcance | osso 380 px/s até 600 px | reach 58 + length 62 = 120 px | raio 130 px |
| Carga parado | 0 | 0,35 s | 0,8 s (frame de carga) |
| Intervalo (início a início) | 1,6 s | 1,8 s | 4,0 s |
| Imunidade por alvo | n/a | 0,6 s | 0,6 s (máx. 2 acertos por giro de 1,2 s) |
| Peso de spawn | 1,0 | 1,0 | 0,35 |
| Bolsas small/medium/big (%) | 35/5/0 | 35/20/0 | 0/40/40 |
| Moedas esperadas | 7,1 | 11,6 | 36 |

Crítico 0 e mitigação própria 0 nos três. Colisão de parede e de props vale como nos demais inimigos; ossos param na parede (`SetPassableAreas`).

## Área de dano e saída dos ataques
Decisão [0010](../decisoes/0010-corpo-do-sprite-no-combate.md).

- `EnemyDefinition::body` (`BodyGeometry`, regras em `body_rules.h`) mede o corpo no sprite, sem a arma, a partir dos pés (`GetPosition()`). O colisor de movimento continua sendo o raio nos pés.
- Projéteis do jogador acertam o retângulo `GetHurtbox()` (base nos pés, centrado em x). Inimigos sem `body` (caverna e mansão) mantêm o círculo de `GetCollisionRadius` nos pés.
- Os ataques saem de `GetHandAnchor()` e miram `EnemyUpdateContext::playerAimPoint` (centro do corpo do jogador; sem ele, `playerPosition`). O alcance de início é medido entre esses dois pontos. A perseguição continua indo até `playerPosition`.
- O projétil leva `height = handHeight`: a parede corta osso pela sombra no chão (ver `colisao-de-salas.md`). Golpes que seguem o inimigo (estocada, giro) mantêm a altura das mãos enquanto ele anda.

## Máquina de estados do ataque
Fases: `Ready` → `Charging` → `Active` → `Ready`.
- Em `Ready`, com o jogador dentro de `startRange` e `cooldown == 0`, o ataque começa e o cooldown vira o intervalo (contado do início da carga, ou do disparo quando não há carga). Com carga, vai a `Charging` (parado, frame de carga, mira travada); sem carga, dispara na hora.
- `Charging` dura `chargeSeconds` e não é cancelada se o jogador sai do alcance (a carga é o aviso). Ao fim, o projétil é criado (`fire`).
- `Active` dura `activeSeconds` (estocada 0,47 s, giro 1,2 s; osso 0). O Guard fica parado; o Executioner persegue o jogador até 48 px (`kChaseContactDistance`) enquanto gira.
- A mira é travada no início da carga: sair da frente a tempo desvia.
- Se o jogador sai da sala, o ataque em curso é cancelado e o cooldown preservado.
- O Victim para ao entrar em 480 px e atira; não recua.
- Estocada e giro usam `ownerLifetime` do inimigo (ver [diagnostico-de-crashes.md](diagnostico-de-crashes.md)): morrem junto com ele.

## Spawn
- Inimigos por sala: `round(área em tiles × 0,02)`, mínimo 2 (de 2 a 8 nas salas de 10 a 20 tiles). Vale para todos os biomas.
- Dungeon: `ComposeRoom` sorteia pelos pesos (42%/42%/15% por inimigo). Teto de Executioner = `floor(n / 4)`; um sorteio acima do teto vira Victim ou Guard (1:1). Salas com menos de 4 inimigos não têm Executioner.
- Caverna e mansão (congeladas) mantêm seus templates e o sorteio por peso, só com a nova contagem por área.

## Escalonamento de dificuldade (temporário)
**Solução temporária da demo; a versão final não usará isto.** `k = max(0, salas normais concluídas − 5)`; inimigos que nascem depois têm vida × (1 + 0,05·k) e dano × (1 + 0,03·k) (`ScaleForClearedRooms`). Os inimigos de uma sala nascem ao entrar nela, então o multiplicador vale a partir da sala seguinte à 6ª concluída.

Sala normal concluída: em `main.cpp`, depois de remover os inimigos mortos, se o jogador está na sala, ela é do tipo Normal e a lista ficou vazia, a sala entra em `EnemyRules::ClearedRooms` (cada sala conta uma vez). O contador é zerado em `BeginNewRun` e passado a `EnemySpawner::SpawnEnemiesForRoom(room, storage, rng, normalRoomsCleared)`. Só afeta a Dungeon. Para remover: apagar a seção do escalonamento em `enemy_rules`, `ClearedRooms`, o parâmetro do spawner e as três linhas em `main.cpp`.

## Onde ajustar cada número
- Atributos, ataque, carga, intervalo, bolsas, pesos, tamanho de frame e corpo (`body`): tabela em `enemy_rules.cpp` (`kDungeonEnemies`).
- Contagem por área, mínimo, razão do teto, constantes do escalonamento: constantes no topo de `enemy_rules.cpp`.
- Tempos de extensão da estocada: `kThrust*Seconds` em `enemy_rules.cpp` e `enemy_attacks.cpp` (mantê-los iguais).
- Faixas de moedas por bolsa: `reward_pouch.cpp`.

## Sprites
`python tools/sprites/dungeon_enemies.py` (Pillow) regenera, de forma determinística, `game/assets/img/enemies/dungeon_{victim,guard,executioner}/` (`idle_sprite`, `walking_spritesheet` de 4 frames verticais, `charge_sprite` no Guard e no Executioner) e as imagens `bone.png`, `spear.png`, `axe.png`. Placeholders com potencial de ficar definitivos. Os sprites olham para a direita. As armas dos corpos (lança e machado) aparecem também nos projéteis durante o ataque.

Os corpos são desenhados em pixel art e ampliados por `SPRITE_SCALE` (1,4, vizinho mais próximo): Victim 48×87, Guard 56×98, Executioner 81×123. Os projéteis (osso, lança, machado) não são ampliados. Ao mudar a escala, rode o script e atualize na tabela de `enemy_rules.cpp` os tamanhos de quadro e o `body` (corpo opaco sem a arma e altura das mãos, já escalados); alcances e colisor de movimento são independentes do sprite. O cavaleiro segue a mesma escala em `tools/sprites/knight.py` (53×95, quadro e `appearance.body` em `player.cpp`).

## Testes
- `game/tests/enemy_rules_tests.cpp` (`mingw32-make test`): atributos da tabela, alcances e blueprints, moedas esperadas, contagem por área (todos os tamanhos pares 10..20), teto de Executioner em 2000 seeds, escalonamento, `ClearedRooms`, tempos de carga/intervalo da máquina de estados e corpos dentro do quadro do sprite.
- `game/tests/body_rules_tests.cpp`: geometria da área de dano, das mãos e do centro; corpo do cavaleiro.
- `game/tests/projectile_lifetime_tests.cpp`: ataques reais dos três inimigos e osso do Victim saindo da altura das mãos.
- Validar jogando: legibilidade da carga do Guard (0,35 s) e do Executioner (0,8 s), osso do Victim em grupo, giro que anda, barra de vida sobre os sprites, e o aumento de dificuldade a partir da 7ª sala. Osso, lança e machado saindo das mãos; tiros e golpes do jogador acertando cabeça e tronco.
