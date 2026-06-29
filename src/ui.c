#include "ui.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>

extern int g_audio_ativado;

static char ultima_descricao[4096]       = "";
static char ultimas_opcoes_display[2048] = "";
static char ultimas_opcoes_fala[2048]    = "";

static void falar(const char *texto) {
    if (g_audio_ativado) tts_speak(texto);
}

void ui_separador(void) {
    printf("\n");
}

void ui_titulo(const char *titulo) {
    printf("\n%s\n\n", titulo);
}

void ui_narrar(const char *texto) {
    printf("\n%s\n", texto);
    strncpy(ultima_descricao, texto, sizeof(ultima_descricao) - 1);
    falar(texto);
}

void ui_msg(const char *texto) {
    printf("%s\n", texto);
    falar(texto);
}

void ui_erro(const char *texto) {
    printf("\nErro: %s\n", texto);
    falar(texto);
}

void ui_limpar_opcoes(void) {
    ultimas_opcoes_display[0] = '\0';
    ultimas_opcoes_fala[0]    = '\0';
}

void ui_prompt_menu(const char *pergunta) {
    ui_msg(pergunta);
    ui_limpar_opcoes();
}

void ui_opcao(int num, const char *texto) {
    char display[256];
    char fala[256];
    snprintf(display, sizeof(display), "  %d - %s\n", num, texto);
    snprintf(fala,    sizeof(fala),    "Op\xc3\xa7\xc3\xa3o %d, %s. ", num, texto);
    printf("%s", display);
    strncat(ultimas_opcoes_display, display,
        sizeof(ultimas_opcoes_display) - strlen(ultimas_opcoes_display) - 1);
    strncat(ultimas_opcoes_fala, fala,
        sizeof(ultimas_opcoes_fala) - strlen(ultimas_opcoes_fala) - 1);
}

void ui_falar_opcoes(void) {
    if (g_audio_ativado && ultimas_opcoes_fala[0] != '\0')
        tts_speak(ultimas_opcoes_fala);
}

void ui_repetir(void) {
    if (ultima_descricao[0] == '\0') {
        ui_msg("Nenhuma descrição anterior para repetir.");
        return;
    }
    printf("\nRepetindo última descrição:\n%s\n", ultima_descricao);
    falar(ultima_descricao);
}

void ui_ajuda(void) {
    const char *texto =
        "Comandos disponíveis a qualquer momento: "
        "ajuda, situação, opções, repetir, salvar, áudio, sair. "
        "Você pode digitar o número ou o nome da ação. "
        "Maiúsculas, acentos e espaços extras são aceitos.";
    printf("\nAjuda - comandos disponíveis a qualquer momento:\n\n");
    printf("  ajuda     - Exibe esta lista de comandos\n");
    printf("  situacao  - Mostra o estado do reino\n");
    printf("  guia      - Explica as regras do reino\n");
    printf("  opcoes    - Repete as opções da estação atual\n");
    printf("  repetir   - Repete a última narração por voz\n");
    printf("  salvar    - Salva o progresso do jogo\n");
    printf("  audio     - Ativa ou desativa a narração por voz\n");
    printf("  sair      - Sai do jogo\n\n");
    printf("Você pode digitar o número ou o nome da ação.\n");
    printf("Maiúsculas, acentos e espaços extras são aceitos.\n\n");
    falar(texto);
}

void ui_opcoes_globais(void) {
    if (ultimas_opcoes_display[0] != '\0') {
        printf("\nOpções disponíveis:\n%s", ultimas_opcoes_display);
        if (g_audio_ativado) tts_speak(ultimas_opcoes_fala);
    } else {
        ui_msg("Use ajuda para ver os comandos disponíveis.");
    }
}
