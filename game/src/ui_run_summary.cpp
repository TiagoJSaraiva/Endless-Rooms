#include "ui_run_summary.h"

#include "font_manager.h"
#include "raylib.h"

#include <algorithm>
#include <string>

namespace {
constexpr Color kOverlayColor{8, 10, 16, 200};
constexpr Color kPanelColor{20, 26, 36, 240};
constexpr Color kPanelBorderColor{70, 92, 126, 240};
constexpr Color kTitleColor{230, 70, 70, 255};
constexpr Color kTextColor{235, 240, 252, 255};
constexpr Color kMutedTextColor{170, 182, 205, 255};
constexpr Color kButtonColor{40, 62, 98, 245};
constexpr Color kButtonHoverColor{70, 104, 160, 255};

constexpr float kPanelMaxWidth = 560.0f;
constexpr float kScreenMargin = 20.0f;
constexpr float kPanelPadding = 28.0f;
constexpr float kTitleSize = 48.0f;
constexpr float kTextSize = 24.0f;
constexpr float kLineSpacing = 8.0f;
constexpr float kSectionSpacing = 16.0f;
constexpr float kButtonHeight = 56.0f;
constexpr float kBorderThickness = 2.0f;
constexpr int kMaxVisibleItemLines = 6;
constexpr const char* kTitleText = "Voce perdeu!";
constexpr const char* kButtonLabel = "Voltar ao lobby";
}

bool DrawRunSummary(const RunStats& stats, int visitedRooms,
                    const std::vector<RunSummaryItemLine>& itemLines) {
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), kOverlayColor);

    const Font& font = GetGameFont();

    // Exibe no máximo kMaxVisibleItemLines; o restante vira "+N outros".
    const int totalItems = static_cast<int>(itemLines.size());
    const bool hasOverflow = totalItems > kMaxVisibleItemLines;
    const int visibleItems = hasOverflow ? kMaxVisibleItemLines - 1 : totalItems;
    const int hiddenItems = totalItems - visibleItems;

    const float lineHeight = kTextSize + kLineSpacing;
    // Linhas fixas: tempo, salas, inimigos, moedas, cabeçalho de itens.
    const int fixedLines = 5;
    const int itemRows = totalItems == 0 ? 1 : visibleItems + (hasOverflow ? 1 : 0);
    const float contentHeight = kTitleSize + kSectionSpacing + lineHeight * (fixedLines + itemRows) +
                                kSectionSpacing + kButtonHeight;

    // Escala o painel para telas pequenas sem estourar as margens.
    const float panelWidth = std::min(kPanelMaxWidth, screenWidth - kScreenMargin * 2.0f);
    const float panelHeight = std::min(contentHeight + kPanelPadding * 2.0f, screenHeight - kScreenMargin * 2.0f);
    Rectangle panel{(screenWidth - panelWidth) * 0.5f, (screenHeight - panelHeight) * 0.5f, panelWidth, panelHeight};
    DrawRectangleRec(panel, kPanelColor);
    DrawRectangleLinesEx(panel, kBorderThickness, kPanelBorderColor);

    const float left = panel.x + kPanelPadding;
    const float right = panel.x + panel.width - kPanelPadding;
    float y = panel.y + kPanelPadding;

    Vector2 titleMeasure = MeasureTextEx(font, kTitleText, kTitleSize, 0.0f);
    DrawTextEx(font, kTitleText, Vector2{panel.x + (panel.width - titleMeasure.x) * 0.5f, y}, kTitleSize, 0.0f,
               kTitleColor);
    y += kTitleSize + kSectionSpacing;

    auto drawRow = [&](const std::string& label, const std::string& value) {
        DrawTextEx(font, label.c_str(), Vector2{left, y}, kTextSize, 0.0f, kMutedTextColor);
        Vector2 valueMeasure = MeasureTextEx(font, value.c_str(), kTextSize, 0.0f);
        DrawTextEx(font, value.c_str(), Vector2{right - valueMeasure.x, y}, kTextSize, 0.0f, kTextColor);
        y += lineHeight;
    };

    drawRow("Tempo de run", RunStatsRules::FormatDuration(stats.elapsedSeconds));
    drawRow("Salas visitadas", std::to_string(visitedRooms));
    drawRow("Inimigos derrotados", std::to_string(stats.enemiesDefeated));
    drawRow("Moedas coletadas", std::to_string(stats.coinsCollected));

    DrawTextEx(font, "Itens obtidos", Vector2{left, y}, kTextSize, 0.0f, kMutedTextColor);
    y += lineHeight;
    if (totalItems == 0) {
        DrawTextEx(font, "Nenhum", Vector2{left + kPanelPadding * 0.5f, y}, kTextSize, 0.0f, kTextColor);
        y += lineHeight;
    } else {
        for (int i = 0; i < visibleItems; ++i) {
            const std::string text = itemLines[i].name + " x" + std::to_string(itemLines[i].quantity);
            DrawTextEx(font, text.c_str(), Vector2{left + kPanelPadding * 0.5f, y}, kTextSize, 0.0f, kTextColor);
            y += lineHeight;
        }
        if (hasOverflow) {
            const std::string text = "+" + std::to_string(hiddenItems) + " outros";
            DrawTextEx(font, text.c_str(), Vector2{left + kPanelPadding * 0.5f, y}, kTextSize, 0.0f,
                       kMutedTextColor);
        }
    }

    // O botão fica ancorado ao fim do painel, independente do número de linhas.
    Rectangle button{left, panel.y + panel.height - kPanelPadding - kButtonHeight, right - left, kButtonHeight};
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), button);
    DrawRectangleRec(button, hovered ? kButtonHoverColor : kButtonColor);
    DrawRectangleLinesEx(button, kBorderThickness, kPanelBorderColor);
    Vector2 labelMeasure = MeasureTextEx(font, kButtonLabel, kTextSize, 0.0f);
    DrawTextEx(font, kButtonLabel,
               Vector2{button.x + (button.width - labelMeasure.x) * 0.5f,
                       button.y + (button.height - labelMeasure.y) * 0.5f},
               kTextSize, 0.0f, kTextColor);
    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
