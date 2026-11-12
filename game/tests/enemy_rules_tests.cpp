#include "enemy_attacks.h"
#include "enemy_rules.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <random>

using namespace EnemyRules;

namespace {

bool Near(float a, float b, float tolerance = 0.05f) {
    return std::fabs(a - b) <= tolerance;
}

void TestAttributes() {
    const EnemyDefinition& victim = Definition(EnemyKind::Victim);
    const EnemyDefinition& guard = Definition(EnemyKind::Guard);
    const EnemyDefinition& executioner = Definition(EnemyKind::Executioner);

    assert(victim.maxHealth == 20.0f && guard.maxHealth == 45.0f && executioner.maxHealth == 130.0f);
    assert(victim.speed == 170.0f && guard.speed == 140.0f && executioner.speed == 90.0f);
    assert(victim.collisionRadius == 22.0f && guard.collisionRadius == 24.0f && executioner.collisionRadius == 30.0f);
    assert(victim.spawnWeight == 1.0f && guard.spawnWeight == 1.0f && Near(executioner.spawnWeight, 0.35f, 1e-6f));
    assert(victim.attack.damage == 5.0f && guard.attack.damage == 10.0f && executioner.attack.damage == 16.0f);
    assert(Near(victim.attack.intervalSeconds, 1.6f, 1e-6f));
    assert(Near(guard.attack.intervalSeconds, 1.8f, 1e-6f));
    assert(Near(executioner.attack.intervalSeconds, 4.0f, 1e-6f));
    assert(victim.attack.chargeSeconds == 0.0f);
    assert(Near(guard.attack.chargeSeconds, 0.35f, 1e-6f));
    assert(Near(executioner.attack.chargeSeconds, 0.8f, 1e-6f));

    // Ids únicos e na ordem de EnemyKind.
    const auto& all = DungeonEnemies();
    for (std::size_t i = 0; i < all.size(); ++i) {
        assert(static_cast<std::size_t>(all[i].kind) == i);
        for (std::size_t j = i + 1; j < all.size(); ++j) assert(all[i].id != all[j].id);
    }
}

void TestAttackGeometry() {
    // Alcances da tabela: osso 480 de início e 600 de voo, estocada 120, giro 130.
    const AttackDefinition& bone = Definition(EnemyKind::Victim).attack;
    assert(bone.startRange == 480.0f && bone.boneSpeed == 380.0f && EnemyAttacks::MaxReach(bone) == 600.0f);
    const AttackDefinition& thrust = Definition(EnemyKind::Guard).attack;
    assert(thrust.startRange == 120.0f && EnemyAttacks::MaxReach(thrust) == 120.0f);
    const AttackDefinition& spin = Definition(EnemyKind::Executioner).attack;
    assert(spin.startRange == 130.0f && EnemyAttacks::MaxReach(spin) == 130.0f);
    // 1,6 volta a 480 graus/s dura 1,2 s; imunidade de 0,6 s dá no máximo 2 acertos por giro.
    assert(Near(spin.spinRevolutions * 360.0f / spin.spinDegreesPerSecond, 1.2f, 1e-4f));
    assert(Near(spin.activeSeconds, 1.2f, 1e-4f));
    assert(Near(spin.perTargetHitCooldownSeconds, 0.6f, 1e-6f));
    assert(static_cast<int>(spin.activeSeconds / spin.perTargetHitCooldownSeconds - 1e-3f) + 1 <= 2);
    assert(Near(thrust.perTargetHitCooldownSeconds, 0.6f, 1e-6f));

    // Blueprints: família, dano escalonado e crítico zero.
    ProjectileBlueprint boneBlueprint = EnemyAttacks::BuildProjectile(Definition(EnemyKind::Victim), 1.0f);
    assert(boneBlueprint.kind == ProjectileKind::Ranged && boneBlueprint.thrownProjectiles.size() == 1);
    assert(boneBlueprint.thrownProjectiles[0].ammunition.speed == 380.0f);
    assert(boneBlueprint.thrownProjectiles[0].ammunition.maxDistance == 600.0f);
    assert(boneBlueprint.thrownProjectiles[0].common.damage == 5.0f);
    assert(boneBlueprint.thrownProjectiles[0].common.criticalChance == 0.0f);
    ProjectileBlueprint thrustBlueprint = EnemyAttacks::BuildProjectile(Definition(EnemyKind::Guard), 2.0f);
    assert(thrustBlueprint.kind == ProjectileKind::Spear && thrustBlueprint.common.damage == 20.0f);
    assert(thrustBlueprint.spear.reach + thrustBlueprint.spear.length == 120.0f);
    assert(thrustBlueprint.common.criticalChance == 0.0f);
    ProjectileBlueprint spinBlueprint = EnemyAttacks::BuildProjectile(Definition(EnemyKind::Executioner), 1.0f);
    assert(spinBlueprint.kind == ProjectileKind::FullCircleSwing && spinBlueprint.common.damage == 16.0f);
    assert(spinBlueprint.fullCircle.length == 130.0f && spinBlueprint.fullCircle.followOwner);
    assert(spinBlueprint.common.criticalChance == 0.0f);
}

void TestPouches() {
    // Moedas esperadas por inimigo: 7,1 / 11,6 / 36.
    assert(Near(ExpectedCoins(Definition(EnemyKind::Victim).pouches), 7.1f));
    assert(Near(ExpectedCoins(Definition(EnemyKind::Guard).pouches), 11.6f));
    assert(Near(ExpectedCoins(Definition(EnemyKind::Executioner).pouches), 36.0f));

    // A tabela de drop é válida (soma <= 100) e omite chances zeradas.
    for (const EnemyDefinition& definition : DungeonEnemies()) {
        int total = 0;
        for (const RewardPouchDrop& drop : DropTable(definition.pouches)) {
            assert(drop.chancePercent > 0);
            total += drop.chancePercent;
        }
        assert(total <= 100);
    }
    assert(DropTable(Definition(EnemyKind::Victim).pouches).size() == 2);
    assert(DropTable(Definition(EnemyKind::Executioner).pouches).size() == 2);
}

void TestEnemyCountForArea() {
    for (int width = 10; width <= 20; width += 2) {
        for (int height = 10; height <= 20; height += 2) {
            const int area = width * height;
            const int count = EnemyCountForArea(area);
            assert(count >= 2);
            assert(count == static_cast<int>(std::lround(area * 0.02)));
            assert(count >= 2 && count <= 8);
        }
    }
    assert(EnemyCountForArea(100) == 2);
    assert(EnemyCountForArea(400) == 8);
    assert(EnemyCountForArea(0) == 2);
}

void TestExecutionerCap() {
    assert(ExecutionerCap(2) == 0 && ExecutionerCap(3) == 0);
    assert(ExecutionerCap(4) == 1 && ExecutionerCap(7) == 1);
    assert(ExecutionerCap(8) == 2);

    // Em muitas seeds: tamanho certo, teto respeitado e salas com menos de 4 sem Executioner.
    int executionersSeen = 0;
    int victims = 0;
    int guards = 0;
    for (unsigned seed = 1; seed <= 2000; ++seed) {
        std::mt19937 rng(seed);
        for (int count = 2; count <= 8; ++count) {
            std::vector<EnemyKind> kinds = ComposeRoom(count, rng);
            assert(static_cast<int>(kinds.size()) == count);
            int executioners = 0;
            for (EnemyKind kind : kinds) {
                if (kind == EnemyKind::Executioner) ++executioners;
                if (kind == EnemyKind::Victim) ++victims;
                if (kind == EnemyKind::Guard) ++guards;
            }
            assert(executioners <= ExecutionerCap(count));
            executionersSeen += executioners;
        }
    }
    assert(executionersSeen > 0);
    // Victim e Guard têm o mesmo peso e o excedente de Executioner é dividido 1:1.
    assert(std::fabs(static_cast<double>(victims - guards)) < 0.05 * static_cast<double>(victims + guards));

    // Mesmo estado de RNG, mesma sala.
    std::mt19937 a(77);
    std::mt19937 b(77);
    assert(ComposeRoom(8, a) == ComposeRoom(8, b));
}

void TestScaling() {
    for (int cleared = 0; cleared <= 5; ++cleared) {
        DifficultyScale scale = ScaleForClearedRooms(cleared);
        assert(scale.health == 1.0f && scale.damage == 1.0f);
    }
    DifficultyScale sixth = ScaleForClearedRooms(6);
    assert(Near(sixth.health, 1.05f, 1e-5f) && Near(sixth.damage, 1.03f, 1e-5f));
    DifficultyScale tenth = ScaleForClearedRooms(10);
    assert(Near(tenth.health, 1.25f, 1e-5f) && Near(tenth.damage, 1.15f, 1e-5f));
    assert(ScaleForClearedRooms(-3).health == 1.0f);

    // Salas concluídas contam uma vez cada.
    ClearedRooms cleared;
    assert(cleared.Count() == 0);
    assert(cleared.MarkCleared({1, 2}));
    assert(!cleared.MarkCleared({1, 2}));
    assert(cleared.MarkCleared({2, 1}));
    assert(cleared.Count() == 2);
    cleared.Clear();
    assert(cleared.Count() == 0);
}

// Simula a máquina de estados com passo fixo; devolve os instantes de carga e disparo.
struct Timeline {
    std::vector<float> chargeStarts;
    std::vector<float> fires;
};

Timeline Simulate(const AttackDefinition& attack, float totalSeconds) {
    constexpr float kStep = 0.005f;
    AttackState state;
    Timeline timeline;
    for (float time = 0.0f; time < totalSeconds; time += kStep) {
        AttackTick tick = AdvanceAttack(state, attack, kStep, true);
        if (tick.startedCharge) timeline.chargeStarts.push_back(time);
        if (tick.fire) timeline.fires.push_back(time);
    }
    return timeline;
}

void TestAttackTimings() {
    // Victim: sem carga, um osso a cada 1,6 s.
    {
        const AttackDefinition& attack = Definition(EnemyKind::Victim).attack;
        Timeline timeline = Simulate(attack, 5.0f);
        assert(timeline.chargeStarts.empty());
        assert(timeline.fires.size() == 4);
        assert(Near(timeline.fires[1] - timeline.fires[0], 1.6f, 0.02f));
    }
    // Guard: 0,35 s de carga, intervalo 1,8 s de início a início.
    {
        const AttackDefinition& attack = Definition(EnemyKind::Guard).attack;
        Timeline timeline = Simulate(attack, 6.0f);
        assert(timeline.chargeStarts.size() >= 3 && timeline.fires.size() >= 3);
        assert(Near(timeline.fires[0] - timeline.chargeStarts[0], 0.35f, 0.02f));
        assert(Near(timeline.chargeStarts[1] - timeline.chargeStarts[0], 1.8f, 0.02f));
    }
    // Executioner: 0,8 s de carga, intervalo 4,0 s.
    {
        const AttackDefinition& attack = Definition(EnemyKind::Executioner).attack;
        Timeline timeline = Simulate(attack, 9.0f);
        assert(timeline.chargeStarts.size() == 3 && timeline.fires.size() == 3);
        assert(Near(timeline.fires[0] - timeline.chargeStarts[0], 0.8f, 0.02f));
        assert(Near(timeline.chargeStarts[1] - timeline.chargeStarts[0], 4.0f, 0.02f));
    }
}

void TestMovementAndCancel() {
    const AttackDefinition& spin = Definition(EnemyKind::Executioner).attack;
    AttackState state;
    assert(CanMove(state, spin));
    AdvanceAttack(state, spin, 0.01f, true);
    assert(state.phase == AttackPhase::Charging && !CanMove(state, spin));
    // A carga é inevitável: o jogador sair do alcance não a cancela.
    AdvanceAttack(state, spin, 0.4f, false);
    assert(state.phase == AttackPhase::Charging);
    AttackTick tick = AdvanceAttack(state, spin, 0.45f, false);
    assert(tick.fire && state.phase == AttackPhase::Active);
    // Giro: anda durante a fase ativa e volta a Ready depois dela.
    assert(CanMove(state, spin));
    AdvanceAttack(state, spin, spin.activeSeconds + 0.01f, false);
    assert(state.phase == AttackPhase::Ready);

    // Estocada: parado durante a fase ativa.
    const AttackDefinition& thrust = Definition(EnemyKind::Guard).attack;
    AttackState guardState;
    AdvanceAttack(guardState, thrust, 0.01f, true);
    AdvanceAttack(guardState, thrust, thrust.chargeSeconds, true);
    assert(guardState.phase == AttackPhase::Active && !CanMove(guardState, thrust));

    // Fora do alcance e sem cooldown, nada acontece; cancelar preserva o cooldown.
    AttackState idle;
    assert(!AdvanceAttack(idle, thrust, 1.0f, false).startedCharge);
    AttackState cancelled;
    AdvanceAttack(cancelled, thrust, 0.01f, true);
    CancelAttack(cancelled);
    assert(cancelled.phase == AttackPhase::Ready && cancelled.cooldown > 0.0f);
}

}  // namespace

int main() {
    TestAttributes();
    TestAttackGeometry();
    TestPouches();
    TestEnemyCountForArea();
    TestExecutionerCap();
    TestScaling();
    TestAttackTimings();
    TestMovementAndCancel();
    std::cout << "Enemy rules tests passed\n";
    return 0;
}
