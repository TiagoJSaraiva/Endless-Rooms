<#
.SYNOPSIS
  Pipeline de verificação: toolchain, build do jogo, testes headless e testes de renderização.
  Para na primeira etapa que falhar e retorna exit code 1.

.PARAMETER Fast
  Pula os testes de renderização (que abrem uma janela oculta).

.EXAMPLE
  .\scripts\check.ps1          # completo: antes de concluir uma tarefa
  .\scripts\check.ps1 -Fast    # rápido: após cada passo e antes de cada commit
#>
param([switch]$Fast)

$ErrorActionPreference = "Continue"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$gameDir = Join-Path $root "game"

if ($env:COMPILER_PATH) {
    $env:PATH = "$env:COMPILER_PATH;$env:PATH"
}

function Invoke-Step([string]$Name, [scriptblock]$Action) {
    Write-Host "`n== $Name ==" -ForegroundColor Cyan
    $global:LASTEXITCODE = 0
    & $Action
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FALHOU: $Name" -ForegroundColor Red
        exit 1
    }
    Write-Host "OK: $Name" -ForegroundColor Green
}

# O Makefile usa caminhos relativos a game/; -C preserva isso sem mudar o diretório do chamador.
$jobs = if ($env:NUMBER_OF_PROCESSORS) { $env:NUMBER_OF_PROCESSORS } else { 4 }
$renderTargets = @(
    "test-floor-render",
    "test-wall-render",
    "test-prop-render",
    "test-wall-prop-render",
    "test-reward-pouch-render"
)

# 1. Toolchain e versão da raylib
Invoke-Step "Toolchain" {
    & (Join-Path $PSScriptRoot "check-toolchain.ps1")
}

# 2. Build do jogo (inclui a raylib vendorizada)
Invoke-Step "Build do jogo (mingw32-make game)" {
    mingw32-make -C $gameDir -j $jobs game
}

# 3. Testes headless
Invoke-Step "Testes (mingw32-make test)" {
    mingw32-make -C $gameDir -j $jobs test
}

# 4. Testes de renderização: precisam de contexto gráfico (janela oculta)
if ($Fast) {
    Write-Host "`n(-Fast: testes de renderização pulados)" -ForegroundColor DarkGray
} else {
    # Sem -j: cada alvo executa o teste na receita, e janelas simultâneas competem pelo contexto.
    Invoke-Step "Testes de renderização" {
        mingw32-make -C $gameDir @renderTargets
    }
}

Write-Host "`nCHECK OK" -ForegroundColor Green
exit 0
