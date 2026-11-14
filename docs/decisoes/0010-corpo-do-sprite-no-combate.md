# 0010 — Corpo do sprite como área de dano e origem dos ataques

- **Data**: 2026-11
- **Status**: aceita

## Contexto
Com os sprites ampliados em 1,4×, as áreas de dano (círculos nos pés dos inimigos e 16 px acima dos pés do cavaleiro) passaram a cobrir só a parte de baixo do corpo, e os ataques saíam da posição lógica, rente ao chão. A posição lógica dos personagens também serve para o colisor de movimento, que precisa continuar pequeno e nos pés para portas, corredores e props.

## Decisão
- Cada personagem tem um `BodyGeometry` (`body_rules.h`), medido no sprite a partir dos pés: largura e altura do corpo sem a arma e altura das mãos no gesto de ataque.
- A área de dano é o retângulo do corpo, com a base nos pés. O colisor de movimento não muda.
- Ataques saem das mãos e miram o centro do corpo do alvo (inimigos) ou o mouse (jogador). Golpes que seguem o dono preservam o deslocamento entre a origem e a posição dele.
- Projéteis levam a altura da origem (`ProjectileSpawnContext::height`), e a parede os corta pela sombra no chão, não pelo ponto desenhado.
- As medidas ficam nas tabelas de dados (`enemy_rules.cpp`, `player.cpp`). Personagens sem corpo medido mantêm o círculo e a origem antigos.

## Alternativas descartadas
- **Escalar os círculos em 1,4×**: continua cobrindo os pés e não o tronco.
- **Projétil lógico no chão e só desenhado na altura das mãos**: exigiria separar posição de desenho e de colisão em todas as famílias de projétil, e o acerto não bateria com o que se vê no corpo retangular.
- **Ponto das mãos guardado no dono e apontado por `followTarget`**: mais um ponteiro cru e um frame de atraso em relação ao movimento.

## Consequências
Acertos batem com o que aparece na tela. Ao mudar a escala ou a arte de um sprite, é preciso remedir o `body`. O corte nas paredes depende da altura informada no disparo: um ataque novo que esqueça de informá-la é cortado pelo ponto desenhado, como antes. O `displayOffset` das armas do jogador passa a ser relativo às mãos.
