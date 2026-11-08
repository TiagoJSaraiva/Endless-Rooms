# Sistemas

Uma nota por sistema, criada quando o sistema passa a existir ou ganha regras que não cabem em `arquitetura.md`.

| Nota | Sistema |
|---|---|
| [props-e-iluminacao.md](props-e-iluminacao.md) | Props de chão, catálogo, geração e iluminação |
| [reward-pouches.md](reward-pouches.md) | Bolsas de recompensa: pools, drops e coleta |
| [colisao-de-salas.md](colisao-de-salas.md) | Limite inferior das salas e navegação junto à borda |
| [diagnostico-de-crashes.md](diagnostico-de-crashes.md) | Validade de donos de projéteis e cache de wall props |

Modelo:

```markdown
# <Sistema>

## Propósito
Uma ou duas frases.

## Componentes
Módulos (`.h`/`.cpp`) envolvidos, com o papel de cada um.

## Regras
As regras de gameplay que o código implementa (valores, limites, condições).

## Contratos
Garantias entre módulos: quem é dono de quê, tempo de vida, invariantes.

## Testes
Executáveis em `game/tests/` e o que cobrem; verificação manual quando necessária.
```
