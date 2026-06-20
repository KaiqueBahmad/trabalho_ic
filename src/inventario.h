#ifndef INVENTARIO_H
#define INVENTARIO_H

#include "tipos.h"

typedef struct {
    TipoItem tipo;
    char     nome[NOME_MAX];
    char     descricao[DESC_MAX];
} Item;

typedef struct {
    Item itens[MAX_ITENS];
    int  quantidade;
} Inventario;

void        inventario_inicializar(Inventario *inv);
int         inventario_adicionar(Inventario *inv, TipoItem tipo);
int         inventario_remover(Inventario *inv, TipoItem tipo);
int         inventario_tem(const Inventario *inv, TipoItem tipo);
void        inventario_mostrar(const Inventario *inv);
void        inventario_examinar_todos(const Inventario *inv);
const char *item_get_nome(TipoItem tipo);
const char *item_get_descricao(TipoItem tipo);

#endif
