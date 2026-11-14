#pragma once

#include "prop.h"
#include "wall_rules.h"

struct WallPropDefinition {
    std::string id;
    std::string texturePath;
    int widthTiles{2};
    std::vector<WallRules::Section> sections{WallRules::Section::Upper, WallRules::Section::Bottom};
    std::vector<BiomeType> biomes{BiomeType::Dungeon};
    float weight{1.0f};
    std::optional<PropAnimation> animation;
    std::optional<PropLight> light;
    // Só para a seção Side: tamanho do quadro na tela, em pixels. A arte olha para a direita
    // (presa à parede oeste) e é espelhada na parede leste.
    Vector2 sideSize{0.0f, 0.0f};
};

struct WallPropKey {
    WallRules::HostId host;
    WallRules::Section section{WallRules::Section::Upper};
    WallRules::Position position{};
    bool operator==(const WallPropKey& other) const noexcept {
        return host == other.host && section == other.section && position == other.position;
    }
};

struct WallPropInstance {
    WallPropKey key;
    const WallPropDefinition* definition{nullptr}; // Catalog must outlive the registry.
    WallRules::Module module{};
    float animationPhase{0.0f};
    // Upper/Bottom: módulo inteiro. Side: sideSize encostado na face interna da parede,
    // centralizado na altura do módulo.
    Rectangle Bounds() const;
    float Depth() const;
    Rectangle LightClip() const;
    // Verdadeiro na parede leste: o sprite lateral é espelhado para apontar para dentro da sala.
    bool FlipX() const;
};

struct WallPropView {
    const WallPropInstance* prop;
    float visibility;
};

const std::vector<WallPropDefinition>& GetWallPropDefinitions();

namespace WallPropRules {
struct GenerationSettings {
    float occupancyChance{0.6f};
    // Distância mínima obrigatória entre props na mesma parede.
    int gapTiles{0};
    // Multiplica a chance quando o módulo encosta numa prop já colocada: tochas vizinhas
    // ficam possíveis, mas raras (0,6 × 0,2 = 12%).
    float adjacentChanceFactor{0.2f};
};

// Pure generation, using all generated rooms at full visibility, never the reveal state.
std::vector<WallPropInstance> Generate(const std::vector<WallRules::RoomView>& rooms, std::uint64_t seed,
                                     const std::vector<WallPropDefinition>& catalog = GetWallPropDefinitions(),
                                     GenerationSettings settings = {});
std::vector<WallPropView> VisibleInstances(const std::vector<WallPropInstance>& props,
                                         const std::vector<WallRules::RoomView>& rooms);
}
