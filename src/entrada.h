#ifndef ENTRADA_H
#define ENTRADA_H

#include "tipos.h"

void entrada_ler(char *buf, int size);
void entrada_normalizar(char *buf);
int  entrada_eh_numero(const char *buf);
int  entrada_to_numero(const char *buf);
int  entrada_confirmar(const char *pergunta);

#endif
