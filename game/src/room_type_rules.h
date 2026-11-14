#pragma once

#include "room_types.h"

#include <vector>

namespace RoomTypeRules {

// Peso relativo de cada tipo de sala sorteável.
struct RoomTypeWeight {
    RoomType type;
    double weight;
};

// Tabela vigente; os pesos somam 100 e se leem como porcentagem por sala gerada.
const std::vector<RoomTypeWeight>& Weights();

// Loja, forja e baú.
bool IsSpecial(RoomType type);

// Sorteia um tipo com `roll` em [0, 1). Um tipo especial presente em `neighborTypes` (salas já
// geradas nas quatro coordenadas vizinhas) fica fora do sorteio, e o peso dele se redistribui
// entre os demais. Como toda sala nova confere as já geradas, especiais iguais nunca ficam vizinhas.
RoomType Pick(double roll, const std::vector<RoomType>& neighborTypes,
              const std::vector<RoomTypeWeight>& weights = Weights());

}  // namespace RoomTypeRules
