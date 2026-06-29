#include "reino.h"
#include "ui.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

/* ---- parametros de equilibrio da simulacao ---- */
#define CONSUMO_POR_PESSOA 2    /* graos consumidos por habitante a cada inverno */
#define RENDIMENTO_CAMPO   24   /* graos por campo com clima normal */
#define TRAB_POR_CAMPO     6    /* habitantes necessarios para lavrar um campo */
#define PASTO_POR_CAMPO    5    /* cabecas de gado que cada campo sustenta */
#define REPRODUCAO_GADO    25   /* crescimento natural do rebanho por ano, em % */
#define IMPOSTO_NEUTRO     15   /* abaixo disso, atrai gente; acima, afugenta */

void reino_inicializar(Reino *r, const char *nome) {
    memset(r, 0, sizeof(Reino));
    strncpy(r->nome, nome, NOME_MAX - 1);
    r->ano       = 1;
    r->estacao   = EST_PRIMAVERA;
    r->populacao = 120;
    r->comida    = 240;
    r->gado      = 20;
    r->campos    = 10;
    r->ouro      = 150;
    r->imposto   = 15;
    r->clima     = 100;
    r->soldados  = 0;
    r->muralhas  = 0;
    r->fase_drakmar = 0;
    r->ameaca    = 0;
    r->tributos_pagos = 0;
}

const char *reino_nome_estacao(Estacao e) {
    switch (e) {
        case EST_PRIMAVERA: return "Primavera";
        case EST_VERAO:     return "Verão";
        case EST_OUTONO:    return "Outono";
        case EST_INVERNO:   return "Inverno";
        default:            return "Desconhecida";
    }
}

int reino_campos_trabalhaveis(const Reino *r) {
    int limite_mao = r->populacao / TRAB_POR_CAMPO;
    return r->campos < limite_mao ? r->campos : limite_mao;
}

int reino_colheita_prevista(const Reino *r) {
    return reino_campos_trabalhaveis(r) * RENDIMENTO_CAMPO * r->clima / 100;
}

int reino_comida_necessaria(const Reino *r) {
    /* o povo e os soldados consomem do celeiro no inverno */
    return (r->populacao + r->soldados) * CONSUMO_POR_PESSOA;
}

int reino_capacidade_pasto(const Reino *r) {
    return r->campos * PASTO_POR_CAMPO;
}

/* ---- processamento de cada estacao ---- */

static void processar_primavera(Reino *r) {
    int cap   = reino_capacidade_pasto(r);
    int antes = r->gado;
    int crias = r->gado * REPRODUCAO_GADO / 100;
    r->gado += crias;
    if (r->gado > cap) r->gado = cap;
    int ganho = r->gado - antes;

    ui_narrar("A primavera chega a Avalon. Os camponeses preparam a terra para o plantio "
              "e os campos voltam a ganhar vida.");
    if (ganho > 0) {
        char buf[120];
        snprintf(buf, sizeof(buf), "O rebanho cresceu: nasceram %d cabeças de gado. Total: %d.", ganho, r->gado);
        ui_msg(buf);
    } else if (r->gado >= cap && r->gado > 0) {
        ui_msg("O pasto está no limite: o rebanho não cresceu por falta de terra. Roce mais campos para criar mais gado.");
    }
}

static void processar_verao(Reino *r) {
    (void)r;
    ui_narrar("O verão amadurece as lavouras sob o sol. O reino aguarda a colheita.");
}

static void processar_outono(Reino *r) {
    int colheita = reino_colheita_prevista(r);
    r->comida += colheita;
    char buf[160];
    snprintf(buf, sizeof(buf), "Chega o outono e a colheita é recolhida: %d medidas de grãos. "
             "O celeiro agora guarda %d medidas.", colheita, r->comida);
    ui_narrar(buf);
    if (reino_campos_trabalhaveis(r) < r->campos)
        ui_msg("Faltou gente para lavrar todos os campos. Mais habitantes renderiam uma colheita maior.");
}

static void processar_inverno(Reino *r) {
    ui_narrar("O inverno cobre Avalon de frio. O povo recolhe-se e vive do que guardou no celeiro.");

    int necessario = reino_comida_necessaria(r);
    int mortes_fome = 0;
    if (r->comida >= necessario) {
        r->comida -= necessario;
        char buf[140];
        snprintf(buf, sizeof(buf), "O povo passa o inverno alimentado. Restam %d medidas no celeiro.", r->comida);
        ui_msg(buf);
    } else {
        int deficit = necessario - r->comida;
        r->comida = 0;
        mortes_fome = deficit / 2;
        if (mortes_fome < 1) mortes_fome = 1;
        if (mortes_fome > r->populacao) mortes_fome = r->populacao;
        r->populacao -= mortes_fome;
        char buf[180];
        snprintf(buf, sizeof(buf), "Fome. O celeiro não bastou para o inverno. %d habitantes morreram de fome.", mortes_fome);
        ui_narrar(buf);
    }

    /* impostos do ano */
    int receita = r->populacao * r->imposto / 100 * 2;
    r->ouro += receita;
    {
        char buf[140];
        snprintf(buf, sizeof(buf), "Os impostos do ano renderam %d moedas de ouro. Tesouro: %d.", receita, r->ouro);
        ui_msg(buf);
    }

    /* movimento da populacao: nascimentos, imigracao e emigracao */
    int nascimentos = 0, imigrantes = 0, emigrantes = 0;
    if (mortes_fome == 0) {
        nascimentos = r->populacao * 4 / 100;
        if (r->imposto < IMPOSTO_NEUTRO)
            imigrantes = r->populacao * (IMPOSTO_NEUTRO - r->imposto) * 2 / 1000;
    }
    if (r->imposto > 20)
        emigrantes = r->populacao * (r->imposto - 20) * 4 / 1000;

    int delta = nascimentos + imigrantes - emigrantes;
    r->populacao += delta;
    if (r->populacao < 0) r->populacao = 0;

    if (nascimentos || imigrantes || emigrantes) {
        char buf[200];
        snprintf(buf, sizeof(buf),
            "Movimento do povo: %d nasceram, %d chegaram em busca de uma vida melhor, "
            "%d partiram por causa dos impostos. População: %d.",
            nascimentos, imigrantes, emigrantes, r->populacao);
        ui_msg(buf);
    }
}

void reino_avancar_estacao(Reino *r) {
    switch (r->estacao) {
        case EST_PRIMAVERA: processar_primavera(r); break;
        case EST_VERAO:     processar_verao(r);     break;
        case EST_OUTONO:    processar_outono(r);    break;
        case EST_INVERNO:   processar_inverno(r);   break;
        default: break;
    }

    r->estacao = (Estacao)((r->estacao + 1) % EST_TOTAL);
    if (r->estacao == EST_PRIMAVERA) {
        r->ano++;
        r->clima = utils_rand(60, 140);   /* o clima do novo ano */
    }
}
