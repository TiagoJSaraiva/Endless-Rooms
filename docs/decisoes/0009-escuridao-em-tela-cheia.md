# 0009 — Escuridão como filtro de tela cheia

- **Data**: 2026-11
- **Status**: aceita

## Contexto
A escuridão dos biomas é um mapa de luz multiplicativo aplicado por sala (`SceneLighting::Ambient` em `prop_renderer.cpp`). Como o escurecimento é recortado pela área de cada sala, ele vaza para fora das paredes da Dungeon. Além disso, o modelo por sala não descreve bem a passagem gradual entre ambientes de luminosidade diferente.

## Decisão
A escuridão passa a ser um único filtro sobre toda a área do mundo na tela, com um alpha-alvo por ambiente (lobby claro, Dungeon escura e futuros ambientes com valores próprios, em tabela de dados).
- Ao atravessar um corredor entre ambientes de níveis diferentes, o alpha é interpolado conforme o progresso do jogador entre o início e o fim do corredor.
- Tochas e velas continuam recortando o filtro e clareando o entorno.
- HUD, inventário, lojas, prompts e console ficam fora do filtro.
- O cálculo do alpha (alvo por ambiente e interpolação pelo progresso) é regra testável sem janela, separado do desenho (decisão 0007).

## Alternativas descartadas
- **Corrigir o recorte por sala**: remove o vazamento, mas mantém a transição presa aos limites das salas.
- **Filtro uniforme sem recorte das luzes**: é mais simples, mas perde a atmosfera das tochas.

## Consequências
A intensidade passa a depender de onde o jogador está, e não do bioma de cada sala desenhada. Por isso, salas vizinhas visíveis compartilham o mesmo nível de escuro. Novos ambientes só precisam de uma entrada na tabela. Quando o filtro for implementado, `docs/sistemas/props-e-iluminacao.md` deve ser atualizado.
