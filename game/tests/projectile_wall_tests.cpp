#include "combat_geometry.h"
#include "projectile.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

bool Near(float a, float b) {
    return std::fabs(a - b) < 0.001f;
}

// Sala de 0..200 em x e -100..100 em y.
const Rectangle kRoom{0.0f, -100.0f, 200.0f, 200.0f};

void CheckExitFraction() {
    const std::vector<Rectangle> areas{kRoom};

    auto right = CombatGeometry::ExitFraction({100, 0}, {300, 0}, areas);
    assert(right && Near(*right, 0.5f));
    auto left = CombatGeometry::ExitFraction({100, 0}, {-100, 0}, areas);
    assert(left && Near(*left, 0.5f));
    auto up = CombatGeometry::ExitFraction({100, 0}, {100, -300}, areas);
    assert(up && Near(*up, 100.0f / 300.0f));
    auto down = CombatGeometry::ExitFraction({100, 0}, {100, 300}, areas);
    assert(down && Near(*down, 100.0f / 300.0f));

    // Permanecer dentro não sai.
    assert(!CombatGeometry::ExitFraction({10, 0}, {190, 0}, areas));

    // Atravessar para um retângulo contíguo (corredor) não sai; sair do segundo sim.
    const std::vector<Rectangle> joined{kRoom, Rectangle{200.0f, -20.0f, 100.0f, 40.0f}};
    assert(!CombatGeometry::ExitFraction({100, 0}, {250, 0}, joined));
    auto beyond = CombatGeometry::ExitFraction({100, 0}, {400, 0}, joined);
    assert(beyond && Near(*beyond, 200.0f / 300.0f));
    // Um corredor fora da linha do tiro não ajuda.
    auto offLine = CombatGeometry::ExitFraction({100, 80}, {400, 80}, joined);
    assert(offLine && Near(*offLine, 100.0f / 300.0f));

    // Começar fora ou sem áreas não corta.
    assert(!CombatGeometry::ExitFraction({-50, 0}, {300, 0}, areas));
    assert(!CombatGeometry::ExitFraction({100, 0}, {300, 0}, {}));
}

ProjectileBlueprint Ammo() {
    ProjectileBlueprint blueprint;
    blueprint.kind = ProjectileKind::Ranged;
    ThrownProjectileBlueprint shot;
    shot.common.damage = 12.0f;
    shot.common.lifespanSeconds = 5.0f;
    shot.ammunition.speed = 500.0f;
    shot.ammunition.maxDistance = 1000.0f;
    shot.ammunition.radius = 2.0f;
    blueprint.thrownProjectiles.push_back(shot);
    return blueprint;
}

ProjectileBlueprint Laser() {
    ProjectileBlueprint blueprint;
    blueprint.kind = ProjectileKind::Ranged;
    ThrownProjectileBlueprint beam;
    beam.kind = ThrownProjectileKind::Laser;
    beam.common.damage = 7.0f;
    beam.laser.length = 400.0f;
    beam.laser.thickness = 10.0f;
    beam.laser.duration = 1.0f;
    blueprint.thrownProjectiles.push_back(beam);
    return blueprint;
}

size_t HitsAt(ProjectileSystem& system, Vector2 center) {
    return system.CollectDamageEvents(center, 8.0f, 1).size();
}

void CheckAmmunition() {
    // Parede em x=200: alvo atrás dela (x=300) não é atingido; alvo antes (x=150) é.
    ProjectileSystem behind;
    behind.SetPassableAreas({kRoom});
    behind.SpawnProjectile(Ammo(), {{100, 0}, nullptr, {1, 0}});
    behind.Update(0.4f);  // vai a x=300, mas para em x=200
    assert(HitsAt(behind, {300, 0}) == 0);

    ProjectileSystem before;
    before.SetPassableAreas({kRoom});
    before.SpawnProjectile(Ammo(), {{100, 0}, nullptr, {1, 0}});
    before.Update(0.4f);
    assert(HitsAt(before, {150, 0}) == 1);

    // No frame seguinte o projétil já expirou.
    ProjectileSystem expired;
    expired.SetPassableAreas({kRoom});
    expired.SpawnProjectile(Ammo(), {{100, 0}, nullptr, {1, 0}});
    expired.Update(0.4f);
    expired.Update(0.01f);
    assert(HitsAt(expired, {200, 0}) == 0);

    // Sem áreas: comportamento antigo, atravessa.
    ProjectileSystem unlimited;
    unlimited.SpawnProjectile(Ammo(), {{100, 0}, nullptr, {1, 0}});
    unlimited.Update(0.4f);
    assert(HitsAt(unlimited, {300, 0}) == 1);

    // Disparado de fora da área (encostado na parede) não é cortado.
    ProjectileSystem outside;
    outside.SetPassableAreas({kRoom});
    outside.SpawnProjectile(Ammo(), {{250, 0}, nullptr, {1, 0}});
    outside.Update(0.2f);
    assert(HitsAt(outside, {320, 0}) == 1);
}

void CheckLaser() {
    ProjectileSystem behind;
    behind.SetPassableAreas({kRoom});
    behind.SpawnProjectile(Laser(), {{100, 0}, nullptr, {1, 0}});
    behind.Update(0.1f);  // feixe de x=100 a 500, cortado em x=200
    assert(HitsAt(behind, {350, 0}) == 0);
    assert(HitsAt(behind, {180, 0}) == 1);

    ProjectileSystem unlimited;
    unlimited.SpawnProjectile(Laser(), {{100, 0}, nullptr, {1, 0}});
    unlimited.Update(0.1f);
    assert(HitsAt(unlimited, {350, 0}) == 1);
}

} // namespace

int main() {
    CheckExitFraction();
    CheckAmmunition();
    CheckLaser();
    std::cout << "Projectile wall tests passed\n";
    return 0;
}
