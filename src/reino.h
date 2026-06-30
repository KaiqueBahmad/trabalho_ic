#ifndef REINO_H
#define REINO_H

#include "tipos.h"

/* Estado completo do reino de Avalon. Tudo sao recursos concretos. */
typedef struct {
    char nome[NOME_MAX];   /* nome do rei */

    int ano;               /* ano de reinado, comeca em 1 */
    Estacao estacao;       /* estacao atual */

    int populacao;         /* habitantes (mao de obra, base de impostos) */
    int comida;            /* graos no celeiro */
    int gado;              /* cabecas de gado */
    int campos;            /* campos cultivaveis */
    int ouro;              /* tesouro real */
    int imposto;           /* aliquota de imposto em porcentagem */

    int clima;             /* qualidade do ano para a lavoura, em % (100 = normal) */

    /* militar */
    int soldados;          /* exercito (so do jogador pode mobilizar para atacar) */
    int muralha_nivel;     /* nivel de fortificacao (0 a MURALHA_MAX) */
    int muralha_vida;      /* pontos de vida atuais da muralha */

    /* economia de minas */
    int minas;             /* quantas minas o reino construiu */

    /* controle */
    int jogo_encerrado;
    int final_obtido;      /* 0 nenhum; ver jogo.c */
} Reino;

/* ---- muralhas: vida defensiva por nivel ---- */
#define MURALHA_MAX 5
/* Pontos de vida totais de uma muralha no nivel dado (0 se sem muralha). */
int reino_muralha_vida_max(int nivel);

void reino_inicializar(Reino *r, const char *nome);

/* Nome textual da estacao atual. */
const char *reino_nome_estacao(Estacao e);

/* Calculos de apoio usados pela interface. */
int reino_campos_trabalhaveis(const Reino *r); /* limite pela mao de obra */
int reino_colheita_prevista(const Reino *r);   /* estimativa para a proxima colheita */
int reino_comida_necessaria(const Reino *r);   /* consumo do proximo inverno */
int reino_capacidade_pasto(const Reino *r);    /* gado maximo sustentavel */

/* Avanca uma estacao, aplicando a simulacao correspondente. Escreve um
   relato do que aconteceu (producao, consumo, migracao) via ui. */
void reino_avancar_estacao(Reino *r);

#endif
