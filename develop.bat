@echo off
setlocal

set REPO_URL=https://github.com/KaiqueBahmad/trabalho_ic.git
set REPO_DIR=trabalho_ic

set MINGIT_URL=https://github.com/git-for-windows/git/releases/download/v2.45.2.windows.1/MinGit-2.45.2-64-bit.zip
set MINGIT_ZIP=mingit.zip

echo === O Peso da Coroa - Ambiente de Desenvolvimento ===
echo.

REM --- Git ---
where git >nul 2>&1
if %errorlevel%==0 (
    echo Git ja encontrado no sistema, usando o existente.
    set "GIT_CMD=git"
) else (
    if not exist "mingit\cmd\git.exe" (
        echo Git nao encontrado. Baixando Git portatil ^(MinGit, ~50MB^)...
        curl -L -o "%MINGIT_ZIP%" "%MINGIT_URL%"
        if errorlevel 1 ( echo Erro ao baixar o Git. Verifique sua conexao. & exit /b 1 )
        echo Extraindo Git...
        if not exist "mingit\" mkdir mingit
        tar -xf "%MINGIT_ZIP%" -C mingit
        if errorlevel 1 ( echo Erro ao extrair o Git. & exit /b 1 )
        del "%MINGIT_ZIP%"
        echo Git portatil instalado em mingit\
    ) else (
        echo Git portatil ja instalado em mingit\
    )
    set "GIT_CMD=%CD%\mingit\cmd\git.exe"
)
echo.

REM --- Clonar o projeto ---
if exist "%REPO_DIR%\" (
    echo A pasta "%REPO_DIR%" ja existe. Pulando o clone.
    echo Para atualizar, entre na pasta e rode: git pull
) else (
    echo Clonando o projeto de %REPO_URL% ...
    "%GIT_CMD%" clone "%REPO_URL%" "%REPO_DIR%"
    if errorlevel 1 ( echo Erro ao clonar o repositorio. & exit /b 1 )
    echo Projeto clonado em %REPO_DIR%\
)
echo.

echo Pronto. Agora:
echo   1. Entre na pasta:  cd %REPO_DIR%
echo   2. Compile e jogue: run.bat
echo.

endlocal
