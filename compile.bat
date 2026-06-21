@echo off
setlocal

set PIPER_URL=https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_windows_amd64.zip
set PIPER_ZIP=piper_windows_amd64.zip
set PIPER_DIR=piper-windows

echo === O Peso da Coroa - Compilador ===
echo.

if not exist "%PIPER_DIR%\" (
    echo Baixando piper para Windows...
    curl -L -o "%PIPER_ZIP%" "%PIPER_URL%"
    if errorlevel 1 (
        echo Erro ao baixar piper. Verifique sua conexao.
        exit /b 1
    )
    echo Extraindo piper...
    tar -xf "%PIPER_ZIP%"
    if errorlevel 1 (
        echo Erro ao extrair piper.
        exit /b 1
    )
    rename piper "%PIPER_DIR%"
    del "%PIPER_ZIP%"
    echo Piper instalado em %PIPER_DIR%\
) else (
    echo Piper ja instalado em %PIPER_DIR%\
)

echo.
echo Compilando...
gcc main.c audio/audio.c src/jogador.c src/inventario.c src/entrada.c src/ui.c src/salvamento.c src/npcs.c src/locais.c src/jogo.c src/utils.c -I. -o main.exe -lwinmm
if errorlevel 1 (
    echo Erro na compilacao.
    exit /b 1
)

echo Compilacao concluida. Execute main.exe para jogar.
endlocal
