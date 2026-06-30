@echo off
setlocal

set PIPER_URL=https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_windows_amd64.zip
set PIPER_ZIP=piper_windows_amd64.zip

set MINGW_URL=https://github.com/brechtsanders/winlibs_mingw/releases/download/16.1.0posix-14.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64ucrt-14.0.0-r2.zip
set MINGW_ZIP=mingw64_gcc.zip

echo === O Peso da Coroa - Compilador ===
echo.

REM --- Piper ---
if not exist "piper\" (
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
    del "%PIPER_ZIP%"
    echo Piper instalado em piper\
) else (
    echo Piper ja instalado em piper\
)
echo.

REM --- GCC ---
where gcc >nul 2>&1
if %errorlevel%==0 (
    echo GCC ja encontrado no sistema, usando o existente.
    set "GCC_CMD=gcc"
) else (
    if not exist "mingw64\bin\gcc.exe" (
        echo GCC nao encontrado. Baixando compilador portatil ^(MinGW-w64, ~260MB^)...
        echo Isso pode demorar dependendo da sua conexao.
        curl -L -o "%MINGW_ZIP%" "%MINGW_URL%"
        if errorlevel 1 (
            echo Erro ao baixar o GCC. Verifique sua conexao.
            exit /b 1
        )
        echo Extraindo GCC...
        tar -xf "%MINGW_ZIP%"
        if errorlevel 1 (
            echo Erro ao extrair o GCC.
            exit /b 1
        )
        del "%MINGW_ZIP%"
        echo GCC instalado em mingw64\
    ) else (
        echo GCC portatil ja instalado em mingw64\
    )
    set "GCC_CMD=%CD%\mingw64\bin\gcc.exe"
)
echo.

echo Compilando...
"%GCC_CMD%" main.c src/audio.c src/entrada.c src/ui.c src/salvamento.c src/utils.c src/reino.c src/eventos.c src/mundo.c src/jogo.c -I. -o main.exe -lwinmm
if errorlevel 1 (
    echo Erro na compilacao.
    exit /b 1
)

echo Compilacao concluida. Execute main.exe para jogar.
endlocal
