#ifndef AUDIO_H
#define AUDIO_H

/* Quando 1, as narracoes pendentes do prompt atual sao puladas (ESC).
   A leitura de entrada zera esse sinalizador a cada novo prompt. */
extern int g_pular_narracao;

void tts_speak(const char *text);

/* Velocidade da narracao: alterna normal -> rapida -> muito rapida. */
void        audio_proxima_velocidade(void);
const char *audio_velocidade_nome(void);

#endif
