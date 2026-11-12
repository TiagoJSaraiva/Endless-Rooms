#pragma once

#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

#include "enemy_common.h"

class Room;

// Fabrica inimigos conforme bioma e configurações padrão.
class EnemySpawner {
public:
    EnemySpawner();

    // `normalRoomsCleared`: salas normais concluídas na run, para o escalonamento temporário
    // de dificuldade da demo (enemy_rules.h). O padrão 0 não escala nada.
    void SpawnEnemiesForRoom(Room& room,
                             std::vector<std::unique_ptr<Enemy>>& storage,
                             std::mt19937& rng,
                             int normalRoomsCleared = 0) const;

private:
    struct EnemyTemplate {
        EnemyConfig config{};
        float range{240.0f};
        const WeaponBlueprint* weapon{nullptr};
        EnemySpriteInfo sprite{};
        // Não nulo nos inimigos da Dungeon: ataque, carga e bolsas vêm de enemy_rules.
        const EnemyRules::EnemyDefinition* definition{nullptr};
    };

    std::unordered_map<BiomeType, std::vector<EnemyTemplate>> templates_;

    // Registra presets hardcoded utilizados na demo atual.
    void RegisterDefaults();
};
