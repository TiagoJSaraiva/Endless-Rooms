#pragma once

#include "lighting_rules.h"
#include "wall_rules.h"

#include <array>
#include <unordered_map>
#include <vector>

// Owns the wall texture catalogs and stable per-run choices.
class WallTiles {
public:
    WallTiles();
    ~WallTiles();
    WallTiles(const WallTiles&) = delete;
    WallTiles& operator=(const WallTiles&) = delete;

    void BeginRun(std::uint64_t seed);
    void DrawBackground(const std::vector<WallRules::RoomView>& rooms) const;
    void DrawForeground(const std::vector<WallRules::RoomView>& rooms) const;
    void DrawForeground(const std::vector<WallRules::RoomView>& rooms,
                        Rectangle playerBounds, float deltaSeconds) const;
    // Paredes do passe frontal (inferiores e seus acabamentos laterais) com a opacidade em que
    // foram desenhadas. Chamar depois de DrawForeground com o jogador, que atualiza a translucidez.
    std::vector<LightingRules::LightOccluder> ForegroundLightOccluders(
        const std::vector<WallRules::RoomView>& rooms) const;
    void Unload();

private:
    struct Catalog {
        std::vector<Texture2D> textures;
        std::vector<std::uint64_t> cumulative;
    };

    struct BottomWallTween {
        float visibility{1.0f};
        float startVisibility{1.0f};
        float targetVisibility{1.0f};
        float elapsed{0.0f};
    };

    void Draw(const WallRules::Frame& frame, bool foreground,
              bool useBottomWallTweens = false) const;
    void UpdateBottomWallTweens(const WallRules::Frame& frame,
                                Rectangle playerBounds, float deltaSeconds) const;
    void DrawFallback(const WallRules::Module& module) const;
    // Visibilidade da revelação combinada com a translucidez da fileira inferior, se houver.
    float ForegroundVisibility(const WallRules::Segment& segment, bool useBottomWallTweens) const;

    std::array<Catalog, 16> catalogs_{};
    mutable WallRules::TileMemory memory_;
    mutable std::unordered_map<WallRules::BottomWallRowId, BottomWallTween,
                               WallRules::BottomWallRowIdHash>
        bottomWallTweens_;
    mutable std::unordered_map<WallRules::Key, WallRules::BottomWallRowId,
                               WallRules::KeyHash>
        bottomWallRowsBySegment_;
};
