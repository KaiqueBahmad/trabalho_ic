#include "utils.h"

SistemaOperacional utils_detectar_so(void) {
#if defined(_WIN32) || defined(_WIN64)
    return SO_WINDOWS;
#elif defined(__linux__)
    return SO_LINUX;
#elif defined(__APPLE__) && defined(__MACH__)
    return SO_MACOS;
#else
    return SO_DESCONHECIDO;
#endif
}

const char *utils_nome_so(SistemaOperacional so) {
    switch (so) {
        case SO_WINDOWS:     return "Windows";
        case SO_LINUX:       return "Linux";
        case SO_MACOS:       return "macOS";
        default:             return "Desconhecido";
    }
}
