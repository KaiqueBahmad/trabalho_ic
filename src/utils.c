#include "utils.h"
#include <stdlib.h>
#include <time.h>

void utils_semear(void) {
    srand((unsigned int)time(NULL));
}

int utils_rand(int min, int max) {
    if (max <= min) return min;
    return min + rand() % (max - min + 1);
}

int utils_chance(int percentual) {
    return (rand() % 100) < percentual;
}

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
