#include "wall_tiles.h"

#include "floor_rules.h"
#include "room_types.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <random>
#include <tuple>
#include <unordered_set>

namespace {

constexpr const char* kBiomeDirectories[] = {"lobby", "cave", "mansion", "dungeon"};
constexpr const char* kSectionDirectories[] = {"side", "upper", "bottom", "edge"};
constexpr std::size_t kCatalogsPerBiome = 4;
constexpr float kOccludedBottomWallVisibility = 0.6f;
constexpr float kBottomWallTweenDuration = 0.2f;
// Altura do rodapé da parede inferior que se dissolve no vazio.
constexpr float kFooterFadeHeight = TILE_SIZE * 0.5f;
// Bordas calculadas de tiles inteiros; a folga só absorve arredondamento.
constexpr float kEdgeTolerance = 0.5f;

std::size_t BiomeIndex(BiomeType biome) {
    switch (biome) {
        case BiomeType::Lobby: return 0;
        case BiomeType::Cave: return 1;
        case BiomeType::Mansion: return 2;
        case BiomeType::Dungeon: return 3;
        default: return 0;
    }
}

std::size_t SectionIndex(WallRules::Section section) {
    return static_cast<std::size_t>(section);
}

std::size_t CatalogIndex(BiomeType biome, WallRules::Section section,
                         WallRules::WallType wallType) {
    const std::size_t sectionIndex = section == WallRules::Section::Side &&
            wallType == WallRules::WallType::Edge
        ? 3
        : SectionIndex(section);
    return BiomeIndex(biome) * kCatalogsPerBiome + sectionIndex;
}

Color WallBaseColorForBiome(BiomeType biome) {
    switch (biome) {
        case BiomeType::Cave: return Color{108, 108, 116, 255};
        case BiomeType::Mansion: return Color{128, 88, 56, 255};
        case BiomeType::Dungeon: return Color{76, 126, 86, 255};
        case BiomeType::Lobby: return Color{90, 92, 110, 255};
        default: return Color{90, 92, 110, 255};
    }
}

// Paredes inferiores e acabamentos laterais encaixados nelas ficam na frente dos atores.
bool DrawsInForeground(const WallRules::Segment& segment) {
    return segment.section == WallRules::Section::Bottom ||
           segment.sidePlacement != WallRules::SidePlacement::Edge;
}

unsigned char ClampToByte(int value) {
    return static_cast<unsigned char>(std::clamp(value, 0, 255));
}

Color OffsetRgb(Color color, int delta) {
    color.r = ClampToByte(static_cast<int>(color.r) + delta);
    color.g = ClampToByte(static_cast<int>(color.g) + delta);
    color.b = ClampToByte(static_cast<int>(color.b) + delta);
    return color;
}

Color RandomWallColorForTile(int tileX, int tileY, Color baseColor) {
    const std::uint64_t seedX = static_cast<std::uint64_t>(static_cast<std::int64_t>(tileX));
    const std::uint64_t seedY = static_cast<std::uint64_t>(static_cast<std::int64_t>(tileY));
    std::uint64_t seed = seedX * 0x9e3779b97f4a7c15ULL ^ seedY;
    seed ^= (seed >> 23);
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> delta(-12, 12);
    baseColor.r = ClampToByte(static_cast<int>(baseColor.r) + delta(rng));
    baseColor.g = ClampToByte(static_cast<int>(baseColor.g) + delta(rng));
    baseColor.b = ClampToByte(static_cast<int>(baseColor.b) + delta(rng));
    return baseColor;
}

void DrawNorthWallColumn(int tileX, int topTileY, Color baseColor,
                         int widthTiles, int heightTiles) {
    const float x = static_cast<float>(tileX * TILE_SIZE);
    const float bottom = static_cast<float>(topTileY * TILE_SIZE);
    const float width = static_cast<float>(widthTiles * TILE_SIZE);
    const float height = static_cast<float>(heightTiles * TILE_SIZE);
    const Rectangle wallRect{x, bottom - height, width, height};
    DrawRectangleRec(wallRect, baseColor);
    DrawRectangleRec(Rectangle{x, bottom - height, width, height * 0.2f},
                     OffsetRgb(baseColor, 25));
}

void DrawSouthWallColumn(int tileX, int floorTileY, Color baseColor,
                         int widthTiles, int heightTiles) {
    const float x = static_cast<float>(tileX * TILE_SIZE);
    const float y = static_cast<float>(floorTileY * TILE_SIZE);
    const float width = static_cast<float>(widthTiles * TILE_SIZE);
    const float height = static_cast<float>(heightTiles * TILE_SIZE);
    DrawRectangleRec(Rectangle{x, y, width, height}, baseColor);
    DrawRectangleRec(Rectangle{x, y, width, height * 0.18f}, OffsetRgb(baseColor, 24));
    DrawRectangleRec(Rectangle{x, y + height * 0.18f, width, height * 0.32f}, OffsetRgb(baseColor, 8));
    DrawRectangleRec(Rectangle{x, y + height * 0.76f, width, height * 0.24f}, OffsetRgb(baseColor, -34));
}

Rectangle SideWallRect(const WallRules::Segment& segment, int heightTiles) {
    return WallRules::ModuleBounds({segment, 1, heightTiles});
}

void DrawSideWallTile(const WallRules::Segment& segment, Color baseColor, int heightTiles) {
    const Rectangle wallRect = SideWallRect(segment, heightTiles);
    const float highlightWidth = wallRect.width * 0.16f;
    DrawRectangleRec(wallRect, baseColor);
    DrawRectangleRec(Rectangle{wallRect.x, wallRect.y, highlightWidth, wallRect.height},
                     OffsetRgb(baseColor, 18));
    DrawRectangleRec(Rectangle{wallRect.x + wallRect.width - highlightWidth, wallRect.y,
                               highlightWidth, wallRect.height}, OffsetRgb(baseColor, -28));
}

Texture2D LoadWallTexture(const std::filesystem::path& path) {
    Image image = LoadImage(path.generic_string().c_str());
    if (image.data == nullptr || image.width <= 0 || image.height <= 0) {
        std::cerr << "[WallTiles] Invalid image " << path.generic_string() << " ("
                  << image.width << 'x' << image.height << "): expected a readable image "
                  << "with positive dimensions.\n";
        if (image.data != nullptr) UnloadImage(image);
        return Texture2D{};
    }

    Texture2D texture = LoadTextureFromImage(image);
    const int width = image.width;
    const int height = image.height;
    UnloadImage(image);
    if (texture.id == 0) {
        std::cerr << "[WallTiles] Cannot upload image " << path.generic_string() << " ("
                  << width << 'x' << height << ").\n";
        return Texture2D{};
    }

    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    return texture;
}

} // namespace

WallTiles::WallTiles() {
    namespace fs = std::filesystem;
    for (std::size_t biome = 0; biome < 4; ++biome) {
        for (std::size_t section = 0; section < kCatalogsPerBiome; ++section) {
            auto& catalog = catalogs_[biome * kCatalogsPerBiome + section];
            const fs::path directory = fs::path("assets/wall") / kBiomeDirectories[biome] /
                                       kSectionDirectories[section];
            std::vector<fs::path> paths;
            std::error_code error;
            fs::directory_iterator iterator(directory, error);
            const fs::directory_iterator end;
            while (!error && iterator != end) {
                const auto entry = *iterator;
                std::error_code fileError;
                if (entry.is_regular_file(fileError)) paths.push_back(entry.path());
                iterator.increment(error);
            }
            if (error && section != 3) {
                std::cerr << "[WallTiles] Cannot read " << directory.generic_string()
                          << ": " << error.message() << '\n';
            }
            std::sort(paths.begin(), paths.end());
            for (const auto& path : paths) {
                const auto name = path.filename().string();
                const auto metadata = FloorRules::ParseFilename(name);
                if (!metadata) {
                    if (path.extension() == ".png" || path.extension() == ".PNG") {
                        std::cerr << "[WallTiles] Invalid filename: " << path.generic_string() << '\n';
                    }
                    continue;
                }
                Texture2D texture = LoadWallTexture(path);
                if (texture.id == 0) continue;
                if (!FloorRules::AddWeight(catalog.cumulative, metadata->weight)) {
                    std::cerr << "[WallTiles] Weight sum overflow: " << path.generic_string() << '\n';
                    UnloadTexture(texture);
                    continue;
                }
                catalog.textures.push_back(texture);
            }
            if (catalog.textures.empty() && section != 3) {
                std::cerr << "[WallTiles] No valid sprites in " << directory.generic_string()
                          << "; using procedural wall color.\n";
            }
        }
    }
}

WallTiles::~WallTiles() { Unload(); }

void WallTiles::Unload() {
    for (auto& catalog : catalogs_) {
        for (auto texture : catalog.textures) UnloadTexture(texture);
        catalog.textures.clear();
        catalog.cumulative.clear();
    }
    memory_.Reset(0);
    bottomWallTweens_.clear();
    bottomWallRowsBySegment_.clear();
}

void WallTiles::BeginRun(std::uint64_t seed) {
    memory_.Reset(seed);
    bottomWallTweens_.clear();
    bottomWallRowsBySegment_.clear();
}

void WallTiles::DrawBackground(const std::vector<WallRules::RoomView>& rooms) const {
    Draw(WallRules::BuildFrame(rooms), false);
}

void WallTiles::DrawForeground(const std::vector<WallRules::RoomView>& rooms) const {
    Draw(WallRules::BuildFrame(rooms), true);
}

void WallTiles::DrawForeground(const std::vector<WallRules::RoomView>& rooms,
                               Rectangle playerBounds, float deltaSeconds) const {
    const auto frame = WallRules::BuildFrame(rooms);
    UpdateBottomWallTweens(frame, playerBounds, deltaSeconds);
    Draw(frame, true, true);
}

void WallTiles::UpdateBottomWallTweens(const WallRules::Frame& frame,
                                       Rectangle playerBounds, float deltaSeconds) const {
    std::unordered_set<WallRules::BottomWallRowId, WallRules::BottomWallRowIdHash> activeRows;
    std::unordered_set<WallRules::BottomWallRowId, WallRules::BottomWallRowIdHash> occludedRows;
    bottomWallRowsBySegment_.clear();
    const float safeDelta = std::clamp(deltaSeconds, 0.0f, 1.0f);

    for (const auto& module : WallRules::BuildModules(frame)) {
        const auto& segment = module.segment;
        if (segment.section != WallRules::Section::Bottom) continue;
        if (segment.bottomWallRow.kind == WallRules::BottomWallRowKind::None) continue;

        const WallRules::Key moduleKey{segment.position, WallRules::Section::Bottom,
                                       segment.sidePlacement};
        bottomWallRowsBySegment_.emplace(moduleKey, segment.bottomWallRow);
        activeRows.insert(segment.bottomWallRow);

        const Rectangle wallBounds = WallRules::ModuleBounds(module);
        const bool overlapsPlayer = playerBounds.x < wallBounds.x + wallBounds.width &&
            playerBounds.x + playerBounds.width > wallBounds.x &&
            playerBounds.y < wallBounds.y + wallBounds.height &&
            playerBounds.y + playerBounds.height > wallBounds.y;
        if (overlapsPlayer) occludedRows.insert(segment.bottomWallRow);
    }

    for (const auto& row : activeRows) {
        auto [stateIt, inserted] = bottomWallTweens_.try_emplace(row);
        auto& tween = stateIt->second;
        if (inserted) {
            tween.elapsed = kBottomWallTweenDuration;
        }
        const float targetVisibility = occludedRows.find(row) != occludedRows.end()
            ? kOccludedBottomWallVisibility
            : 1.0f;
        if (tween.targetVisibility != targetVisibility) {
            tween.startVisibility = tween.visibility;
            tween.targetVisibility = targetVisibility;
            tween.elapsed = 0.0f;
        }

        if (tween.elapsed < kBottomWallTweenDuration) {
            tween.elapsed = std::min(tween.elapsed + safeDelta, kBottomWallTweenDuration);
            const float progress = tween.elapsed / kBottomWallTweenDuration;
            const float easedProgress = progress * progress * (3.0f - 2.0f * progress);
            tween.visibility = tween.startVisibility +
                (tween.targetVisibility - tween.startVisibility) * easedProgress;
        } else {
            tween.visibility = tween.targetVisibility;
        }
    }

    for (auto it = bottomWallTweens_.begin(); it != bottomWallTweens_.end();) {
        if (activeRows.find(it->first) == activeRows.end()) {
            it = bottomWallTweens_.erase(it);
        } else {
            ++it;
        }
    }
}

float WallTiles::ForegroundVisibility(const WallRules::Segment& segment,
                                      bool useBottomWallTweens) const {
    if (!useBottomWallTweens || segment.section != WallRules::Section::Bottom) {
        return segment.visibility;
    }
    const WallRules::Key moduleKey{segment.position, WallRules::Section::Bottom,
                                   segment.sidePlacement};
    const auto row = bottomWallRowsBySegment_.find(moduleKey);
    if (row == bottomWallRowsBySegment_.end()) return segment.visibility;
    const auto tween = bottomWallTweens_.find(row->second);
    if (tween == bottomWallTweens_.end()) return segment.visibility;
    return segment.visibility * tween->second.visibility;
}

std::vector<WallTiles::FooterFade> WallTiles::FooterFades(
    const std::vector<WallRules::Module>& modules, bool useBottomWallTweens) const {
    std::vector<Rectangle> bottomWalls;
    std::vector<Rectangle> sideWalls;
    for (const auto& module : modules) {
        const Rectangle bounds = WallRules::ModuleBounds(module);
        if (module.segment.section == WallRules::Section::Bottom) bottomWalls.push_back(bounds);
        else if (module.segment.section == WallRules::Section::Side) sideWalls.push_back(bounds);
    }
    const auto bottomOf = [](Rectangle r) { return r.y + r.height; };
    // Encostar conta como vizinhança (canto lateral junto da parede inferior).
    const auto touchesX = [](Rectangle a, Rectangle b) {
        return a.x <= b.x + b.width + kEdgeTolerance && b.x <= a.x + a.width + kEdgeTolerance;
    };
    // Faixas laterais que seguem para baixo (parede de corredor ao sul) não têm rodapé.
    std::vector<Rectangle> continuingSides;
    for (const auto& side : sideWalls) {
        for (const auto& below : sideWalls) {
            if (std::abs(below.y - bottomOf(side)) < kEdgeTolerance && touchesX(below, side)) {
                continuingSides.push_back(side);
                break;
            }
        }
    }
    const auto continues = [&](Rectangle bounds) {
        for (const auto& side : continuingSides)
            if (side.x == bounds.x && side.y == bounds.y) return true;
        return false;
    };

    std::vector<FooterFade> fades;
    for (const auto& module : modules) {
        const Rectangle bounds = WallRules::ModuleBounds(module);
        const float height = std::min(kFooterFadeHeight, bounds.height);
        const Rectangle strip{bounds.x, bottomOf(bounds) - height, bounds.width, height};
        const bool sideWall = module.segment.section == WallRules::Section::Side;
        if (sideWall) {
            // Cantos e acabamentos laterais que terminam junto de uma parede inferior somem com ela.
            bool endsWithBottomWall = false;
            for (const auto& wall : bottomWalls)
                if (std::abs(bottomOf(wall) - bottomOf(bounds)) < kEdgeTolerance && touchesX(wall, bounds))
                    endsWithBottomWall = true;
            if (!endsWithBottomWall || continues(bounds)) continue;
        } else if (module.segment.section != WallRules::Section::Bottom) {
            continue;
        }
        const float visibility = ForegroundVisibility(module.segment, useBottomWallTweens);
        if (visibility <= 0.0f) continue;
        fades.push_back({strip, visibility, sideWall});
    }
    return fades;
}

std::vector<LightingRules::LightOccluder> WallTiles::ForegroundLightOccluders(
    const std::vector<WallRules::RoomView>& rooms) const {
    std::vector<LightingRules::LightOccluder> occluders;
    const auto modules = WallRules::BuildModules(WallRules::BuildFrame(rooms));
    for (const auto& module : modules) {
        if (!DrawsInForeground(module.segment)) continue;
        const float opacity = ForegroundVisibility(module.segment, true);
        if (opacity <= 0.0f) continue;
        occluders.push_back({WallRules::ModuleBounds(module), opacity});
    }
    for (const auto& fade : FooterFades(modules, true)) {
        occluders.push_back({fade.strip, fade.visibility, true});
    }
    return occluders;
}

void WallTiles::Draw(const WallRules::Frame& frame, bool foreground,
                     bool useBottomWallTweens) const {
    const auto allModules = WallRules::BuildModules(frame);
    std::vector<WallRules::Module> modules;
    for (const auto& module : allModules) {
        if (DrawsInForeground(module.segment) == foreground) {
            modules.push_back(module);
        }
    }
    // Acabamentos laterais vêm por último: ficam sobre a ponta da parede inferior.
    constexpr int kSideFinishOrder = 2;
    const auto drawOrder = [](const WallRules::Segment& segment) {
        if (segment.section == WallRules::Section::Side) {
            return segment.sidePlacement == WallRules::SidePlacement::Edge ? 0 : kSideFinishOrder;
        }
        return segment.section == WallRules::Section::Upper ? 1 : 0;
    };
    std::sort(modules.begin(), modules.end(), [&drawOrder](const auto& a, const auto& b) {
        const auto& aSegment = a.segment;
        const auto& bSegment = b.segment;
        return std::make_tuple(drawOrder(aSegment), aSegment.position.y, aSegment.position.x,
                               aSegment.section) <
               std::make_tuple(drawOrder(bSegment), bSegment.position.y, bSegment.position.x,
                               bSegment.section);
    });

    if (!foreground) {
        for (const auto& module : modules) DrawModule(module, useBottomWallTweens);
        return;
    }

    const auto fades = FooterFades(allModules, useBottomWallTweens);
    const auto drawFades = [&fades](bool sideWalls) {
        for (const auto& fade : fades) {
            if (fade.sideWall != sideWalls) continue;
            const Rectangle& strip = fade.strip;
            DrawRectangleGradientV(static_cast<int>(strip.x), static_cast<int>(strip.y),
                                   static_cast<int>(strip.width), static_cast<int>(strip.height),
                                   ColorAlpha(VOID_COLOR, 0.0f), ColorAlpha(VOID_COLOR, fade.visibility));
        }
    };
    // O rodapé da parede inferior vem antes dos acabamentos, que o cobrem quando visíveis; onde
    // estão ocultos (corredor fechado), a ponta da parede aparece já esmaecida. O rodapé das
    // faixas laterais vem por último para também cobrir os cantos, desenhados no passe de fundo.
    const auto firstSideFinish = std::partition_point(modules.begin(), modules.end(),
        [&drawOrder](const auto& module) { return drawOrder(module.segment) < kSideFinishOrder; });
    for (auto it = modules.begin(); it != firstSideFinish; ++it) DrawModule(*it, useBottomWallTweens);
    drawFades(false);
    for (auto it = firstSideFinish; it != modules.end(); ++it) DrawModule(*it, useBottomWallTweens);
    drawFades(true);
}

void WallTiles::DrawModule(const WallRules::Module& module, bool useBottomWallTweens) const {
    auto drawModule = module;
    auto& segment = drawModule.segment;
    segment.visibility = ForegroundVisibility(segment, useBottomWallTweens);

    WallRules::WallType selectedWallType = segment.wallType;
    std::size_t catalogIndex = CatalogIndex(segment.biome, segment.section, selectedWallType);
    if (selectedWallType == WallRules::WallType::Edge &&
        catalogs_[catalogIndex].textures.empty()) {
        selectedWallType = WallRules::WallType::Standard;
        catalogIndex = CatalogIndex(segment.biome, WallRules::Section::Side, selectedWallType);
    }
    const auto& catalog = catalogs_[catalogIndex];
    const auto& choice = memory_.GetOrCreate(segment.position, segment.section,
                                              segment.biome, catalog.cumulative,
                                              segment.sidePlacement, selectedWallType);
    if (choice.sprite == WallRules::NoSprite) {
        DrawFallback(drawModule);
        return;
    }

    const Texture2D texture = catalog.textures[choice.sprite];
    const Rectangle source{0.0f, 0.0f, static_cast<float>(texture.width),
                           static_cast<float>(texture.height)};
    const Rectangle destination = WallRules::ModuleBounds(module);
    DrawTexturePro(texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f,
                   ColorAlpha(WHITE, segment.visibility));
}

void WallTiles::DrawFallback(const WallRules::Module& module) const {
    const auto& segment = module.segment;
    Color color = WallBaseColorForBiome(segment.biome);
    int variationY = segment.position.y;
    if (segment.section == WallRules::Section::Upper) variationY = segment.edgeTile.y - module.heightTiles;
    else if (segment.section == WallRules::Section::Bottom) variationY = segment.edgeTile.y + 1;
    color = RandomWallColorForTile(segment.edgeTile.x, variationY, color);
    color = ColorAlpha(color, segment.visibility);

    if (segment.section == WallRules::Section::Upper) {
        DrawNorthWallColumn(segment.edgeTile.x, segment.edgeTile.y, color,
                            module.widthTiles, module.heightTiles);
    } else if (segment.section == WallRules::Section::Bottom) {
        DrawSouthWallColumn(segment.edgeTile.x, segment.edgeTile.y - 1, color,
                            module.widthTiles, module.heightTiles);
    } else {
        DrawSideWallTile(segment, color, module.heightTiles);
    }
}
