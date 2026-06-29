#ifndef SALVAMENTO_H
#define SALVAMENTO_H

#include "reino.h"

int salvar_reino(const Reino *r);
int carregar_reino(Reino *r);

/* Preferencias globais (velocidade da narracao) persistidas entre sessoes. */
void salvar_config(void);
void carregar_config(void);

#endif
