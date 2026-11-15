#pragma once

// Abertura da janela e ritmo do loop principal, com as diferenças entre desktop e navegador.
namespace GameWindow {

// Abre a janela do jogo. No desktop: sem bordas, no monitor atual, limitada a 60 FPS.
// No navegador: canvas do tamanho pedido, com o ritmo dado pelo requestAnimationFrame.
void Open(int width, int height, const char* title);

// Chamada no início de cada volta do loop. Retorna false quando o jogo deve fechar.
// No navegador devolve o controle à página até o próximo quadro e nunca pede para fechar.
bool NextFrame();

} // namespace GameWindow
