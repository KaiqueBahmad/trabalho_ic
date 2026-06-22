#include "inventario.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>

extern int g_audio_ativado;

static const char *nomes[ITEM_TOTAL] = {
    "Espada Real",
    "Pergaminho Antigo",
    "Chave da Masmorra",
    "Anel de Aliança",
    "Bolsa de Ouro Extra"
};

static const char *descricoes[ITEM_TOTAL] = {
    "A espada cerimonial do reino de Avalon. Sua lâmina afiada simboliza o poder e a justiça do rei. Em batalha, inspira coragem nos soldados e aumenta a eficácia do exército.",
    "Um pergaminho desbotado com táticas de negociação usadas por reis antigos de Avalon. A leitura confere grande sabedoria diplomática.",
    "Uma chave velha e enferrujada encontrada na biblioteca real. Parece se encaixar na fechadura de uma das celas da masmorra.",
    "Um anel dourado com o brasão do Reino de Veira. Simboliza um pacto de paz, cooperação e apoio mútuo entre os dois reinos.",
    "Uma bolsa de couro pesada, repleta de moedas de ouro do tesouro real."
};

const char *item_get_nome(TipoItem tipo) {
    if (tipo < 0 || tipo >= ITEM_TOTAL) return "Item desconhecido";
    return nomes[tipo];
}

const char *item_get_descricao(TipoItem tipo) {
    if (tipo < 0 || tipo >= ITEM_TOTAL) return "Sem descrição disponível.";
    return descricoes[tipo];
}

void inventario_inicializar(Inventario *inv) {
    memset(inv, 0, sizeof(Inventario));
    inv->quantidade = 0;
}

int inventario_adicionar(Inventario *inv, TipoItem tipo) {
    if (inv->quantidade >= MAX_ITENS) {
        printf("Inventário cheio! Não é possível carregar mais itens.\n");
        return 0;
    }
    if (inventario_tem(inv, tipo)) return 0;
    Item *it = &inv->itens[inv->quantidade++];
    it->tipo = tipo;
    strncpy(it->nome,      item_get_nome(tipo),      NOME_MAX - 1);
    strncpy(it->descricao, item_get_descricao(tipo),  DESC_MAX - 1);
    printf("Item obtido: %s\n", it->nome);
    return 1;
}

int inventario_remover(Inventario *inv, TipoItem tipo) {
    for (int i = 0; i < inv->quantidade; i++) {
        if (inv->itens[i].tipo == tipo) {
            for (int j = i; j < inv->quantidade - 1; j++)
                inv->itens[j] = inv->itens[j + 1];
            inv->quantidade--;
            return 1;
        }
    }
    return 0;
}

int inventario_tem(const Inventario *inv, TipoItem tipo) {
    for (int i = 0; i < inv->quantidade; i++)
        if (inv->itens[i].tipo == tipo) return 1;
    return 0;
}

void inventario_mostrar(const Inventario *inv) {
    printf("\nINVENTARIO %d de %d itens:\n", inv->quantidade, MAX_ITENS);
    if (inv->quantidade == 0) {
        printf("  vazio\n");
    } else {
        for (int i = 0; i < inv->quantidade; i++)
            printf("  %d. %s\n", i + 1, inv->itens[i].nome);
    }
    printf("  Use examinar para ver a descricao dos itens.\n\n");
    if (g_audio_ativado) {
        char resumo[512] = "Inventário: ";
        if (inv->quantidade == 0) {
            strncat(resumo, "vazio.", sizeof(resumo) - strlen(resumo) - 1);
        } else {
            for (int i = 0; i < inv->quantidade; i++) {
                strncat(resumo, inv->itens[i].nome, sizeof(resumo) - strlen(resumo) - 1);
                if (i < inv->quantidade - 1)
                    strncat(resumo, ", ", sizeof(resumo) - strlen(resumo) - 1);
                else
                    strncat(resumo, ".", sizeof(resumo) - strlen(resumo) - 1);
            }
        }
        tts_speak(resumo);
    }
}

void inventario_examinar_todos(const Inventario *inv) {
    if (inv->quantidade == 0) {
        printf("Seu inventário está vazio.\n");
        return;
    }
    printf("\nExaminando inventario:\n");
    for (int i = 0; i < inv->quantidade; i++) {
        printf("\n%d. %s\n", i + 1, inv->itens[i].nome);
        printf("   %s\n", inv->itens[i].descricao);
    }
    printf("\n");
}
