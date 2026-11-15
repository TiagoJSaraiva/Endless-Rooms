#include "game_window.h"

#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten.h>

// O loop do jogo é um while bloqueante. Com ASYNCIFY, esta espera suspende o loop e devolve o
// controle ao navegador até o próximo quadro, que então apresenta o canvas e entrega o input.
EM_ASYNC_JS(void, WaitForAnimationFrame, (), {
    await new Promise(resolve => requestAnimationFrame(resolve));
});
#endif

namespace {
constexpr int kDesktopTargetFps = 60;
}

namespace GameWindow {

void Open(int width, int height, const char* title) {
#if defined(PLATFORM_WEB)
    // Sem SetTargetFPS: na web a espera da raylib é ativa e travaria a página;
    // o requestAnimationFrame já sincroniza com a tela.
    InitWindow(width, height, title);
#else
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_VSYNC_HINT);
    InitWindow(width, height, title);
    const int monitorIndex = GetCurrentMonitor();
    const Vector2 monitorPosition = GetMonitorPosition(monitorIndex);
    SetWindowPosition(static_cast<int>(monitorPosition.x), static_cast<int>(monitorPosition.y));
    SetTargetFPS(kDesktopTargetFps);
#endif
}

bool NextFrame() {
#if defined(PLATFORM_WEB)
    // WindowShouldClose da raylib web dorme 16 ms fixos; a espera pelo quadro é mais suave.
    WaitForAnimationFrame();
    return true;
#else
    return !WindowShouldClose();
#endif
}

} // namespace GameWindow
