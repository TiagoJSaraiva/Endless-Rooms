#pragma once

#include "reward_pouch.h"

#include <functional>
#include <string>
#include <vector>

class Room;

// Item agregado da run: quantidade total efetivamente recebida das bolsas de recompensa.
struct RunStatsItem {
    int itemId{0};
    int quantity{0};
};

// Estatísticas de uma run, exibidas na tela de fim de run.
// Moedas e itens contam só o que foi entregue por recibos de coleta de bolsas.
struct RunStats {
    float elapsedSeconds{0.0f};
    int enemiesDefeated{0};
    int coinsCollected{0};
    // Ordem estável: a de primeira coleta de cada item.
    std::vector<RunStatsItem> items;
};

// Linha pronta para exibição (nome já resolvido).
struct RunSummaryItemLine {
    std::string name;
    int quantity{0};
};

namespace RunStatsRules {
// Rótulo usado quando o id do item não tem definição conhecida.
extern const char* const kUnknownItemName;

// Delta negativo é ignorado.
void AddTime(RunStats& stats, float deltaSeconds);
void AddEnemyDefeated(RunStats& stats);
// Recibos vazios não alteram nada; quantidades <= 0 são ignoradas.
void AddCollection(RunStats& stats, int coins, const std::vector<RewardPouchItem>& items);

// Salas visitadas, sem contar o lobby. Ponteiros nulos são ignorados.
int CountVisitedRooms(const std::vector<const Room*>& rooms);

// "mm:ss"; a partir de 1 hora vira "h:mm:ss". Frações são descartadas.
std::string FormatDuration(float seconds);

// Resolve o nome de um item; retorna nullptr se o id for desconhecido.
using ItemNameLookup = std::function<const char*(int itemId)>;
std::vector<RunSummaryItemLine> BuildItemLines(const RunStats& stats, const ItemNameLookup& lookup);
}
