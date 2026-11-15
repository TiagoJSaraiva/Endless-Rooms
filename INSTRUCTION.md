# Instruções de build e execução

## Pré-requisitos

- GNU Make; no Windows, use `mingw32-make`.
- MinGW-w64 com `g++`, `gcc` e `ar` disponíveis no `PATH`.
- A Raylib 5.5 já está incluída em `game/vendor/raylib` e é compilada junto com o jogo.

O compilador e o Make não são incluídos no repositório. Se o MinGW não estiver no `PATH`, informe a pasta do toolchain com `COMPILER_PATH`, por exemplo:

```powershell
mingw32-make game COMPILER_PATH=C:/caminho/do/mingw/bin
```

## Compilar e executar

Execute os comandos a partir da raiz do repositório. O jogo deve ser iniciado com `game/` como diretório atual porque os caminhos dos assets são relativos a essa pasta.

```powershell
Set-Location game
mingw32-make game
.\game.exe
```

## Testes

Dentro de `game/`, rode:

```powershell
mingw32-make test
```

Os testes atuais não precisam de framework externo.

## Opções de teste local

Para forçar o bioma das salas geradas em uma run, crie `game/biome.txt` com `cave`, `mansion` ou `dungeon`. O jogo lê esse arquivo ao iniciar, usando `game/` como diretório de trabalho. O lobby não muda. Se o arquivo estiver ausente ou vazio, o bioma continua sendo sorteado; um valor inválido também usa o sorteio e exibe um aviso.

Com `game/debug.txt` contendo `true`, o modo debug equipa o Kit do Testador e o inclui nos pools de loja e baú. Os dois arquivos são locais e não são versionados.

## Build para distribuição (Windows)

```powershell
mingw32-make game PORTABLE=TRUE
```

Gera um `game.exe` sem dependência das DLLs do MinGW e sem janela de console. Para distribuir, basta o executável e a pasta `assets/` ao lado dele.

## Build web

Exige o [Emscripten SDK](https://emscripten.org) instalado e ativado no terminal (`emsdk_env`). Dentro de `game/`:

```powershell
mingw32-make clean
mingw32-make game PLATFORM=PLATFORM_WEB
```

A página sai em `game/build-web/` (`index.html`, `.js`, `.wasm` e `.data`) e precisa ser servida por HTTP, por exemplo com `python -m http.server -d build-web`. O `clean` antes e depois evita misturar objetos da raylib de desktop e web.

## Publicação

- Todo push na `main` recompila a versão web e a publica no GitHub Pages (`.github/workflows/deploy-web.yml`).
- Para publicar uma release com o zip para Windows, crie e envie uma tag `v*`:

```powershell
git tag v1.0.0
git push origin v1.0.0
```

O workflow `.github/workflows/release-windows.yml` compila com `PORTABLE=TRUE`, roda os testes e anexa `EndlessRooms-windows.zip` à release.

## Limpar arquivos gerados

Dentro de `game/`, rode:

```powershell
mingw32-make clean
```

O comando remove os objetos e executáveis gerados pelo projeto, além dos objetos e bibliotecas compilados dentro da Raylib vendorizada. Ele preserva os assets e as DLLs do projeto.

## Mais informações

- [Arquitetura e módulos](docs/arquitetura.md)
- [Orientações para agentes](AGENTS.md)
