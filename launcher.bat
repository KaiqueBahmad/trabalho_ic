@echo off
setlocal
set PIPER_URL=https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_windows_amd64.zip
set PIPER_ZIP=piper_windows_amd64.zip

if not exist "piper\" (
    echo Baixando piper...
    curl -sL -o "%PIPER_ZIP%" "%PIPER_URL%" >nul 2>&1
    if errorlevel 1 ( echo Erro ao baixar piper. & exit /b 1 )
    echo Extraindo piper...
    tar -xf "%PIPER_ZIP%" >nul 2>&1
    if errorlevel 1 ( echo Erro ao extrair piper. & exit /b 1 )
    del "%PIPER_ZIP%" >nul 2>&1
)

if not exist "models\" (
    echo Baixando modelos de voz...
    mkdir models >nul 2>&1
    curl -sL -o "models\pt_BR-faber-medium.onnx.json" "https://raw.githubusercontent.com/KaiqueBahmad/trabalho_ic/main/models/pt_BR-faber-medium.onnx.json" >nul 2>&1
    curl -sL -o "models\pt_BR-faber-medium.onnx" "https://raw.githubusercontent.com/KaiqueBahmad/trabalho_ic/main/models/pt_BR-faber-medium.onnx" >nul 2>&1
    if errorlevel 1 ( echo Erro ao baixar modelos. & exit /b 1 )
)

if not exist "main.exe" (
    echo Baixando o jogo...
    curl -sL -o "main.exe" "https://raw.githubusercontent.com/KaiqueBahmad/trabalho_ic/main/main.exe" >nul 2>&1
    if errorlevel 1 ( echo Erro ao baixar o jogo. & exit /b 1 )
)

main.exe

endlocal
