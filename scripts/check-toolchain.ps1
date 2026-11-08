# Confere o toolchain (MinGW-w64 + GNU Make) e a versão da raylib vendorizada.
# Exit 0 = ok; exit 1 = problema. Roda no hook SessionStart do Claude Code e no início do check.ps1.
#
# Se o MinGW não estiver no PATH, defina COMPILER_PATH (mesma variável aceita pelo Makefile)
# com a pasta bin do toolchain.

$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$failed = $false

if ($env:COMPILER_PATH) {
    $env:PATH = "$env:COMPILER_PATH;$env:PATH"
}

foreach ($tool in @("g++", "gcc", "ar", "mingw32-make")) {
    $cmd = Get-Command $tool -ErrorAction SilentlyContinue
    if (-not $cmd) {
        Write-Output "ERRO: '$tool' não encontrado no PATH. Instale o MinGW-w64 ou defina COMPILER_PATH."
        $failed = $true
    }
}

if (-not $failed) {
    $gccVersion = (& g++ --version 2>&1 | Select-Object -First 1)
    Write-Output "OK: $gccVersion"
}

# A versão fixada no Makefile deve corresponder à cópia em vendor/ (decisão 0001).
$makefile = Join-Path $root "game\Makefile"
$raylibHeader = Join-Path $root "game\vendor\raylib\src\raylib.h"

if (-not (Test-Path $raylibHeader)) {
    Write-Output "ERRO: raylib vendorizada não encontrada em game/vendor/raylib."
    $failed = $true
} else {
    $headerMatch = Select-String -Path $raylibHeader -Pattern '#define RAYLIB_VERSION\s+"([\d.]+)"'
    $makeMatch = Select-String -Path $makefile -Pattern '^RAYLIB_VERSION\s*:?=\s*([\d.]+)'
    if (-not $headerMatch -or -not $makeMatch) {
        Write-Output "AVISO: não foi possível ler a versão da raylib no header ou no Makefile."
    } else {
        $vendored = $headerMatch.Matches[0].Groups[1].Value
        $pinned = $makeMatch.Matches[0].Groups[1].Value
        if ($pinned.StartsWith($vendored)) {
            Write-Output "OK: raylib $vendored vendorizada (Makefile: $pinned)"
        } else {
            Write-Output "ERRO: raylib vendorizada é $vendored, mas o Makefile fixa $pinned."
            $failed = $true
        }
    }
}

if ($failed) { exit 1 }
exit 0
