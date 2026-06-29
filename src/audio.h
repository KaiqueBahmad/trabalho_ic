#ifndef AUDIO_H
#define AUDIO_H

/* Quando 1, as narracoes pendentes do prompt atual sao puladas (ESC).
   A leitura de entrada zera esse sinalizador a cada novo prompt. */
extern int g_pular_narracao;

void tts_speak(const char *text);

#endif
