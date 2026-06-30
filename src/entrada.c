#include "entrada.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* Beep curto e suave para sinalizar que e a vez do jogador digitar.
   Toca o arquivo prefabricado models/beep.wav (ver audio.c). */
static void beep_suave(void) {
    extern int g_audio_ativado;
    if (g_audio_ativado) audio_beep();
}

static void remover_acentos(char *s) {
    unsigned char *p = (unsigned char *)s;
    unsigned char *w = p;
    while (*p) {
        unsigned char c = *p;
        if (c == 0xC3 && *(p + 1)) {
            unsigned char n = *(p + 1);
            char sub = 0;
            switch (n) {
                case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5:
                case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85:
                    sub = 'a'; break;
                case 0xA7: case 0x87: sub = 'c'; break;
                case 0xA8: case 0xA9: case 0xAA: case 0xAB:
                case 0x88: case 0x89: case 0x8A: case 0x8B:
                    sub = 'e'; break;
                case 0xAC: case 0xAD: case 0xAE: case 0xAF:
                case 0x8C: case 0x8D: case 0x8E: case 0x8F:
                    sub = 'i'; break;
                case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6:
                case 0x92: case 0x93: case 0x94: case 0x95: case 0x96:
                    sub = 'o'; break;
                case 0xB9: case 0xBA: case 0xBB: case 0xBC:
                case 0x99: case 0x9A: case 0x9B: case 0x9C:
                    sub = 'u'; break;
            }
            if (sub) { *w++ = sub; p += 2; continue; }
        }
        *w++ = *p++;
    }
    *w = '\0';
}

int entrada_ler_linha(char *buf, int size) {
    /* novo prompt: as narracoes seguintes podem voltar a tocar */
    g_pular_narracao = 0;
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return 0; }
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return 1;
}

void entrada_ler(char *buf, int size) {
    beep_suave();
    printf("> ");
    fflush(stdout);
    if (!entrada_ler_linha(buf, size)) {
        if (feof(stdin)) exit(0);
    }
}

void entrada_normalizar(char *buf) {
    /* trim leading */
    int start = 0;
    while (buf[start] == ' ' || buf[start] == '\t') start++;
    if (start) memmove(buf, buf + start, strlen(buf) - start + 1);
    /* trim trailing */
    int len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\t')) buf[--len] = '\0';
    /* collapse spaces */
    char tmp[CMD_MAX];
    int j = 0, sp = 0;
    for (int i = 0; buf[i]; i++) {
        if (buf[i] == ' ') { if (!sp && j > 0) { tmp[j++] = ' '; sp = 1; } }
        else { tmp[j++] = buf[i]; sp = 0; }
    }
    tmp[j] = '\0';
    strcpy(buf, tmp);
    /* lowercase */
    for (int i = 0; buf[i]; i++) buf[i] = (char)tolower((unsigned char)buf[i]);
    /* remove acentos */
    remover_acentos(buf);
}

int entrada_eh_numero(const char *buf) {
    if (!*buf) return 0;
    for (int i = 0; buf[i]; i++)
        if (!isdigit((unsigned char)buf[i])) return 0;
    return 1;
}

int entrada_to_numero(const char *buf) {
    return atoi(buf);
}

int entrada_confirmar(const char *pergunta) {
    extern int g_audio_ativado;
    char buf[CMD_MAX];
    printf("\n%s\n", pergunta);
    printf("  1 - Sim\n");
    printf("  2 - Não\n");
    if (g_audio_ativado) {
        char completo[CMD_MAX + 32];
        snprintf(completo, sizeof(completo), "%s. Um para sim, dois para não.", pergunta);
        tts_speak(completo);
    }
    entrada_ler(buf, CMD_MAX);
    entrada_normalizar(buf);
    return (strcmp(buf, "1") == 0 || strcmp(buf, "sim") == 0 || strcmp(buf, "s") == 0);
}
