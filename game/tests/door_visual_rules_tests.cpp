#include "door_visual_rules.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace DoorVisualRules;

namespace {

constexpr float kTile = static_cast<float>(TILE_SIZE);

bool Near(float a, float b) {
    return std::fabs(a - b) < 0.001f;
}

float Right(Rectangle r) { return r.x + r.width; }
float Bottom(Rectangle r) { return r.y + r.height; }

// Corredor horizontal de 4 tiles x 2 tiles, começando em (10, 4).
const Rectangle kHorizontalCorridor{10 * kTile, 4 * kTile, 4 * kTile, 2 * kTile};

// Porta leste: a porta fica à esquerda do corredor e a transição começa logo atrás dela.
void CheckEastDoorMask() {
    const Rectangle door{10 * kTile - 20, 4 * kTile, 30, 2 * kTile};
    const auto mask = CorridorMaskBehindDoor(Direction::East, kHorizontalCorridor, door);
    assert(mask.has_value());
    assert(mask->doorOnLeft);
    const float maskLeft = Right(door) + 1.0f;
    assert(Near(mask->fade.x, maskLeft));
    assert(Near(mask->fade.width, kTile));
    assert(Near(mask->solid.x, Right(mask->fade)));
    assert(Near(Right(mask->solid), Right(kHorizontalCorridor)));
    // Cobre do topo da parede superior (2 tiles acima do piso) até a borda inferior do piso.
    for (const Rectangle& part : {mask->fade, mask->solid}) {
        assert(Near(part.y, 2 * kTile));
        assert(Near(Bottom(part), 6 * kTile));
    }
}

// Porta oeste: espelho da leste, com a transição na borda direita.
void CheckWestDoorMask() {
    const Rectangle door{14 * kTile - 10, 4 * kTile, 30, 2 * kTile};
    const auto mask = CorridorMaskBehindDoor(Direction::West, kHorizontalCorridor, door);
    assert(mask.has_value());
    assert(!mask->doorOnLeft);
    assert(Near(Right(mask->fade), door.x - 1.0f));
    assert(Near(mask->fade.width, kTile));
    assert(Near(mask->solid.x, kHorizontalCorridor.x));
    assert(Near(Right(mask->solid), mask->fade.x));
    assert(Near(mask->solid.y, 2 * kTile) && Near(Bottom(mask->solid), 6 * kTile));
}

// Corredores verticais mantêm a máscara só sobre o piso, sem transição.
void CheckVerticalDoorMasks() {
    const Rectangle corridor{10 * kTile, 4 * kTile, 2 * kTile, 3 * kTile};
    const Rectangle northDoor{10 * kTile, 7 * kTile - 20, 2 * kTile, 30};
    const auto north = CorridorMaskBehindDoor(Direction::North, corridor, northDoor);
    assert(north.has_value());
    assert(Near(north->fade.width, 0.0f));
    assert(Near(north->solid.y, corridor.y) && Near(Bottom(north->solid), northDoor.y - 1.0f));
    assert(Near(north->solid.x, corridor.x) && Near(north->solid.width, corridor.width));

    const Rectangle southDoor{10 * kTile, 4 * kTile - 10, 2 * kTile, 30};
    const auto south = CorridorMaskBehindDoor(Direction::South, corridor, southDoor);
    assert(south.has_value());
    assert(Near(south->fade.width, 0.0f));
    assert(Near(south->solid.y, Bottom(southDoor) + 1.0f) && Near(Bottom(south->solid), Bottom(corridor)));
}

// Porta que cobre o corredor inteiro não gera máscara; corredor curto não estoura a transição.
void CheckDegenerateMasks() {
    const Rectangle wideDoor{9 * kTile, 4 * kTile, 6 * kTile, 2 * kTile};
    assert(!CorridorMaskBehindDoor(Direction::East, kHorizontalCorridor, wideDoor).has_value());
    assert(!CorridorMaskBehindDoor(Direction::West, kHorizontalCorridor, wideDoor).has_value());

    const Rectangle shortCorridor{10 * kTile, 4 * kTile, kTile * 0.5f, 2 * kTile};
    const Rectangle door{10 * kTile - 20, 4 * kTile, 10, 2 * kTile};
    const auto mask = CorridorMaskBehindDoor(Direction::East, shortCorridor, door);
    assert(mask.has_value());
    assert(Near(Right(mask->fade), Right(shortCorridor)));
    assert(mask->solid.width <= 0.001f);
}

} // namespace

int main() {
    CheckEastDoorMask();
    CheckWestDoorMask();
    CheckVerticalDoorMasks();
    CheckDegenerateMasks();
    std::cout << "Door visual rules tests passed" << std::endl;
    return 0;
}
