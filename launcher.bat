@echo off
setlocal
set PIPER_URL=https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_windows_amd64.zip
set PIPER_ZIP=piper_windows_amd64.zip

echo === O Peso da Coroa ===
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

if not exist "main.exe" (
    echo Erro: main.exe nao encontrado na pasta do projeto.
    exit /b 1
)

echo Iniciando o jogo...
main.exe

endlocal
