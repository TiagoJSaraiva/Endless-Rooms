# AGENTS.md — Endless Rooms

Instruções para qualquer agente de IA (Claude Code, Codex e outros) que trabalhe neste repositório. Este arquivo é a fonte da verdade; `CLAUDE.md` apenas o importa.

## Projeto

- Endless Rooms: roguelite 2D em **C++17** com **raylib 5.5**. Caminho até a demo em `docs/status.md`.
- Ambiente: Windows, PowerShell, MinGW-w64 (`g++`, `gcc`, `ar`) e GNU Make (`mingw32-make`).
- Código e assets de runtime em `game/`. Os caminhos dos assets são relativos ao diretório de trabalho: execute o jogo com `game/` como diretório atual (`game.exe`).
- O desenvolvedor lê e revisa todo o código. Legibilidade vale mais que esperteza.
- Idioma: responda, comente e documente em **português**. Identificadores e nomes de arquivo em **inglês**.

## Comece aqui

1. Leia `docs/status.md`: etapa do roadmap, onde paramos, próximo passo, bloqueios.
2. Antes de mexer em um sistema, leia `docs/arquitetura.md` (mapa de módulos) e a nota do sistema em `docs/sistemas/`.
3. Antes de escrever C++, leia `docs/convencoes-cpp.md`.
4. Antes de propor mudança estrutural, consulte `docs/decisoes/`.

## Fluxo de trabalho

Procedimentos detalhados em `.agents/fluxos/` (também expostos como comandos no Claude Code):

| Fluxo | Quando |
|---|---|
| `inicio.md` | Início de sessão |
| `planejar.md` | Etapa do roadmap (levantar requisitos e quebrar em subtarefas) ou tarefa que crie sistema, altere contrato de header ou mexa em mais de ~3 arquivos |
| `revisar.md` | Revisão de diff, a pedido |
| `fim.md` | Fim de sessão ou de tarefa |

Regras gerais:
- Tarefa não trivial: **plano primeiro, código só após aprovação**.
- Antes de editar, confira `git status` e preserve alterações locais fora do escopo.
- Mudanças pequenas e incrementais. Um passo verificável por vez.
- Não refatore o que não foi pedido; sugira no resumo.
- Dúvida real de design ou escopo: pergunte em vez de supor.
- Toda resposta a prompts neste projeto termina com a mensagem de commit: os commits feitos na resposta (hash curto e mensagem) ou, quando nada foi commitado, uma sugestão de mensagem para o trabalho em questão.

## Commits

Decisão `docs/decisoes/0008-commits-pelo-agente.md`.

- O agente **faz commits** durante e ao fim da implementação: um commit por módulo concluído (parte coesa da tarefa: uma regra com seus testes, um renderizador, a integração no `main.cpp`, a atualização de `docs/`).
- Tudo direto na `main`. **Nunca** faça `push`, não crie branches e não reescreva histórico (`--amend`, `rebase`, `reset` de commits) sem pedido explícito.
- Antes de cada commit: `scripts/check.ps1 -Fast` passando. Nunca use `--no-verify`.
- Adicione só os arquivos do módulo (`git add <caminhos>`), nunca alterações locais fora do escopo nem artefatos de build.
- Mensagem: Conventional Commits em português, `<tipo>[(escopo)]: <verbo no infinitivo> <o quê>`, minúscula, sem ponto final, até ~72 caracteres. Corpo opcional explicando o porquê.
  - Tipos: `feat`, `fix`, `refactor`, `test`, `docs`, `chore`, `perf`.
  - Exemplos: `feat: adicionar props de parede com tochas animadas`, `fix(walls): corrigir altura do side wall no canto superior`.
- **Sem** declaração de coautoria (`Co-Authored-By`) e sem menção ao agente na mensagem.

## Regras essenciais de C++

Detalhes e exemplos em `docs/convencoes-cpp.md`. O mínimo inegociável:
- **Headers públicos são contratos** entre módulos. Extrações internas preservam comportamento e APIs públicas; mudança de assinatura é citada no plano e no resumo.
- **Regras separadas de apresentação** (decisão 0007): regras de jogo em funções testáveis sem janela (`<dominio>_rules`, `namespace <Dominio>Rules`); renderizadores e UI não decidem regra.
- **Tempo de vida explícito**: nada de `new`/`delete` manuais; nenhum ponteiro para objeto que pode morrer ou realocar sem mecanismo de validade (ver `docs/sistemas/diagnostico-de-crashes.md`).
- Conteúdo variável (catálogos, pools, configs por bioma) em tabelas de dados, não em `if`/`switch` espalhados.
- Sem números mágicos: constantes nomeadas.
- Comentários explicam **por quê**, não o quê.
- `main.cpp` é ponto de integração: lógica nova nasce em módulo próprio.

## Build e verificação

Comandos (a partir da raiz do repositório):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check.ps1 -Fast   # toolchain + build + testes headless
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check.ps1         # + testes de renderização
```

- Manualmente, dentro de `game/`: `mingw32-make game`, `mingw32-make test`, `mingw32-make test-<dominio>-render`, `mingw32-make clean`.
- `COMPILER_PATH` é opcional e adiciona ao `PATH` a pasta do toolchain quando ela não está configurada globalmente (Makefile e scripts).
- Durante a implementação: `-Fast` após cada passo relevante e antes de cada commit.
- **Antes de declarar uma tarefa concluída: check completo passando.**
- Nunca apague, pule ou enfraqueça um teste para o check passar. Se a falha não for sua ou não tiver solução clara, reporte.
- Avisos `IMAGE: Failed to load image data` e `Cannot load assets/...` nos testes de renderização são esperados: testam o fallback de asset ausente.

## Testes

- Toda regra de gameplay que puder ser isolada ganha teste headless em `game/tests/<dominio>_tests.cpp` (decisão 0006).
- Testes que precisam de janela vão para `<dominio>_render_tests.cpp`, com alvo `test-<dominio>-render`, e entram na lista de `scripts/check.ps1`.
- Novo executável: alvo e fontes no Makefile, dependência em `test`, entrada em `clean` e linha em `game/.gitignore`.
- Game feel e fluxos visuais não são testáveis: descreva no resumo o que o desenvolvedor deve validar jogando.

## Memória do projeto (`docs/`)

A memória entre sessões é a pasta `docs/`, versionada com o código.

| Arquivo | Conteúdo | Atualizar quando |
|---|---|---|
| `status.md` | Roadmap até a demo, etapa atual e subtarefas, próximo passo, bloqueios | Fim de toda sessão |
| `arquitetura.md` | Módulos, pastas, estado por partida, build | Mudou algum desses |
| `convencoes-cpp.md` | Padrões de código | Convenção nova aceita |
| `sistemas/<sistema>.md` | Como um sistema funciona, regras, contratos, testes | Criou ou alterou o sistema |
| `decisoes/NNNN-titulo.md` | Decisão não trivial: contexto, escolha, alternativas | Tomou uma decisão estrutural |

Regras:
- Notas curtas, factuais e atuais. Prefira **editar** uma nota a criar outra.
- Registre o que é verdade sobre o projeto, não o histórico de tentativas.
- Atualize a nota no mesmo passo (e no mesmo commit) da mudança que a tornou desatualizada.
- Uma decisão aceita só muda com uma nova decisão que a substitua (marque a antiga como substituída).
- Ao criar ou alterar uma regra, convenção ou decisão, confira se ela contradiz `AGENTS.md`, `docs/convencoes-cpp.md` ou uma decisão aceita. Se contradisser, não aplique: aponte o conflito ao desenvolvedor e decidam juntos.

## Arquivos: cuidados

- `game/vendor/raylib/` e `game/inc/` (raygui, rlcpp): terceiros. **Não editar**; preservar os avisos de licença.
- Não adicione objetos, executáveis ou bibliotecas geradas ao Git: `game/obj/`, `*.exe`, executáveis de `game/tests/` e saídas de build de `game/vendor/raylib/src/` são artefatos locais.
- `game/Makefile`: `clean` remove apenas saídas de build do projeto e da raylib vendorizada.
- `game/biome.txt` é configuração local de teste (força o bioma da run) e não é versionado.
- `game/debug.txt` é configuração local de teste (`true` liga o modo debug: Kit do Testador equipado e nos pools de loja e baú) e não é versionado.
- `anotacoes/` é pessoal e ignorada pelo Git; não a publique nem altere sem pedido explícito.
- Validação jogando: não há ferramenta para rodar e observar o jogo; quando necessário, peça ao desenvolvedor para rodar `game.exe` e reportar.
