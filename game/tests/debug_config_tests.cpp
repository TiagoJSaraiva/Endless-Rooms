#include "debug_config.h"

#include <cassert>
#include <cstdio>
#include <fstream>

int main() {
    assert(DebugConfig::ParseDebugMode("true"));
    assert(DebugConfig::ParseDebugMode("TRUE"));
    assert(DebugConfig::ParseDebugMode("  On \r\n"));
    assert(DebugConfig::ParseDebugMode("1"));
    assert(DebugConfig::ParseDebugMode("yes"));

    assert(!DebugConfig::ParseDebugMode(""));
    assert(!DebugConfig::ParseDebugMode("   \n"));
    assert(!DebugConfig::ParseDebugMode("false"));
    assert(!DebugConfig::ParseDebugMode("0"));
    assert(!DebugConfig::ParseDebugMode("off"));
    assert(!DebugConfig::ParseDebugMode("talvez"));

    // Arquivo ausente = desligado.
    assert(!DebugConfig::LoadDebugMode("arquivo_que_nao_existe_debug.txt"));

    const char* path = "debug_config_tests_tmp.txt";
    {
        std::ofstream file(path);
        file << "true\n";
    }
    assert(DebugConfig::LoadDebugMode(path));
    {
        std::ofstream file(path);
        file << "";
    }
    assert(!DebugConfig::LoadDebugMode(path));
    std::remove(path);

    return 0;
}
