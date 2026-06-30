#!/bin/sh
set -e

PIPER_BASE_URL="https://github.com/rhasspy/piper/releases/download/2023.11.14-2"

echo "=== O Peso da Coroa - Compilador ==="
echo ""

# Detectar sistema operacional
OS=$(uname -s)
case "$OS" in
    Linux*)
        PIPER_FILE="piper_linux_x86_64.tar.gz"
        COMPILE_FLAGS=""
        ;;
    Darwin*)
        PIPER_FILE="piper_macos_aarch64.tar.gz"
        COMPILE_FLAGS=""
        ;;
    MINGW*|MSYS*|CYGWIN*)
        PIPER_FILE="piper_windows_amd64.zip"
        COMPILE_FLAGS="-lwinmm"
        ;;
    *)
        echo "Sistema operacional nao suportado: $OS"
        exit 1
        ;;
esac

echo "Sistema detectado: $OS"
echo ""

# Baixar piper se necessario
if [ ! -d "piper" ]; then
    echo "Baixando piper ($PIPER_FILE)..."
    if command -v curl > /dev/null 2>&1; then
        curl -L -o "$PIPER_FILE" "$PIPER_BASE_URL/$PIPER_FILE"
    elif command -v wget > /dev/null 2>&1; then
        wget -O "$PIPER_FILE" "$PIPER_BASE_URL/$PIPER_FILE"
    else
        echo "Erro: curl ou wget nao encontrado."
        exit 1
    fi

    echo "Extraindo piper..."
    case "$PIPER_FILE" in
        *.tar.gz) tar -xzf "$PIPER_FILE" ;;
        *.zip)    unzip -q "$PIPER_FILE" ;;
    esac

    rm -f "$PIPER_FILE"
    echo "Piper instalado em piper/"
else
    echo "Piper ja instalado em piper/"
fi

echo ""
echo "Compilando..."
gcc main.c src/audio.c src/entrada.c src/ui.c src/salvamento.c \
    src/utils.c src/reino.c src/eventos.c src/mundo.c src/jogo.c \
    -I. -o main $COMPILE_FLAGS

echo "Compilacao concluida. Execute ./main para jogar."
