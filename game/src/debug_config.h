#pragma once

#include <string_view>

namespace DebugConfig {

// Interpreta o conteúdo de debug.txt. Aceita "true", "1", "on" e "yes" (sem diferenciar
// maiúsculas, ignorando espaços); qualquer outro valor, inclusive vazio, é desligado.
bool ParseDebugMode(std::string_view value);

// Lê debug.txt do diretório de trabalho do jogo. Arquivo ausente ou ilegível = desligado.
bool LoadDebugMode(const char* path = "debug.txt");

} // namespace DebugConfig
