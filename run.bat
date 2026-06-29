@echo off
setlocal enabledelayedexpansion

set PIPER_URL=https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_windows_amd64.zip
set PIPER_ZIP=piper_windows_amd64.zip

set MINGW_URL=https://github.com/brechtsanders/winlibs_mingw/releases/download/16.1.0posix-14.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64ucrt-14.0.0-r2.zip
set MINGW_ZIP=mingw64_gcc.zip

set MODEL_BASE=https://raw.githubusercontent.com/KaiqueBahmad/trabalho_ic/main/models

echo === O Peso da Coroa - Instalar, Compilar e Jogar ===
echo.

REM --- Piper (narracao por voz) ---
if not exist "piper\" (
    echo Baixando piper para Windows...
    curl -L -o "%PIPER_ZIP%" "%PIPER_URL%"
    if errorlevel 1 ( echo Erro ao baixar piper. Verifique sua conexao. & exit /b 1 )
    echo Extraindo piper...
    tar -xf "%PIPER_ZIP%"
    if errorlevel 1 ( echo Erro ao extrair piper. & exit /b 1 )
    del "%PIPER_ZIP%"
    echo Piper instalado em piper\
) else (
    echo Piper ja instalado em piper\
)
echo.

REM --- Modelo de voz ---
if not exist "models\pt_BR-faber-medium.onnx" (
    echo Baixando modelo de voz...
    if not exist "models\" mkdir models
    curl -L -o "models\pt_BR-faber-medium.onnx.json" "%MODEL_BASE%/pt_BR-faber-medium.onnx.json"
    curl -L -o "models\pt_BR-faber-medium.onnx" "%MODEL_BASE%/pt_BR-faber-medium.onnx"
    if errorlevel 1 ( echo Erro ao baixar o modelo de voz. & exit /b 1 )
    echo Modelo de voz instalado em models\
) else (
    echo Modelo de voz ja instalado em models\
)
echo.

REM --- GCC (compilador) ---
where gcc >nul 2>&1
if %errorlevel%==0 (
    echo GCC ja encontrado no sistema, usando o existente.
    set "GCC_CMD=gcc"
) else (
    if not exist "mingw64\bin\gcc.exe" (
        echo GCC nao encontrado. Baixando compilador portatil ^(MinGW-w64, ~260MB^)...
        echo Isso pode demorar dependendo da sua conexao.
        curl -L -o "%MINGW_ZIP%" "%MINGW_URL%"
        if errorlevel 1 ( echo Erro ao baixar o GCC. Verifique sua conexao. & exit /b 1 )
        echo Extraindo GCC...
        tar -xf "%MINGW_ZIP%"
        if errorlevel 1 ( echo Erro ao extrair o GCC. & exit /b 1 )
        del "%MINGW_ZIP%"
        echo GCC instalado em mingw64\
    ) else (
        echo GCC portatil ja instalado em mingw64\
    )
    set "GCC_CMD=%CD%\mingw64\bin\gcc.exe"
)
echo.

REM --- Compilar ---
echo Compilando...
"%GCC_CMD%" main.c src/audio.c src/entrada.c src/ui.c src/salvamento.c src/utils.c src/reino.c src/eventos.c src/jogo.c -I. -o main.exe -lwinmm
if errorlevel 1 ( echo Erro na compilacao. & exit /b 1 )
echo Compilacao concluida.
echo.

REM --- Jogar ---
echo Iniciando o jogo...
echo.
main.exe

endlocal
