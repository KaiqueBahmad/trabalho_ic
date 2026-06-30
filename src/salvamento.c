#include "salvamento.h"
#include "mundo.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <sys/stat.h>

extern int   g_audio_ativado;
extern Mundo g_mundo;

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
    fprintf(f, "MURALHA_NIVEL=%d\n", r->muralha_nivel);
    fprintf(f, "MURALHA_VIDA=%d\n",  r->muralha_vida);
    fprintf(f, "MINAS=%d\n",         r->minas);
    fprintf(f, "AUDIO=%d\n",         g_audio_ativado);

    /* mundo: um reino rival por linha, depois o jornal */
    for (int i = 0; i < NUM_NPCS; i++) {
        const ReinoNPC *n = &g_mundo.npc[i];
        fprintf(f, "NPC=%d;%d;%d;%d;%d;%d;%d;%d;%d;%s\n",
                n->ativo, n->populacao, n->soldados, n->muralha_nivel, n->muralha_vida,
                n->ouro, n->comida, n->gado, n->minas, n->nome);
    }
    for (int i = 0; i < g_mundo.jornal_n; i++)
        fprintf(f, "JORNAL=%s\n", g_mundo.jornal[i]);

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
    mundo_inicializar(&g_mundo);   /* base; sobrescrita pelos dados salvos */
    g_mundo.jornal_n = 0;
    int npc_idx = 0;

    char linha[256];
    while (fgets(linha, sizeof(linha), f)) {
        char chave[64], valor[192];
        if (sscanf(linha, "%63[^=]=%191[^\n]", chave, valor) != 2) continue;

        if (!strcmp(chave, "NPC")) {
            if (npc_idx < NUM_NPCS) {
                ReinoNPC *n = &g_mundo.npc[npc_idx];
                char nome[NOME_MAX] = "";
                sscanf(valor, "%d;%d;%d;%d;%d;%d;%d;%d;%d;%63[^\n]",
                       &n->ativo, &n->populacao, &n->soldados, &n->muralha_nivel, &n->muralha_vida,
                       &n->ouro, &n->comida, &n->gado, &n->minas, nome);
                if (nome[0]) strncpy(n->nome, nome, NOME_MAX - 1);
                npc_idx++;
            }
            continue;
        }
        if (!strcmp(chave, "JORNAL")) { mundo_jornal_add(&g_mundo, valor); continue; }

        if      (!strcmp(chave, "NOME"))          strncpy(r->nome, valor, NOME_MAX - 1);
        else if (!strcmp(chave, "ANO"))           r->ano           = atoi(valor);
        else if (!strcmp(chave, "ESTACAO"))       r->estacao       = (Estacao)atoi(valor);
        else if (!strcmp(chave, "POPULACAO"))     r->populacao     = atoi(valor);
        else if (!strcmp(chave, "COMIDA"))        r->comida        = atoi(valor);
        else if (!strcmp(chave, "GADO"))          r->gado          = atoi(valor);
        else if (!strcmp(chave, "CAMPOS"))        r->campos        = atoi(valor);
        else if (!strcmp(chave, "OURO"))          r->ouro          = atoi(valor);
        else if (!strcmp(chave, "IMPOSTO"))       r->imposto       = atoi(valor);
        else if (!strcmp(chave, "CLIMA"))         r->clima         = atoi(valor);
        else if (!strcmp(chave, "SOLDADOS"))      r->soldados      = atoi(valor);
        else if (!strcmp(chave, "MURALHA_NIVEL")) r->muralha_nivel = atoi(valor);
        else if (!strcmp(chave, "MURALHA_VIDA"))  r->muralha_vida  = atoi(valor);
        else if (!strcmp(chave, "MINAS"))         r->minas         = atoi(valor);
        else if (!strcmp(chave, "AUDIO"))         g_audio_ativado  = atoi(valor);
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
