#ifndef MUNDO_H
#define MUNDO_H

#include "reino.h"

/* O mapa tem 6 reinos: o jogador e mais NUM_NPCS reinos rivais. */
#define NUM_NPCS    5
#define JORNAL_MAX  10

/* Um reino rival controlado pela simulacao. */
typedef struct {
    char nome[NOME_MAX];
    int  ativo;            /* 1 enquanto nao for anexado */

    int  populacao;
    int  soldados;
    int  muralha_nivel;
    int  muralha_vida;
    int  ouro;
    int  comida;
    int  gado;
    int  minas;
} ReinoNPC;

typedef struct {
    ReinoNPC npc[NUM_NPCS];
    char jornal[JORNAL_MAX][220];
    int  jornal_n;         /* quantas manchetes guardadas (0..JORNAL_MAX) */
} Mundo;

void mundo_inicializar(Mundo *m);

/* Quantos reinos ainda existem no mapa, contando o jogador. */
int  mundo_reinos_ativos(const Mundo *m);

/* Avanca o mundo uma estacao: os NPCs evoluem e podem atacar uns aos outros
   e o jogador. Se o jogador for conquistado, marca o fim do jogo nele. */
void mundo_nova_estacao(Mundo *m, Reino *jogador);

/* Adiciona uma manchete ao jornal (rola as antigas para fora). */
void mundo_jornal_add(Mundo *m, const char *texto);

/* Submenu "Reinos": atacar, obter informacoes, jornal. */
void mundo_menu_reinos(Mundo *m, Reino *jogador);

#endif
