#include "debug_config.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iterator>
#include <string>

namespace DebugConfig {
namespace {

// Valores que ligam o modo debug. Comparados em minúsculas.
constexpr std::array<std::string_view, 4> kEnabledValues{"true", "1", "on", "yes"};

std::string Normalize(std::string_view value) {
    std::size_t first = 0;
    std::size_t last = value.size();
    while (first < last && std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }

    std::string normalized(value.substr(first, last - first));
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return normalized;
}

} // namespace

bool ParseDebugMode(std::string_view value) {
    const std::string normalized = Normalize(value);
    return std::find(kEnabledValues.begin(), kEnabledValues.end(), normalized) != kEnabledValues.end();
}

bool LoadDebugMode(const char* path) {
    std::ifstream config(path);
    if (!config) {
        return false;
    }
    const std::string value((std::istreambuf_iterator<char>(config)), std::istreambuf_iterator<char>());
    return ParseDebugMode(value);
}

} // namespace DebugConfig
