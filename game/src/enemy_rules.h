#pragma once

#include "reward_pouch.h"
#include "room_types.h"

#include <array>
#include <random>
#include <unordered_set>
#include <vector>

// Regras puras dos inimigos da Dungeon (decisão 0007): tabela de atributos, composição da
// sala, escalonamento temporário de dificuldade e máquina de estados do ataque.
// Nada aqui desenha, lê input ou usa relógio: tudo chega por parâmetro.
namespace EnemyRules {

enum class EnemyKind { Victim, Guard, Executioner };

// Como o ataque se manifesta no mundo; cada padrão reutiliza uma família de projétil.
enum class AttackPattern {
    ThrownBone,  // osso em linha reta (munição arremessada)
    Thrust,      // estocada, como a da Machadinha (família Spear)
    Spin         // giro completo, como o da Espada Rúnica (família FullCircleSwing)
};

struct AttackDefinition {
    AttackPattern pattern{AttackPattern::ThrownBone};
    float damage{0.0f};
    // Distância ao jogador a partir da qual o inimigo para e inicia o ataque.
    float startRange{0.0f};
    // Tempo parado antes do ataque ativar (telegrafa o golpe); 0 = sem carga.
    float chargeSeconds{0.0f};
    // Início a início: conta a partir do começo da carga (ou do disparo, sem carga).
    float intervalSeconds{1.0f};
    // Duração do ataque depois de ativado (a estocada e o giro duram; o osso é instantâneo).
    float activeSeconds{0.0f};
    // true: o inimigo persegue o jogador enquanto o ataque está ativo (giro do Executioner).
    bool movesDuringAttack{false};
    // Imunidade por alvo entre acertos do mesmo ataque.
    float perTargetHitCooldownSeconds{0.0f};

    // ThrownBone
    float boneSpeed{0.0f};
    float boneMaxDistance{0.0f};
    float boneRadius{0.0f};
    float boneLength{0.0f};
    // Thrust: a ponta chega a thrustReach + thrustLength do inimigo.
    float thrustReach{0.0f};
    float thrustLength{0.0f};
    float thrustThickness{0.0f};
    // Spin: o raio do giro é spinRadius.
    float spinRadius{0.0f};
    float spinThickness{0.0f};
    float spinRevolutions{0.0f};
    float spinDegreesPerSecond{0.0f};
};

struct SpriteDefinition {
    const char* folder{""};  // contém idle_sprite, walking_spritesheet e, se houver carga, charge_sprite
    int frameWidth{0};
    int frameHeight{0};
    int walkFrameCount{4};
    float secondsPerFrame{0.16f};
    const char* weaponSprite{""};  // arma desenhada pelo projétil (lança, machado); vazio = nenhuma
    const char* bonePath{""};      // sprite do projétil (osso); vazio = nenhum
};

// Chances (em %) de cada bolsa; o restante é "nada".
struct PouchChances {
    int small{0};
    int medium{0};
    int big{0};
};

struct EnemyDefinition {
    EnemyKind kind{EnemyKind::Victim};
    int id{0};
    const char* name{""};
    float maxHealth{1.0f};
    float speed{0.0f};
    float collisionRadius{0.0f};
    float spawnWeight{1.0f};
    PouchChances pouches{};
    SpriteDefinition sprite{};
    AttackDefinition attack{};
};

// Os três inimigos da Dungeon, na ordem de EnemyKind.
const std::array<EnemyDefinition, 3>& DungeonEnemies();
const EnemyDefinition& Definition(EnemyKind kind);

RewardPouchDropTable DropTable(const PouchChances& chances);
// Moedas esperadas por inimigo, pela tabela de bolsas e pelas faixas de moedas de cada bolsa.
float ExpectedCoins(const PouchChances& chances);

// Sala: round(área em tiles × 0,02), mínimo 2.
int EnemyCountForArea(int areaTiles);
// Teto de Executioner na sala: floor(n / 4).
int ExecutionerCap(int enemyCount);
// Sorteia os tipos da sala pelos pesos de spawn. Um Executioner acima do teto vira Victim
// ou Guard (1:1). Determinístico para o mesmo estado de `rng`.
std::vector<EnemyKind> ComposeRoom(int enemyCount, std::mt19937& rng);

// ---- Escalonamento de dificuldade: SOLUÇÃO TEMPORÁRIA DA DEMO ----
// Sem boss, um jogador bom nunca morreria; por isso, depois da 5ª sala normal concluída,
// os inimigos novos ficam mais fortes. A versão final do jogo NÃO usará este escalonamento:
// remova esta seção, ClearedRooms e a chamada em main.cpp quando a progressão definitiva existir.
struct DifficultyScale {
    float health{1.0f};
    float damage{1.0f};
};
// k = max(0, salas normais concluídas - 5): vida × (1 + 0,05·k), dano × (1 + 0,03·k).
DifficultyScale ScaleForClearedRooms(int normalRoomsCleared);

// Salas normais concluídas na run (todos os inimigos mortos). Cada sala conta uma vez.
class ClearedRooms {
public:
    // Retorna true se a sala ainda não tinha sido contada.
    bool MarkCleared(const RoomCoords& coords);
    int Count() const { return static_cast<int>(rooms_.size()); }
    void Clear() { rooms_.clear(); }

private:
    std::unordered_set<RoomCoords, RoomCoordsHash> rooms_;
};

// ---- Máquina de estados do ataque ----
enum class AttackPhase { Ready, Charging, Active };

struct AttackState {
    AttackPhase phase{AttackPhase::Ready};
    float phaseTimer{0.0f};  // tempo restante da carga ou da fase ativa
    float cooldown{0.0f};    // tempo até poder iniciar outro ataque
};

struct AttackTick {
    bool startedCharge{false};  // o inimigo começou a carregar: travar a mira e mostrar o frame de carga
    bool fire{false};           // o ataque ativou neste tick: criar o projétil
};

// Avança o estado por `deltaSeconds`. `targetInRange` diz se o jogador está dentro de startRange.
AttackTick AdvanceAttack(AttackState& state, const AttackDefinition& attack, float deltaSeconds, bool targetInRange);
// Interrompe qualquer ataque em curso (jogador saiu da sala); o cooldown é preservado.
void CancelAttack(AttackState& state);
// O inimigo só se desloca quando livre ou, no giro, durante a fase ativa.
bool CanMove(const AttackState& state, const AttackDefinition& attack);
// Distância mínima ao jogador ao perseguir durante o ataque ativo (evita sobrepor o jogador).
extern const float kChaseContactDistance;

}  // namespace EnemyRules
