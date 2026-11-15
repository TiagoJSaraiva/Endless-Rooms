#pragma once

#include "room.h"

#include <algorithm>
#include <optional>

namespace DoorVisualRules {

constexpr float FadeDuration = 1.0f;

inline float OpenProgress(const DoorInstance* state) {
    if (state == nullptr) return 0.0f;
    if (state->open) return 1.0f;
    if (!state->opening) return 0.0f;
    return std::clamp(state->fadeProgress / FadeDuration, 0.0f, 1.0f);
}

inline float MaskAlpha(const DoorInstance& state) {
    return 1.0f - OpenProgress(&state);
}

// Máscara escura sobre o corredor de uma porta fechada, em pixels de mundo.
// solid fica totalmente coberto. fade é a faixa junto da porta, transparente do lado da porta
// e opaca do lado do corredor; só existe em corredores horizontais (largura 0 nos verticais).
struct CorridorMask {
    Rectangle solid{};
    Rectangle fade{};
    bool doorOnLeft{false}; // a porta fica à esquerda da faixa: o alpha cresce para a direita
};

// A máscara começa logo atrás da porta. Em corredores horizontais cobre na vertical das paredes
// do corredor: do topo da parede superior até o fim da parede inferior (borda do piso).
// Vazio quando a porta já cobre todo o corredor.
std::optional<CorridorMask> CorridorMaskBehindDoor(Direction direction, Rectangle corridorPixels,
                                                   Rectangle doorHitbox);

} // namespace DoorVisualRules
