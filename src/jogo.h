#ifndef JOGO_H
#define JOGO_H

#include "jogador.h"
#include "inventario.h"

extern Jogador    g_jogador;
extern Inventario g_inventario;
extern int        g_audio_ativado;

void jogo_iniciar(void);
void jogo_loop(void);
int  jogo_verificar_fim(void);
void jogo_mostrar_final(void);
int  jogo_processar_global(const char *cmd);

#endif
