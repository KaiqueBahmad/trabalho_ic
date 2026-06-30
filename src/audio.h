#ifndef AUDIO_H
#define AUDIO_H

/* Quando 1, as narracoes pendentes do prompt atual sao puladas (ESC).
   A leitura de entrada zera esse sinalizador a cada novo prompt. */
extern int g_pular_narracao;

void tts_speak(const char *text);

/* Toca um beep curto e suave (models/beep.wav) ao pedir entrada do jogador. */
void audio_beep(void);

/* Velocidade da narracao: alterna normal -> rapida -> muito rapida. */
void        audio_proxima_velocidade(void);
const char *audio_velocidade_nome(void);
int         audio_velocidade_idx(void);
void        audio_velocidade_set(int idx);

#endif
