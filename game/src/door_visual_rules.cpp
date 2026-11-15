#include "door_visual_rules.h"

#include "wall_rules.h"

namespace DoorVisualRules {

namespace {

// Folga entre a porta e o início da máscara, para a máscara não encostar no sprite da porta.
constexpr float kMaskClearance = 1.0f;
// Largura da transição entre a porta lateral e a parte totalmente escura do corredor.
constexpr float kHorizontalFadeWidth = static_cast<float>(TILE_SIZE);

} // namespace

std::optional<CorridorMask> CorridorMaskBehindDoor(Direction direction, Rectangle corridorPixels,
                                                   Rectangle doorHitbox) {
    Rectangle mask = corridorPixels;
    const float right = mask.x + mask.width;
    const float bottom = mask.y + mask.height;

    switch (direction) {
        case Direction::North: {
            const float doorBack = doorHitbox.y - kMaskClearance;
            mask.height = std::min(doorBack, bottom) - mask.y;
            if (mask.height <= 0.0f) return std::nullopt;
            return CorridorMask{mask, {}, false};
        }
        case Direction::South: {
            const float doorBack = doorHitbox.y + doorHitbox.height + kMaskClearance;
            mask.y = std::max(doorBack, mask.y);
            mask.height = bottom - mask.y;
            if (mask.height <= 0.0f) return std::nullopt;
            return CorridorMask{mask, {}, false};
        }
        case Direction::East:
        case Direction::West:
            break;
    }

    const bool doorOnLeft = direction == Direction::East;
    if (doorOnLeft) {
        mask.x = std::max(doorHitbox.x + doorHitbox.width + kMaskClearance, mask.x);
        mask.width = right - mask.x;
    } else {
        mask.width = std::min(doorHitbox.x - kMaskClearance, right) - mask.x;
    }
    if (mask.width <= 0.0f) return std::nullopt;

    // A parede superior do corredor sobe UpperWallHeightTiles acima do piso; a inferior é
    // desenhada sobre as duas últimas fileiras e termina na borda do piso.
    const float upperWallHeight = static_cast<float>(WallRules::UpperWallHeightTiles * TILE_SIZE);
    mask.y -= upperWallHeight;
    mask.height += upperWallHeight;

    const float fadeWidth = std::min(kHorizontalFadeWidth, mask.width);
    CorridorMask result{mask, mask, doorOnLeft};
    result.fade.width = fadeWidth;
    result.solid.width = mask.width - fadeWidth;
    if (doorOnLeft) {
        result.solid.x = mask.x + fadeWidth;
    } else {
        result.fade.x = mask.x + mask.width - fadeWidth;
    }
    return result;
}

} // namespace DoorVisualRules
