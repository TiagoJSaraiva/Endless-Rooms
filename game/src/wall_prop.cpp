#include "wall_prop.h"

#include "door_visual_rules.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <tuple>

namespace {
auto CorridorKey(TileRect bounds) {
    return std::make_tuple(bounds.x, bounds.y, bounds.width, bounds.height);
}

std::uint64_t PlacementSeed(std::uint64_t seed, const WallPropKey& key) {
    auto value = HashCombine(seed, 0x57414C4C50524F50ULL);
    auto add = [&](int part) { value = HashCombine(value, static_cast<std::uint64_t>(part)); };
    add(static_cast<int>(key.host.kind));
    add(key.host.room.x); add(key.host.room.y);
    add(key.host.bounds.x); add(key.host.bounds.y);
    add(key.host.bounds.width); add(key.host.bounds.height);
    add(static_cast<int>(key.section)); add(key.position.x); add(key.position.y);
    return value;
}

bool Supports(const WallPropDefinition& definition, const WallRules::Module& module) {
    return definition.widthTiles > 0 && definition.widthTiles <= module.widthTiles &&
           std::isfinite(definition.weight) && definition.weight > 0 &&
           std::find(definition.sections.begin(), definition.sections.end(), module.segment.section) != definition.sections.end() &&
           std::find(definition.biomes.begin(), definition.biomes.end(), module.segment.biome) != definition.biomes.end();
}

// Verdadeiro se o módulo encosta numa prop já colocada na mesma linha de parede do mesmo anfitrião.
// Paredes laterais encostam na vertical: módulos da mesma coluna, um logo acima do outro.
bool TouchesPlaced(const std::vector<WallPropInstance>& placed, const WallPropKey& key, const WallRules::Module& module) {
    for (const auto& prop : placed) {
        if (!(prop.key.host == key.host) || prop.key.section != key.section) continue;
        if (key.section == WallRules::Section::Side) {
            if (prop.key.position.x != key.position.x) continue;
            if (prop.key.position.y + prop.module.heightTiles == key.position.y ||
                key.position.y + module.heightTiles == prop.key.position.y) return true;
            continue;
        }
        if (prop.key.position.y != key.position.y) continue;
        if (prop.key.position.x + prop.definition->widthTiles == key.position.x ||
            key.position.x + module.widthTiles == prop.key.position.x) return true;
    }
    return false;
}

// Só faixas laterais comuns de dois tiles recebem props; cantos e acabamentos de corredor ficam livres.
// A faixa que desce até um piso com parede inferior também fica livre: essa parede é desenhada
// por cima da última fileira do piso e esconderia a prop.
bool RegularSidePair(const WallRules::Module& module, const WallRules::Frame& frame) {
    const auto& segment = module.segment;
    if (segment.sidePlacement != WallRules::SidePlacement::Edge || segment.position.y != segment.edgeTile.y ||
        module.heightTiles != 2) return false;
    const auto lower = frame.find({{segment.position.x, segment.position.y + 1}, WallRules::Section::Side});
    if (lower == frame.end() || lower->second.edgeTile.y != segment.edgeTile.y + 1 ||
        lower->second.edgeTile.x != segment.edgeTile.x) return false;
    return frame.find({lower->second.edgeTile, WallRules::Section::Bottom}) == frame.end();
}

bool DoorNeighbor(const WallRules::Module& module, const Room& room) {
    const auto& bounds = room.Layout().tileBounds;
    const auto& segment = module.segment;
    for (const auto& door : room.Layout().doors) {
        if (door.sealed) continue;
        if ((door.direction == Direction::North && segment.section != WallRules::Section::Upper) ||
            (door.direction == Direction::South && segment.section != WallRules::Section::Bottom) ||
            (door.direction != Direction::North && door.direction != Direction::South)) continue;
        const int doorX = bounds.x + door.offset;
        if (segment.position.x + module.widthTiles == doorX ||
            segment.position.x == doorX + door.width) return true;
    }
    return false;
}
}

Rectangle WallPropInstance::Bounds() const {
    Rectangle bounds = WallRules::ModuleBounds(module);
    if (key.section == WallRules::Section::Side) {
        const Vector2 size = definition->sideSize;
        const float x = FlipX() ? bounds.x - size.x : bounds.x + bounds.width;
        return {x, bounds.y + (bounds.height - size.y) * 0.5f, size.x, size.y};
    }
    bounds.width = static_cast<float>(definition->widthTiles * TILE_SIZE);
    return bounds;
}

bool WallPropInstance::FlipX() const {
    return key.section == WallRules::Section::Side && module.segment.position.x > module.segment.edgeTile.x;
}

float WallPropInstance::Depth() const {
    auto bounds = Bounds();
    return bounds.y + bounds.height;
}

Rectangle WallPropInstance::LightClip() const {
    const auto& b = key.host.bounds;
    return {float(b.x * TILE_SIZE), float((b.y - 2) * TILE_SIZE),
            float(b.width * TILE_SIZE), float((b.height + 3) * TILE_SIZE)};
}

const std::vector<WallPropDefinition>& GetWallPropDefinitions() {
    static const std::vector<WallPropDefinition> catalog = [] {
        WallPropDefinition torch;
        torch.id = "torch";
        torch.texturePath = "assets/img/wallprop/torch.png";
        torch.animation = PropAnimation{995, 957, 8, 0.10f, false};
        torch.light = PropLight{320.0f, 0.65f, {255, 210, 140, 255}, {0.50f, 0.36f}};

        // Tocha das paredes laterais: 8 quadros de 30x83 px, desenhada no tamanho nativo.
        WallPropDefinition sideTorch;
        sideTorch.id = "side-torch";
        sideTorch.texturePath = "assets/img/sidewallprop/torch.png";
        sideTorch.widthTiles = 1;
        sideTorch.sections = {WallRules::Section::Side};
        sideTorch.animation = PropAnimation{30, 83, 8, 0.10f, false};
        sideTorch.light = PropLight{320.0f, 0.65f, {255, 210, 140, 255}, {0.62f, 0.40f}};
        sideTorch.sideSize = Vector2{30.0f, 83.0f};
        return std::vector<WallPropDefinition>{torch, sideTorch};
    }();
    return catalog;
}

namespace WallPropRules {
std::vector<WallPropInstance> Generate(const std::vector<WallRules::RoomView>& rooms, std::uint64_t seed,
                                     const std::vector<WallPropDefinition>& catalog, GenerationSettings settings) {
    std::vector<WallRules::RoomView> complete;
    std::unordered_map<RoomCoords, const Room*, RoomCoordsHash> owners;
    for (const auto& view : rooms) if (view.room) {
        complete.push_back({view.room, 1.0f});
        owners.emplace(view.room->GetCoords(), view.room);
    }
    const auto frame = WallRules::BuildFrame(complete);
    auto modules = WallRules::BuildModules(frame);
    std::sort(modules.begin(), modules.end(), [](const auto& a, const auto& b) {
        return std::tie(a.segment.position.y, a.segment.position.x, a.segment.section) <
               std::tie(b.segment.position.y, b.segment.position.x, b.segment.section);
    });
    std::vector<WallPropInstance> result;
    for (const auto& module : modules) {
        const auto& segment = module.segment;
        const bool side = segment.section == WallRules::Section::Side;
        if (segment.host.kind == WallRules::HostKind::None || (side && !RegularSidePair(module, frame))) continue;
        // A module can straddle two compatible wall styles; props must stay on one host.
        bool sameHost = true;
        const int span = side ? module.heightTiles : module.widthTiles;
        for (int d = 0; d < span; ++d) {
            const WallRules::Position part{segment.position.x + (side ? 0 : d), segment.position.y + (side ? d : 0)};
            const auto found = frame.find({part, segment.section});
            if (found == frame.end() || !(found->second.host == segment.host)) sameHost = false;
        }
        if (!sameHost) continue;

        bool guaranteed = false;
        if (segment.host.kind == WallRules::HostKind::Room) {
            const auto found = owners.find(segment.host.room);
            const Room* owner = found == owners.end() ? nullptr : found->second;
            if (!owner || owner->GetType() == RoomType::Lobby || owner->GetType() == RoomType::Unknown) continue;
            if (owner->GetType() != RoomType::Normal) {
                if (!DoorNeighbor(module, *owner)) continue;
                guaranteed = true;
            }
        }

        WallPropKey key{segment.host, segment.section, segment.position};
        std::mt19937_64 rng(PlacementSeed(seed, key));
        float chance = std::clamp(settings.occupancyChance, 0.0f, 1.0f);
        if (TouchesPlaced(result, key, module)) chance *= std::clamp(settings.adjacentChanceFactor, 0.0f, 1.0f);
        if (!guaranteed && std::generate_canonical<double, 53>(rng) >= chance) continue;
        std::vector<const WallPropDefinition*> choices;
        std::vector<double> weights;
        for (const auto& definition : catalog) if (Supports(definition, module)) {
            choices.push_back(&definition); weights.push_back(definition.weight);
        }
        if (choices.empty()) continue;
        const auto* definition = choices[std::discrete_distribution<std::size_t>(weights.begin(), weights.end())(rng)];
        bool blocked = false;
        for (const auto& placed : result) {
            if (!(placed.key.host == key.host) || placed.key.section != key.section) continue;
            const int gap = guaranteed ? 0 : std::max(0, settings.gapTiles);
            if (side) {
                // Na lateral, o espaçamento é contado na coluna.
                if (placed.key.position.x == key.position.x &&
                    key.position.y < placed.key.position.y + placed.module.heightTiles + gap &&
                    key.position.y + module.heightTiles + gap > placed.key.position.y) blocked = true;
                continue;
            }
            if (placed.key.position.y != key.position.y) continue;
            if (key.position.x < placed.key.position.x + placed.definition->widthTiles + gap &&
                key.position.x + definition->widthTiles + gap > placed.key.position.x) blocked = true;
        }
        if (blocked) continue;
        float phase = 0.0f;
        if (definition->animation) {
            const auto& clip = *definition->animation;
            phase = static_cast<float>(std::generate_canonical<double, 53>(rng)) *
                    std::max(0.0f, clip.secondsPerFrame) * std::max(1, clip.frameCount);
        }
        result.push_back({key, definition, module, phase});
    }
    return result;
}

std::vector<WallPropView> VisibleInstances(const std::vector<WallPropInstance>& props,
                                         const std::vector<WallRules::RoomView>& rooms) {
    const auto frame = WallRules::BuildFrame(rooms);
    // Index shared doors once instead of searching the entire map for each source every frame.
    std::map<std::tuple<int, int, int, int>, float> corridorExposure;
    for (const auto& view : rooms) {
        if (!view.room || view.visibility <= 0) continue;
        for (const auto& door : view.room->Layout().doors) {
            if (door.sealed || door.corridorTiles.width <= 0 || door.corridorTiles.height <= 0) continue;
            auto& exposure = corridorExposure[CorridorKey(door.corridorTiles)];
            exposure = std::max(exposure, std::clamp(view.visibility, 0.0f, 1.0f) *
                                          DoorVisualRules::OpenProgress(door.doorState.get()));
        }
    }
    std::vector<WallPropView> result;
    for (const auto& prop : props) {
        float visibility = 1.0f;
        const bool side = prop.key.section == WallRules::Section::Side;
        const int span = side ? prop.module.heightTiles : prop.module.widthTiles;
        for (int d = 0; d < span; ++d) {
            const WallRules::Position tile{prop.key.position.x + (side ? 0 : d), prop.key.position.y + (side ? d : 0)};
            const auto part = frame.find({tile, prop.key.section});
            if (part == frame.end() || !(part->second.host == prop.key.host)) { visibility = 0; break; }
            visibility = std::min(visibility, part->second.visibility);
        }
        if (prop.key.host.kind == WallRules::HostKind::Corridor && visibility > 0) {
            const auto exposure = corridorExposure.find(CorridorKey(prop.key.host.bounds));
            visibility = exposure == corridorExposure.end() ? 0 : std::min(visibility, exposure->second);
        }
        if (visibility > 0) result.push_back({&prop, visibility});
    }
    return result;
}
}
