#ifndef NPCS_H
#define NPCS_H

#include "jogador.h"
#include "inventario.h"

void npc_conselheiro(Jogador *j, Inventario *inv);
void npc_general(Jogador *j, Inventario *inv);
void npc_comerciante(Jogador *j, Inventario *inv);
void npc_prisioneiro(Jogador *j, Inventario *inv);
void npc_emissario(Jogador *j, Inventario *inv);

#endif
