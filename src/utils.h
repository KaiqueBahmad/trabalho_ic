#ifndef UTILS_H
#define UTILS_H

typedef enum {
    SO_WINDOWS,
    SO_LINUX,
    SO_MACOS,
    SO_DESCONHECIDO
} SistemaOperacional;

SistemaOperacional utils_detectar_so(void);
const char *utils_nome_so(SistemaOperacional so);

#endif
