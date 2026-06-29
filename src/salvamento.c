#include "salvamento.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <sys/stat.h>

extern int g_audio_ativado;

#define SAVE_FILE   "saves/save.txt"
#define CONFIG_FILE "saves/config.txt"

static void garantir_dir_saves(void) {
#ifdef _WIN32
    _mkdir("saves");
#else
    mkdir("saves", 0755);
#endif
}

int salvar_reino(const Reino *r) {
    garantir_dir_saves();
    FILE *f = fopen(SAVE_FILE, "w");
    if (!f) { printf("Erro ao abrir o arquivo de salvamento.\n"); return 0; }

    fprintf(f, "NOME=%s\n",          r->nome);
    fprintf(f, "ANO=%d\n",           r->ano);
    fprintf(f, "ESTACAO=%d\n",       (int)r->estacao);
    fprintf(f, "POPULACAO=%d\n",     r->populacao);
    fprintf(f, "COMIDA=%d\n",        r->comida);
    fprintf(f, "GADO=%d\n",          r->gado);
    fprintf(f, "CAMPOS=%d\n",        r->campos);
    fprintf(f, "OURO=%d\n",          r->ouro);
    fprintf(f, "IMPOSTO=%d\n",       r->imposto);
    fprintf(f, "CLIMA=%d\n",         r->clima);
    fprintf(f, "SOLDADOS=%d\n",      r->soldados);
    fprintf(f, "MURALHAS=%d\n",      r->muralhas);
    fprintf(f, "FASE_DRAKMAR=%d\n",  r->fase_drakmar);
    fprintf(f, "AMEACA=%d\n",        r->ameaca);
    fprintf(f, "TRIBUTOS_PAGOS=%d\n",r->tributos_pagos);
    fprintf(f, "AUDIO=%d\n",         g_audio_ativado);

    fclose(f);
    printf("Reino salvo em '%s'.\n", SAVE_FILE);
    return 1;
}

int carregar_reino(Reino *r) {
    FILE *f = fopen(SAVE_FILE, "r");
    if (!f) {
        printf("Nenhum reino salvo foi encontrado.\n");
        return 0;
    }
    memset(r, 0, sizeof(Reino));

    char linha[256];
    while (fgets(linha, sizeof(linha), f)) {
        char chave[64], valor[192];
        if (sscanf(linha, "%63[^=]=%191[^\n]", chave, valor) != 2) continue;

        if      (!strcmp(chave, "NOME"))         strncpy(r->nome, valor, NOME_MAX - 1);
        else if (!strcmp(chave, "ANO"))          r->ano          = atoi(valor);
        else if (!strcmp(chave, "ESTACAO"))      r->estacao      = (Estacao)atoi(valor);
        else if (!strcmp(chave, "POPULACAO"))    r->populacao    = atoi(valor);
        else if (!strcmp(chave, "COMIDA"))       r->comida       = atoi(valor);
        else if (!strcmp(chave, "GADO"))         r->gado         = atoi(valor);
        else if (!strcmp(chave, "CAMPOS"))       r->campos       = atoi(valor);
        else if (!strcmp(chave, "OURO"))         r->ouro         = atoi(valor);
        else if (!strcmp(chave, "IMPOSTO"))      r->imposto      = atoi(valor);
        else if (!strcmp(chave, "CLIMA"))        r->clima        = atoi(valor);
        else if (!strcmp(chave, "SOLDADOS"))     r->soldados     = atoi(valor);
        else if (!strcmp(chave, "MURALHAS"))     r->muralhas     = atoi(valor);
        else if (!strcmp(chave, "FASE_DRAKMAR")) r->fase_drakmar = atoi(valor);
        else if (!strcmp(chave, "AMEACA"))       r->ameaca       = atoi(valor);
        else if (!strcmp(chave, "TRIBUTOS_PAGOS")) r->tributos_pagos = atoi(valor);
        else if (!strcmp(chave, "AUDIO"))        g_audio_ativado = atoi(valor);
    }
    fclose(f);
    if (r->ano <= 0 || r->populacao <= 0) {
        printf("O arquivo de salvamento está vazio ou em formato antigo. Não foi possível carregar.\n");
        return 0;
    }
    printf("Reino carregado com sucesso.\n");
    return 1;
}

void salvar_config(void) {
    garantir_dir_saves();
    FILE *f = fopen(CONFIG_FILE, "w");
    if (!f) return;
    fprintf(f, "VELOCIDADE=%d\n", audio_velocidade_idx());
    fclose(f);
}

void carregar_config(void) {
    FILE *f = fopen(CONFIG_FILE, "r");
    if (!f) return;
    char linha[128];
    while (fgets(linha, sizeof(linha), f)) {
        char chave[64], valor[64];
        if (sscanf(linha, "%63[^=]=%63[^\n]", chave, valor) != 2) continue;
        if (!strcmp(chave, "VELOCIDADE")) audio_velocidade_set(atoi(valor));
    }
    fclose(f);
}
