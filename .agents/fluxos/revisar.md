# Fluxo: revisar

1. Revise o diff indicado: alterações locais (`git diff` e `git diff --staged`), um intervalo de commits (`git diff <de>..<até>`) ou os arquivos citados. Sem indicação, revise as alterações locais; se não houver, o último commit.
2. Compare com `docs/convencoes-cpp.md` e com as decisões em `docs/decisoes/`.
3. Reporte separado nestas categorias, omitindo as vazias:
   - **Bugs reais**
   - **Memória e tempo de vida** (ponteiro ou referência que pode ficar inválido, realocação de `vector`, textura não descarregada, dono destruído)
   - **Edge cases** não tratados
   - **Performance** (alocação ou carregamento de asset por frame, buscas lineares no loop quente)
   - **Robustez** (asset ausente, valor de configuração inválido, sala sem vizinhos)
   - **Responsabilidade e acoplamento** (regra de jogo em renderizador ou UI, desenho em regra, `main.cpp` crescendo com lógica que cabe em um módulo, variação de conteúdo em `if`/`switch` em vez de tabela; ver decisão 0007)
   - **Contratos** (header público alterado sem necessidade, pré-condição não documentada)
   - **Testes** (regra nova sem teste headless, teste que depende de RNG sem seed fixa)
   - **Manutenibilidade**
   - **Legibilidade**
   - **Preferência de estilo** (opinativo, baixa prioridade)
4. Para cada item: arquivo e linha, o problema e a sugestão. Ordene por gravidade.
5. Não altere nada até o desenvolvedor escolher o que aplicar.
