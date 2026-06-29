#ifndef EVENTOS_H
#define EVENTOS_H

#include "reino.h"

/* Possivelmente dispara um acontecimento no inicio de uma estacao. */
void eventos_talvez(Reino *r);

/* Avanca o arco de Drakmar no inicio de cada ano: rumores, tributo e guerra. */
void drakmar_inicio_de_ano(Reino *r);

#endif
