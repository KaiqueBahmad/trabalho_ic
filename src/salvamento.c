#include "salvamento.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>

extern int g_audio_ativado;

#define SAVE_FILE "saves/save.txt"

int salvar_jogo(const Jogador *j, const Inventario *inv) {
    _mkdir("saves");
    FILE *f = fopen(SAVE_FILE, "w");
    if (!f) { printf("Erro ao abrir arquivo de salvamento.\n"); return 0; }

    fprintf(f, "NOME=%s\n",                    j->nome);
    fprintf(f, "VIDA=%d\n",                    j->vida);
    fprintf(f, "OURO=%d\n",                    j->ouro);
    fprintf(f, "EXERCITO=%d\n",                j->exercito);
    fprintf(f, "POPULARIDADE=%d\n",            j->popularidade);
    fprintf(f, "SABEDORIA=%d\n",               j->sabedoria);
    fprintf(f, "LOCAL=%d\n",                   (int)j->local_atual);
    fprintf(f, "TURNO=%d\n",                   j->turno);
    fprintf(f, "EMISSARIO_RECEBIDO=%d\n",      j->emissario_recebido);
    fprintf(f, "PRISIONEIRO_LIBERTADO=%d\n",   j->prisioneiro_libertado);
    fprintf(f, "ALIANCA_FORMADA=%d\n",         j->alianca_formada);
    fprintf(f, "EXERCITO_REFORCADO=%d\n",      j->exercito_reforcado);
    fprintf(f, "POVO_AJUDADO=%d\n",            j->povo_ajudado);
    fprintf(f, "TRAIDOR_REVELADO=%d\n",        j->traidor_revelado);
    fprintf(f, "BATALHA_TRAVADA=%d\n",         j->batalha_travada);
    fprintf(f, "MISSAO_POVO_INICIADA=%d\n",    j->missao_povo_iniciada);
    fprintf(f, "CONSELHEIRO_PRESO=%d\n",       j->conselheiro_preso);
    fprintf(f, "ERIK_FALOU_ALDRIC=%d\n",       j->erik_falou_aldric);
    fprintf(f, "INFO_REINO_VEIRA=%d\n",        j->info_reino_veira);
    fprintf(f, "AUDIO=%d\n",                   g_audio_ativado);
    fprintf(f, "NUM_ITENS=%d\n",               inv->quantidade);
    for (int i = 0; i < inv->quantidade; i++)
        fprintf(f, "ITEM=%d\n", (int)inv->itens[i].tipo);

    fclose(f);
    printf("Jogo salvo em '%s'.\n", SAVE_FILE);
    return 1;
}

int carregar_jogo(Jogador *j, Inventario *inv) {
    FILE *f = fopen(SAVE_FILE, "r");
    if (!f) {
        printf("Nenhum arquivo de salvamento encontrado.\n");
        return 0;
    }
    memset(j, 0, sizeof(Jogador));
    inventario_inicializar(inv);

    char linha[256];
    while (fgets(linha, sizeof(linha), f)) {
        char chave[64], valor[192];
        if (sscanf(linha, "%63[^=]=%191[^\n]", chave, valor) != 2) continue;

        if      (!strcmp(chave, "NOME"))                 strncpy(j->nome, valor, NOME_MAX-1);
        else if (!strcmp(chave, "VIDA"))                 j->vida                 = atoi(valor);
        else if (!strcmp(chave, "OURO"))                 j->ouro                 = atoi(valor);
        else if (!strcmp(chave, "EXERCITO"))             j->exercito             = atoi(valor);
        else if (!strcmp(chave, "POPULARIDADE"))         j->popularidade         = atoi(valor);
        else if (!strcmp(chave, "SABEDORIA"))            j->sabedoria            = atoi(valor);
        else if (!strcmp(chave, "LOCAL"))                j->local_atual          = atoi(valor);
        else if (!strcmp(chave, "TURNO"))                j->turno                = atoi(valor);
        else if (!strcmp(chave, "EMISSARIO_RECEBIDO"))   j->emissario_recebido   = atoi(valor);
        else if (!strcmp(chave, "PRISIONEIRO_LIBERTADO"))j->prisioneiro_libertado= atoi(valor);
        else if (!strcmp(chave, "ALIANCA_FORMADA"))      j->alianca_formada      = atoi(valor);
        else if (!strcmp(chave, "EXERCITO_REFORCADO"))   j->exercito_reforcado   = atoi(valor);
        else if (!strcmp(chave, "POVO_AJUDADO"))         j->povo_ajudado         = atoi(valor);
        else if (!strcmp(chave, "TRAIDOR_REVELADO"))     j->traidor_revelado     = atoi(valor);
        else if (!strcmp(chave, "BATALHA_TRAVADA"))      j->batalha_travada      = atoi(valor);
        else if (!strcmp(chave, "MISSAO_POVO_INICIADA")) j->missao_povo_iniciada = atoi(valor);
        else if (!strcmp(chave, "CONSELHEIRO_PRESO"))    j->conselheiro_preso    = atoi(valor);
        else if (!strcmp(chave, "ERIK_FALOU_ALDRIC"))    j->erik_falou_aldric    = atoi(valor);
        else if (!strcmp(chave, "INFO_REINO_VEIRA"))     j->info_reino_veira     = atoi(valor);
        else if (!strcmp(chave, "AUDIO"))                g_audio_ativado         = atoi(valor);
        else if (!strcmp(chave, "ITEM"))                 inventario_adicionar(inv, (TipoItem)atoi(valor));
    }
    fclose(f);
    printf("Jogo carregado com sucesso.\n");
    return 1;
}
