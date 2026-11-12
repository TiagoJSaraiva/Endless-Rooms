#include "enemy_spawner.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <string>

#include "raymath.h"

#include "enemy_rules.h"
#include "room.h"
#include "prop_rules.h"
#include "weapon_blueprints.h"

namespace {

// Converte coordenadas de tiles para pixels ao calcular áreas de spawn.
Rectangle TileRectToPixels(const TileRect& rect) {
    return Rectangle{
        static_cast<float>(rect.x * TILE_SIZE),
        static_cast<float>(rect.y * TILE_SIZE),
        static_cast<float>(rect.width * TILE_SIZE),
        static_cast<float>(rect.height * TILE_SIZE)
    };
}

// Helper para montar EnemyConfig legível.
EnemyConfig MakeConfig(int id,
                       const std::string& name,
                       BiomeType biome,
                       float maxHealth,
                       float speed,
                       float spawnRate,
                       float collisionRadius) {
    EnemyConfig config{};
    config.id = id;
    config.name = name;
    config.biome = biome;
    config.maxHealth = maxHealth;
    config.speed = speed;
    config.spawnRate = spawnRate;
    config.collisionRadius = collisionRadius;
    return config;
}

// Preenche dados de spritesheet a partir da pasta base.
EnemySpriteInfo MakeSpriteInfo(const std::string& basePath) {
    EnemySpriteInfo info{};
        info.idleSpritePath = basePath + "/idle_sprite";
        info.walkingSpriteSheetPath = basePath + "/walking_spritesheet";
    info.frameWidth = 38;
    info.frameHeight = 68;
    info.frameCount = 4;
    info.secondsPerFrame = 0.16f;
    return info;
}

// Sprites dos inimigos com definição própria: tamanho de frame e frame de carga vêm da tabela.
EnemySpriteInfo MakeDefinedSpriteInfo(const EnemyRules::EnemyDefinition& definition) {
    const EnemyRules::SpriteDefinition& sprite = definition.sprite;
    EnemySpriteInfo info{};
    info.idleSpritePath = std::string(sprite.folder) + "/idle_sprite";
    info.walkingSpriteSheetPath = std::string(sprite.folder) + "/walking_spritesheet";
    if (definition.attack.chargeSeconds > 0.0f) {
        info.chargeSpritePath = std::string(sprite.folder) + "/charge_sprite";
    }
    info.frameWidth = sprite.frameWidth;
    info.frameHeight = sprite.frameHeight;
    info.frameCount = sprite.walkFrameCount;
    info.secondsPerFrame = sprite.secondsPerFrame;
    return info;
}

} // namespace

EnemySpawner::EnemySpawner() {
    RegisterDefaults();
}

// Registra os tipos básicos de inimigos por bioma.
void EnemySpawner::RegisterDefaults() {
    templates_.clear();

    // Caverna
    EnemyTemplate caveRanged{};
    caveRanged.config = MakeConfig(100, "caverna_ranged", BiomeType::Cave, 21.0f, 82.5f, 1.0f, 22.0f);
    caveRanged.config.rewardPouchDrops = {{RewardPouchType::Small, 60}, {RewardPouchType::Medium, 10}};
    caveRanged.range = 520.0f;
    caveRanged.weapon = &GetArcoSimplesWeaponBlueprint();
        caveRanged.sprite = MakeSpriteInfo("./assets/img/enemies/caverna_ranged");
    templates_[BiomeType::Cave].push_back(caveRanged);

    EnemyTemplate caveMelee{};
    caveMelee.config = MakeConfig(101, "caverna_melee", BiomeType::Cave, 40.0f, 95.0f, 1.2f, 24.0f);
    caveMelee.config.rewardPouchDrops = {{RewardPouchType::Small, 60}, {RewardPouchType::Medium, 10}};
    caveMelee.range = 140.0f;
    caveMelee.weapon = &GetEspadaCurtaWeaponBlueprint();
        caveMelee.sprite = MakeSpriteInfo("./assets/img/enemies/caverna_melee");
    templates_[BiomeType::Cave].push_back(caveMelee);

    // Dungeon: os três inimigos vêm da tabela de enemy_rules, na ordem de EnemyKind.
    for (const EnemyRules::EnemyDefinition& definition : EnemyRules::DungeonEnemies()) {
        EnemyTemplate dungeonEnemy{};
        dungeonEnemy.config = MakeConfig(definition.id, definition.name, BiomeType::Dungeon, definition.maxHealth,
                                         definition.speed, definition.spawnWeight, definition.collisionRadius);
        dungeonEnemy.config.rewardPouchDrops = EnemyRules::DropTable(definition.pouches);
        dungeonEnemy.range = definition.attack.startRange;
        dungeonEnemy.definition = &definition;
        dungeonEnemy.sprite = MakeDefinedSpriteInfo(definition);
        templates_[BiomeType::Dungeon].push_back(dungeonEnemy);
    }

    // Mansao
    EnemyTemplate mansionRanged{};
    mansionRanged.config = MakeConfig(120, "mansao_ranged", BiomeType::Mansion, 30.0f, 87.5f, 1.1f, 22.0f);
    mansionRanged.config.rewardPouchDrops = {{RewardPouchType::Small, 40}, {RewardPouchType::Medium, 30}, {RewardPouchType::Big, 10}};
    mansionRanged.range = 540.0f;
    mansionRanged.weapon = &GetArcoSimplesWeaponBlueprint();
        mansionRanged.sprite = MakeSpriteInfo("./assets/img/enemies/mansao_ranged");
    templates_[BiomeType::Mansion].push_back(mansionRanged);

    EnemyTemplate mansionMelee{};
    mansionMelee.config = MakeConfig(121, "mansao_melee", BiomeType::Mansion, 52.5f, 100.0f, 1.4f, 26.0f);
    mansionMelee.config.rewardPouchDrops = {{RewardPouchType::Small, 40}, {RewardPouchType::Medium, 30}, {RewardPouchType::Big, 10}};
    mansionMelee.range = 160.0f;
    mansionMelee.weapon = &GetEspadaRunicaWeaponBlueprint();
        mansionMelee.sprite = MakeSpriteInfo("./assets/img/enemies/mansao_melee");
    templates_[BiomeType::Mansion].push_back(mansionMelee);
}

// Cria inimigos aleatórios baseados no bioma e área da sala.
void EnemySpawner::SpawnEnemiesForRoom(Room& room,
                                       std::vector<std::unique_ptr<Enemy>>& storage,
                                       std::mt19937& rng,
                                       int normalRoomsCleared) const {
    PropRules::Initialize(room);
    if (!storage.empty()) {
        return;
    }

    // Combat encounters only appear in standard rooms.
    if (room.GetType() != RoomType::Normal) {
        return;
    }

    auto it = templates_.find(room.GetBiome());
    if (it == templates_.end() || it->second.empty()) {
        return;
    }

    const RoomLayout& layout = room.Layout();
    if (layout.widthTiles <= 0 || layout.heightTiles <= 0) {
        return;
    }

    int spawnCount = EnemyRules::EnemyCountForArea(layout.widthTiles * layout.heightTiles);

    const std::vector<EnemyTemplate>& defs = it->second;
    // Cada vaga da sala vira um índice em `defs`. A Dungeon usa a composição com teto de
    // Executioner; caverna e mansão (congeladas) sorteiam direto pelos pesos.
    std::vector<int> picks;
    picks.reserve(static_cast<std::size_t>(spawnCount));
    if (room.GetBiome() == BiomeType::Dungeon) {
        for (EnemyRules::EnemyKind kind : EnemyRules::ComposeRoom(spawnCount, rng)) {
            picks.push_back(static_cast<int>(kind));
        }
    } else {
        std::vector<double> weights;
        weights.reserve(defs.size());
        for (const auto& def : defs) {
            weights.push_back(std::max(def.config.spawnRate, 0.01f));
        }
        std::discrete_distribution<int> pick(weights.begin(), weights.end());
        for (int i = 0; i < spawnCount; ++i) {
            picks.push_back(pick(rng));
        }
    }

    // Escalonamento temporário da demo (ver enemy_rules.h); só afeta a Dungeon.
    EnemyRules::DifficultyScale scale{};
    if (room.GetBiome() == BiomeType::Dungeon) {
        scale = EnemyRules::ScaleForClearedRooms(normalRoomsCleared);
    }

    Rectangle roomRect = TileRectToPixels(layout.tileBounds);
    float margin = TILE_SIZE * 0.75f;
    std::uniform_real_distribution<float> randX(roomRect.x + margin, roomRect.x + roomRect.width - margin);
    std::uniform_real_distribution<float> randY(roomRect.y + margin, roomRect.y + roomRect.height - margin);

    for (int index : picks) {
        if (index < 0 || index >= static_cast<int>(defs.size())) {
            continue;
        }

        Vector2 spawnPosition{randX(rng), randY(rng)};
        const EnemyTemplate& selected = defs[index];
        std::unique_ptr<EnemyCommon> enemy;
        if (selected.definition != nullptr) {
            EnemyConfig config = selected.config;
            config.maxHealth *= scale.health;
            enemy = std::make_unique<EnemyCommon>(config, *selected.definition, selected.sprite, scale.damage);
        } else {
            enemy = std::make_unique<EnemyCommon>(selected.config, selected.range, selected.weapon, selected.sprite);
        }
        bool available = false;
        for (int attempt = 0; attempt < 64; ++attempt) {
            if (PropRules::IsFree(room, PropRules::EntityBounds(spawnPosition, enemy->GetHalfWidth(), enemy->GetHalfHeight()))) {
                available = true; break;
            }
            spawnPosition = {randX(rng), randY(rng)};
        }
        if (!available) continue;
        enemy->Initialize(room, spawnPosition);
        storage.push_back(std::move(enemy));
    }
}
