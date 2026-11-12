#pragma once
#include "raylib.h"
#include <cstdint>
#include <optional>
#include <vector>

struct ProjectileTarget {
    std::uintptr_t id{0};
    Vector2 center{};
    float radius{0.0f};
    std::optional<Rectangle> rectangle;
    float immunitySeconds{0.0f};
};

namespace CombatGeometry {
// Entry time [0,1], or infinity when the swept circle misses the target.
float Sweep(Vector2 from, Vector2 to, float radius, const ProjectileTarget& target);
bool Segment(const ProjectileTarget& target, Vector2 from, Vector2 to, float radius);
// Fracao [0,1] em que o segmento sai da uniao dos retangulos, partindo de dentro dela.
// Vazio se o segmento permanece dentro, se `from` ja esta fora ou se `areas` e vazio.
std::optional<float> ExitFraction(Vector2 from, Vector2 to, const std::vector<Rectangle>& areas);
bool Box(const ProjectileTarget& target, Vector2 center, Vector2 forward, float halfLength, float halfThickness);
}
