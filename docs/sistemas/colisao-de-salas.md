# Colisão nas salas

O limite de movimento termina alguns pixels antes da borda visual inferior da sala. Os valores ficam separados por bioma em `game/src/room_collision_rules.cpp`; atualmente, todos usam 15 px e podem ser ajustados individualmente.

Ao contornar props, a navegação inimiga ajusta os pontos de cada tile à área em que sua caixa de colisão cabe. Isso mantém a última linha de tiles acessível, mesmo quando o centro nominal do tile deixaria parte da caixa além do recuo inferior.

## Projéteis e paredes

Munição (flechas, ossos, balas) para na parede e lasers terminam nela, tanto os do jogador quanto os dos inimigos. Golpes corpo a corpo não são afetados.

- Área livre = união dos pisos das salas (`Room::Layout().tileBounds`) com os corredores (`Doorway::corridorTiles`) de portas não seladas e abertas (`doorState->open`). Porta fechada bloqueia. Calculada por `RoomCollisionRules::ProjectilePassableAreas` e entregue a cada frame pelo `main.cpp` com `ProjectileSystem::SetPassableAreas` (lista vazia = sem limite, o padrão em testes).
- O contato é pelo centro do projétil. Só há corte quando o trajeto sai da área partindo de dentro (`CombatGeometry::ExitFraction`); um projétil que já começa fora não é cortado, para não expirar tiros disparados encostados na parede.
- Munição: ao sair, vai ao ponto de saída e entra no frame terminal, então acertos antes da parede ainda contam e ela expira no `Update` seguinte. Laser: o comprimento do feixe (desenho e dano) é limitado ao ponto de saída, recalculado a cada frame.
- Testes: `projectile_wall_tests.cpp` (geometria, munição e laser) e `room_collision_rules_tests.cpp` (áreas livres).

## Verificação manual

Execute o jogo a partir de `game/` e, em uma sala de cada bioma sem saída ao sul, caminhe para baixo até a borda. O jogador deve parar antes de cruzar o limite visual inferior. Em uma sala com porta ao sul aberta, confirme também que ainda é possível atravessar para a sala seguinte.

Em uma dungeon com inimigos vivos, posicione o jogador junto à parede inferior, fora do alcance de ataque e com uma jaula ou mesa bloqueando o caminho direto do inimigo. Ele deve contornar o prop e continuar até entrar no alcance de ataque. Repita com o caminho direto livre e movendo o jogador pela borda inferior; o inimigo não deve parar longe do jogador nem atravessar props. Confira novamente a passagem por uma porta sul aberta após o combate.

As regressões automatizadas de navegação e perseguição ficam em `prop_rules_tests.cpp` e `prop_gameplay_tests.cpp` e são executadas por `mingw32-make test` dentro de `game/`.

Para projéteis: dispare flechas e lasers contra as paredes de uma sala; eles devem parar na parede, sem atingir inimigos do outro lado. Com a porta aberta o tiro deve passar pelo corredor até a sala seguinte, e com a porta fechada (inimigos vivos) deve parar nela. Confira também tiros de inimigos contra o jogador encostado na parede.
