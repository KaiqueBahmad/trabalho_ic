#ifndef ENTRADA_H
#define ENTRADA_H

#include "tipos.h"

void entrada_ler(char *buf, int size);
/* Le uma linha sem imprimir o prompt; consome primeiro o que foi digitado
   durante a narracao (ver entrada_pushback_byte). Retorna 0 em fim de arquivo. */
int  entrada_ler_linha(char *buf, int size);
/* Guarda uma tecla digitada durante a narracao para a proxima leitura,
   preservando o que o jogador adiantou em vez de descarta-lo. */
void entrada_pushback_byte(char c);
void entrada_normalizar(char *buf);
int  entrada_eh_numero(const char *buf);
int  entrada_to_numero(const char *buf);
int  entrada_confirmar(const char *pergunta);

#endif
