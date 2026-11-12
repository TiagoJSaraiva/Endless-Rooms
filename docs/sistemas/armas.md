# Armas

## Propósito
Define como o dano das armas é calculado, o DPS esperado por raridade e onde reajustar cada arma à mão.

## Componentes
- `game/src/weapon.h`: `WeaponBlueprint` (dados da arma) e `WeaponState::RecalculateDerivedStats` (dano, cadência e crítico finais por golpe).
- `game/src/weapon_blueprints.cpp`: um blueprint por arma (números de balanceamento).
- `game/src/weapon_rules.h/.cpp` (`namespace WeaponRules`): `PowerDamageMultiplier`, `LaserTicksPerPulse` e `EstimateDps`, puras e testáveis sem janela.
- `game/src/ui_inventory.cpp`: mostra a linha "Dano" lendo `derived.damagePerShot`; não recalcula Poder quando o valor derivado existe.

## Regras
- Dano por golpe: `(dano base + escala × atributo de ataque) × max(0, 1 + poder/100)`. O Poder é aplicado só em `RecalculateDerivedStats`; `main.cpp` e a UI usam o valor já multiplicado. Poder muito negativo zera o dano em vez de invertê-lo.
- DPS (alvo único, atributos de ataque e Poder zerados, só os passivos da própria arma): `dano × ataques/s × acertos por uso × (1 + chance crítica × (mult − 1))`.
  - Ataques/s = `baseAttacksPerSecond + dexterityGainPerPoint × Destreza passiva`, limitado por `attacksPerSecondCap`.
  - Chance crítica = `baseChance + chancePerLetalidade × Letalidade passiva`.
- Acertos por uso:
  - Cajado: laser de 0,22 s com cooldown por alvo de 0,08 s dá ticks em 0; 0,08; 0,16 s = 3 acertos por pulso (`LaserTicksPerPulse`).
  - Espada Rúnica: giro de 1,6 volta a 480 °/s dura 1,2 s; 1 acerto por alvo a cada 0,4 s (`perTargetHitCooldownSeconds`) dá 2 ou 3 acertos conforme a posição do alvo.
  - Broquel: o golpe vive 0,75 s e o cooldown por alvo é 0,8 s, então cada alvo é acertado uma vez por batida.
  - Demais armas: 1.
- Metas (de `balanceamento.md`): raridade 2 ~20; raridade 3 ~30; raridade 5 ~50. O Broquel (raridade 3) fica em ~26, acima do melhor de raridade 2 (22,6) por causa da Defesa +5.

## Tabela de armas (valores vigentes)

| Arma | Rar. | Dano base | Atq/s | Crítico | Acertos/uso | DPS |
|---|---:|---:|---:|---|---:|---:|
| Espada Curta | 2 | 12 | 1,4 (+0,12/Dest.) = 1,52 | 8% +0,6%/Let., ×1,3 | 1 | 18,7 |
| Machadinha | 2 | 16 | 1,1 | 10% +0,7%/Let., ×1,45 | 1 | 18,6 |
| Arco Simples | 2 | 8,5 | 2,2 (+0,14/Dest.) = 2,48 | 12% +0,8%/Let., ×1,45 | 1 | 22,6 |
| Cajado de Carvalho | 3 | 6 por tick (antes 2) | 1,6 | 10%, ×1,55 | 3 ticks | 30,4 |
| Broquel | 3 | 43 (antes 10) | 0,6 | 5%, ×1,2 | 1 | 26,1 |
| Espada Rúnica | 5 | 32,5 (antes 20) | 0,55 | 14% +1%/Let. (20% com a passiva), ×1,65 | 2 a 3 | 40,4 (2) a 60,6 (3); média 50,5 |

## Onde ajustar manualmente
Todas as linhas em `game/src/weapon_blueprints.cpp` (podem se deslocar se o arquivo mudar). "Dano" = `damage.baseDamage` e `attributeScaling`; "cadência" = `cadence.*` (a linha de `cooldownSeconds` só vale sem `baseAttacksPerSecond`); "crítico" = `critical.*`; "alcance" = tamanho da hitbox, alcance ou velocidade/distância.

| Arma | Dano | Cadência | Crítico | Alcance |
|---|---|---|---|---|
| Espada Curta | 101-102 | 98, 103-105 | 106-108 | 85-87 (arco 110 px, 110°); cooldown por alvo 83 |
| Machadinha | 155-156 | 152, 157-159 | 160-162 | 135-137 (comprimento, largura, reach); cooldown por alvo 130 |
| Arco Simples | 264-265 (`arrow.common.damage` na 241 é só reserva) | 261, 266-268 | 269-271 | 247-249 (velocidade, distância máx., raio da flecha) |
| Cajado de Carvalho | 328-329 | 324, 330-332 | 333-335 | 307-309 (comprimento, espessura, duração do laser); cooldown por alvo (ticks) 296 e 306 |
| Broquel | 54-55 | 51, 56-58 | 59-61 | 18-20 e 37-40 (hitbox); cooldown por alvo 35 (precisa ser ≥ `lifespanSeconds`, linha 25) |
| Espada Rúnica | 205-206 | 202, 207-209 | 210-212 | 188-191 (comprimento, espessura, voltas, velocidade angular); cooldown por alvo 186 |

Mudar `perTargetHitCooldownSeconds`, o `laser.duration` ou o giro da Rúnica muda os acertos por uso e, portanto, o DPS real. Os campos `common.damage` dos projéteis (linhas 24, 75, 124, 175) são reserva e são sobrescritos pelo dano derivado da arma.

## Contratos
- Quem muda dano, cadência ou crítico de um blueprint reajusta também as faixas dos testes abaixo, se a arma sair do tier.
- A UI de inventário lê `derived.damagePerShot` (já com Poder); não multiplique Poder de novo fora de `RecalculateDerivedStats`.

## Testes
`game/tests/weapon_rules_tests.cpp` (`mingw32-make test`):
- `PowerDamageMultiplier` (0 → ×1; 10 → ×1,1; negativo não inverte) e Poder aplicado uma vez ao dano derivado.
- Faixas de DPS que travam cada arma: raridade 2 entre 18 e 23 (constantes `kRarity2MinDps`/`kRarity2MaxDps`); Broquel acima de 22,6 e abaixo de 30; Cajado entre 27 e 33; Rúnica ≥ 40 com 2 acertos e entre 45 e 55 na média de 2 e 3 acertos.
- Consistência de acertos: 3 ticks do laser, Broquel com um acerto por batida, 3 acertos máximos por giro da Rúnica.

Validar jogando: sensação de dano do Broquel (golpe lento e forte) e do Cajado (ticks de 6), e o efeito de Poder no número mostrado no inventário.
