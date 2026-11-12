#pragma once

#include "run_stats.h"

#include <vector>

// Desenha o resumo de fim de run sobre a tela inteira e retorna true no frame em que o
// botão "Voltar ao lobby" é clicado. Só apresenta os dados recebidos; não decide regra.
bool DrawRunSummary(const RunStats& stats, int visitedRooms,
                    const std::vector<RunSummaryItemLine>& itemLines);
