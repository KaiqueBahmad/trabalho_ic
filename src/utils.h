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

/* Numeros aleatorios. utils_semear deve ser chamado uma vez no inicio. */
void utils_semear(void);
int  utils_rand(int min, int max);   /* inteiro entre min e max, inclusive */
int  utils_chance(int percentual);   /* 1 com probabilidade percentual (0-100) */

#endif
