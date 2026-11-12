#include "weapon_rules.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include "player.h"
#include "weapon.h"
#include "weapon_blueprints.h"

using namespace WeaponRules;

namespace {

// Faixas de DPS por raridade (docs/sistemas/balanceamento.md e docs/sistemas/armas.md).
// Ao reajustar um blueprint à mão, estas faixas dizem se a arma saiu do tier.
constexpr float kRarity2MinDps = 18.0f;
constexpr float kRarity2MaxDps = 23.0f;
constexpr float kBestRarity2Dps = 22.6f; // Arco Simples: o Broquel precisa ficar acima.
constexpr float kRarity3MaxDps = 30.0f;  // Broquel fica abaixo das outras de raridade 3.
constexpr float kStaffMinDps = 27.0f;
constexpr float kStaffMaxDps = 33.0f;
constexpr float kRunicMinDpsTwoHits = 40.0f;
constexpr float kRunicMinDpsAverage = 45.0f;
constexpr float kRunicMaxDpsAverage = 55.0f;

// Espada Rúnica: giro de 1,6 volta a 480 graus/s, 1 acerto a cada 0,4 s por alvo.
constexpr int kRunicMinHitsPerSpin = 2;
constexpr int kRunicMaxHitsPerSpin = 3;

bool Near(float a, float b, float epsilon = 1e-4f) { return std::fabs(a - b) <= epsilon; }

float PerShotDamageWithPower(const WeaponBlueprint& blueprint, int poder) {
    PlayerCharacter player = CreateKnightCharacter();
    player.totalAttributes.primary.poder = poder;
    WeaponState state{};
    state.blueprint = &blueprint;
    state.RecalculateDerivedStats(player);
    return state.derived.damagePerShot;
}

void TestPowerMultiplier() {
    assert(Near(PowerDamageMultiplier(0), 1.0f));
    assert(Near(PowerDamageMultiplier(10), 1.1f));
    assert(Near(PowerDamageMultiplier(100), 2.0f));
    assert(Near(PowerDamageMultiplier(-50), 0.5f));
    assert(Near(PowerDamageMultiplier(-100), 0.0f));
    assert(Near(PowerDamageMultiplier(-250), 0.0f)); // Nunca inverte o dano.
}

void TestPowerAppliedOnceToDerivedDamage() {
    const WeaponBlueprint& sword = GetEspadaCurtaWeaponBlueprint();
    const float base = PerShotDamageWithPower(sword, 0);
    assert(base > 0.0f);
    assert(Near(PerShotDamageWithPower(sword, 10), base * 1.1f));
    assert(Near(PerShotDamageWithPower(sword, 100), base * 2.0f));
    assert(Near(PerShotDamageWithPower(sword, -10), base * 0.9f));
    assert(PerShotDamageWithPower(sword, -500) <= base * 0.01f + 1e-4f); // Não vira dano negativo.
    assert(PerShotDamageWithPower(sword, -500) >= 0.0f);
}

void TestLaserTicks() {
    assert(LaserTicksPerPulse(0.22f, 0.08f) == 3);
    assert(LaserTicksPerPulse(0.24f, 0.08f) == 3); // Tick em 0,24 cai fora da duração.
    assert(LaserTicksPerPulse(0.25f, 0.08f) == 4);
    assert(LaserTicksPerPulse(0.0f, 0.08f) == 0);
    assert(LaserTicksPerPulse(0.5f, 0.0f) == 1);
}

void TestRarity2Dps() {
    for (const WeaponBlueprint* weapon : {&GetEspadaCurtaWeaponBlueprint(), &GetMachadinhaWeaponBlueprint(),
                                          &GetArcoSimplesWeaponBlueprint()}) {
        const float dps = EstimateDps(*weapon, 1);
        assert(dps >= kRarity2MinDps && dps <= kRarity2MaxDps);
    }
}

void TestStaffDps() {
    const WeaponBlueprint& staff = GetCajadoDeCarvalhoWeaponBlueprint();
    assert(!staff.projectile.thrownProjectiles.empty());
    const auto& beam = staff.projectile.thrownProjectiles.front();
    const int ticks = LaserTicksPerPulse(beam.laser.duration, beam.common.perTargetHitCooldownSeconds);
    assert(ticks == 3);
    const float dps = EstimateDps(staff, ticks);
    assert(dps >= kStaffMinDps && dps <= kStaffMaxDps);
}

void TestBroquelDps() {
    const WeaponBlueprint& shield = GetBroquelWeaponBlueprint();
    // O golpe só pode acertar o mesmo alvo uma vez por batida; senão a estimativa de 1 acerto subestima.
    assert(shield.projectile.common.perTargetHitCooldownSeconds >= shield.projectile.common.lifespanSeconds);
    const float dps = EstimateDps(shield, 1);
    assert(dps > kBestRarity2Dps && dps < kRarity3MaxDps);
    assert(shield.passiveBonuses.primary.defesa == 5);
}

void TestRunicDps() {
    const WeaponBlueprint& runic = GetEspadaRunicaWeaponBlueprint();
    const float twoHits = EstimateDps(runic, kRunicMinHitsPerSpin);
    const float threeHits = EstimateDps(runic, kRunicMaxHitsPerSpin);
    const float average = (twoHits + threeHits) * 0.5f;
    assert(twoHits >= kRunicMinDpsTwoHits);
    assert(average >= kRunicMinDpsAverage && average <= kRunicMaxDpsAverage);

    // Consistência com o blueprint: 1 acerto a cada cooldown por alvo dentro do giro.
    const auto& spin = runic.projectile.fullCircle;
    const float spinSeconds = spin.revolutions * 360.0f / spin.angularSpeedDegreesPerSecond;
    const int maxHits = LaserTicksPerPulse(spinSeconds, runic.projectile.common.perTargetHitCooldownSeconds);
    assert(maxHits == kRunicMaxHitsPerSpin);
}

} // namespace

int main() {
    TestPowerMultiplier();
    TestPowerAppliedOnceToDerivedDamage();
    TestLaserTicks();
    TestRarity2Dps();
    TestStaffDps();
    TestBroquelDps();
    TestRunicDps();
    std::cout << "Weapon rules tests passed" << std::endl;
    return 0;
}
