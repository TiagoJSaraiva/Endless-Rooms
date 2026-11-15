#pragma once

// Abertura da janela e ritmo do loop principal, com as diferenças entre desktop e navegador.
namespace GameWindow {

// Abre a janela do jogo. No desktop: sem bordas, no monitor atual, limitada a 60 FPS.
// No navegador: canvas do tamanho pedido, com o ritmo dado pelo requestAnimationFrame.
void Open(int width, int height, const char* title);

// Chamada no início de cada volta do loop. Retorna false quando o jogo deve fechar.
// No navegador devolve o controle à página até o próximo quadro e nunca pede para fechar.
// noexcept é obrigatório na web: com -fexceptions, uma chamada que pode lançar sai de main()
// por um invoke_* em JS, que o ASYNCIFY não atravessa; main() deixaria de ser instrumentada
// e recomeçaria do zero a cada quadro (tela preta). Pelo mesmo motivo, game_window.cpp é
// compilado sem exceções na web (Makefile).
bool NextFrame() noexcept;

} // namespace GameWindow
