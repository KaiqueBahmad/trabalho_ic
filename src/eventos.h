#ifndef EVENTOS_H
#define EVENTOS_H

#include "reino.h"

/* Possivelmente dispara um acontecimento no inicio de uma estacao. */
void eventos_talvez(Reino *r);

/* Avanca o arco de Drakmar no inicio de cada ano: rumores, tributo e guerra. */
void drakmar_inicio_de_ano(Reino *r);

/* Possivelmente dispara um encontro com Drakmar no inicio de uma estacao
   (batedores, saques, desertores), enquanto a tensao cresce antes da guerra. */
void drakmar_evento_estacao(Reino *r);

/* Descricao vaga da ameaca de Drakmar ("distante", "crescente"...), sem
   revelar o numero exato. */
const char *drakmar_ameaca_desc(const Reino *r);

#endif
