@echo off
setlocal

echo === O Peso da Coroa - Atualizar ===
echo.

REM --- Localizar o Git: sistema, ou o portatil (mingit) ---
set "GIT_CMD="
where git >nul 2>&1
if %errorlevel%==0 (
    set "GIT_CMD=git"
) else if exist "mingit\cmd\git.exe" (
    set "GIT_CMD=%CD%\mingit\cmd\git.exe"
) else if exist "..\mingit\cmd\git.exe" (
    set "GIT_CMD=%CD%\..\mingit\cmd\git.exe"
) else if exist "trabalho_ic\mingit\cmd\git.exe" (
    set "GIT_CMD=%CD%\trabalho_ic\mingit\cmd\git.exe"
)

if not defined GIT_CMD (
    echo Git nao encontrado. Rode develop.bat primeiro ou instale o Git.
    exit /b 1
)

REM --- Localizar o repositorio e atualizar ---
if exist ".git" (
    echo Atualizando o projeto na pasta atual...
    "%GIT_CMD%" pull
) else if exist "trabalho_ic\.git" (
    echo Entrando em trabalho_ic e atualizando...
    cd trabalho_ic
    "%GIT_CMD%" pull
) else (
    echo Repositorio nao encontrado.
    echo Rode este script dentro da pasta trabalho_ic, ou na pasta que a contem.
    exit /b 1
)

if errorlevel 1 ( echo Erro ao atualizar o projeto. & exit /b 1 )

echo.
echo Atualizacao concluida. Use run.bat para compilar e jogar.

endlocal
