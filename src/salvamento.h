#ifndef SALVAMENTO_H
#define SALVAMENTO_H

#include "jogador.h"
#include "inventario.h"

int salvar_jogo(const Jogador *j, const Inventario *inv);
int carregar_jogo(Jogador *j, Inventario *inv);

#endif
