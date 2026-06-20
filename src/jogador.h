#ifndef JOGADOR_H
#define JOGADOR_H

#include "tipos.h"

typedef struct {
    char nome[NOME_MAX];
    int  vida;
    int  ouro;
    int  exercito;
    int  popularidade;
    int  sabedoria;
    Local local_atual;
    int  turno;
    /* flags de progresso */
    int emissario_recebido;
    int prisioneiro_libertado;
    int alianca_formada;
    int exercito_reforcado;
    int povo_ajudado;
    int traidor_revelado;
    int batalha_travada;
    int missao_povo_iniciada;
    int conselheiro_preso;
    int erik_falou_aldric;
    int info_reino_veira;
    /* controle */
    int jogo_encerrado;
    int final_obtido; /* 0=nenhum 1=militar 2=diplomatico 3=heroi 4=tragico 5=secreto */
} Jogador;

void jogador_inicializar(Jogador *j, const char *nome);
void jogador_mostrar_status(const Jogador *j);
void jogador_alterar_vida(Jogador *j, int delta);
void jogador_alterar_ouro(Jogador *j, int delta);
void jogador_alterar_exercito(Jogador *j, int delta);
void jogador_alterar_popularidade(Jogador *j, int delta);
void jogador_alterar_sabedoria(Jogador *j, int delta);

#endif
