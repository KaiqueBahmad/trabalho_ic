@echo off
setlocal

set REPO_URL=https://github.com/KaiqueBahmad/trabalho_ic/archive/refs/heads/main.zip
set REPO_ZIP=trabalho_ic.zip
set REPO_DIR=trabalho_ic-main

echo === Launcher - O Peso da Coroa ===
echo.

if not exist "%REPO_DIR%\" (
    echo Baixando o projeto do GitHub...
    curl -L -o "%REPO_ZIP%" "%REPO_URL%"
    if errorlevel 1 (
        echo Erro ao baixar o projeto. Verifique sua conexao.
        pause
        exit /b 1
    )
    echo Extraindo projeto...
    tar -xf "%REPO_ZIP%"
    if errorlevel 1 (
        echo Erro ao extrair o projeto.
        pause
        exit /b 1
    )
    del "%REPO_ZIP%"
    echo Projeto extraido em %REPO_DIR%\
) else (
    echo Projeto ja extraido em %REPO_DIR%\
)
echo.

if not exist "%REPO_DIR%\compile.bat" (
    echo Erro: compile.bat nao encontrado em %REPO_DIR%\
    pause
    exit /b 1
)

echo Executando compilador do jogo...
echo.
pushd "%REPO_DIR%"
call compile.bat
popd

echo.
echo Processo concluido. Pressione qualquer tecla para sair.
pause >nul
endlocal
