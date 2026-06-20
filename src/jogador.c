#include "jogador.h"
#include "../audio/audio.h"
#include <stdio.h>
#include <string.h>

extern int g_audio_ativado;

static int clamp(int v, int mn, int mx) {
    return v < mn ? mn : v > mx ? mx : v;
}

void jogador_inicializar(Jogador *j, const char *nome) {
    memset(j, 0, sizeof(Jogador));
    strncpy(j->nome, nome, NOME_MAX - 1);
    j->vida         = 100;
    j->ouro         = 200;
    j->exercito     = 40;
    j->popularidade = 50;
    j->sabedoria    = 20;
    j->local_atual  = LOCAL_SALA_TRONO;
    j->turno        = 1;
}

void jogador_alterar_vida(Jogador *j, int d)        { j->vida         = clamp(j->vida         + d, 0, 100); }
void jogador_alterar_exercito(Jogador *j, int d)    { j->exercito     = clamp(j->exercito     + d, 0, 100); }
void jogador_alterar_popularidade(Jogador *j, int d){ j->popularidade = clamp(j->popularidade + d, 0, 100); }
void jogador_alterar_sabedoria(Jogador *j, int d)   { j->sabedoria    = clamp(j->sabedoria    + d, 0, 100); }
void jogador_alterar_ouro(Jogador *j, int d)        { j->ouro += d; if (j->ouro < 0) j->ouro = 0; }

void jogador_mostrar_status(const Jogador *j) {
    static const char *locais[] = {
        "Salão do Trono","Masmorra","Mercado",
        "Muralhas","Biblioteca","Jardins","Vila"
    };
    printf("\nSTATUS DO REI %s\n", j->nome);
    printf("  Vida:         %d de 100\n", j->vida);
    printf("  Ouro:         %d moedas\n",  j->ouro);
    printf("  Exercito:     %d de 100\n", j->exercito);
    printf("  Popularidade: %d de 100\n", j->popularidade);
    printf("  Sabedoria:    %d de 100\n", j->sabedoria);
    printf("  Local atual:  %s\n",
        (j->local_atual >= 0 && j->local_atual < LOCAL_TOTAL)
        ? locais[j->local_atual] : "Desconhecido");
    printf("  Turno:        %d\n\n", j->turno);
    if (g_audio_ativado) {
        char resumo[512];
        snprintf(resumo, sizeof(resumo),
            "Status do Rei %s. Vida: %d de cem. Ouro: %d moedas. "
            "Exército: %d de cem. Popularidade: %d de cem. Sabedoria: %d de cem.",
            j->nome, j->vida, j->ouro,
            j->exercito, j->popularidade, j->sabedoria);
        tts_speak(resumo);
    }
}
