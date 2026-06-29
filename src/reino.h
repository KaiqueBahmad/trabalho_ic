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

    /* arco de Drakmar */
    int soldados;          /* exercito */
    int muralhas;          /* nivel de fortificacao */
    int fase_drakmar;      /* 0 paz, 1 rumores, 2 tributo, 3 guerra */
    int ameaca;            /* forca de Drakmar */
    int tributos_pagos;    /* quantas vezes cedeu ao tributo */

    /* controle */
    int jogo_encerrado;
    int final_obtido;      /* 0 nenhum; ver jogo.c */
} Reino;

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
