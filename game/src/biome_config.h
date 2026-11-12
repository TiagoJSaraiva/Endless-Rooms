#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "room_types.h"

namespace BiomeConfig {

// Biomas sorteáveis em uma run sem override. Caverna e mansão estão congeladas na demo:
// o código delas continua no projeto e ainda pode ser forçado por biome.txt.
const std::vector<BiomeType>& RandomBiomes();

// Converte o nome de um bioma selecionável em uma configuração opcional.
std::optional<BiomeType> ParseBiomeOverride(std::string_view value);

// Lê biome.txt do diretório de trabalho do jogo. Arquivo ausente ou vazio mantém o sorteio.
std::optional<BiomeType> LoadBiomeOverride(const char* path = "biome.txt");

} // namespace BiomeConfig
