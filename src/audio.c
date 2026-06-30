#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#pragma comment(lib, "winmm.lib")
#else
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#endif

#define TEMP_WAV    "piper_out.wav"
#define PIPER_MODEL "models/pt_BR-faber-medium.onnx"
#define BEEP_WAV    "models/beep.wav"

/* Sinalizador de "pular narracao". O ESC durante a fala o ativa, fazendo as
   narracoes restantes do prompt atual serem ignoradas. A leitura de entrada
   o zera a cada novo prompt (ver entrada_ler_linha). */
int g_pular_narracao = 0;

/* Velocidade da narracao. length_scale menor = fala mais rapida. */
static int g_vel = 0;
static const double escalas[]   = { 0.80, 0.60, 0.45 };
static const char  *nomes_vel[] = { "NORMAL", "RÁPIDA", "MUITO RÁPIDA" };

void audio_proxima_velocidade(void) {
    g_vel = (g_vel + 1) % 3;
}

const char *audio_velocidade_nome(void) {
    return nomes_vel[g_vel];
}

int audio_velocidade_idx(void) {
    return g_vel;
}

void audio_velocidade_set(int idx) {
    if (idx >= 0 && idx < 3) g_vel = idx;
}

/* ================================================================
   REPRODUCAO INTERROMPIVEL
   Toca o audio e, enquanto ele toca, vigia o teclado. Se o jogador
   apertar ESC, a narracao atual e interrompida e o jogo segue.
   ================================================================ */

#ifdef _WIN32
static void play_wav(const char *path) {
    char cmd[600], ret[64];
    snprintf(cmd, sizeof(cmd), "open \"%s\" type waveaudio alias coroasnd", path);
    if (mciSendStringA(cmd, NULL, 0, NULL) != 0) {
        /* sem MCI, cai para reproducao simples (nao interrompivel) */
        PlaySoundA(path, NULL, SND_FILENAME | SND_SYNC);
        return;
    }
    mciSendStringA("play coroasnd", NULL, 0, NULL);
    for (;;) {
        ret[0] = '\0';
        mciSendStringA("status coroasnd mode", ret, sizeof(ret), NULL);
        if (strncmp(ret, "playing", 7) != 0) break;   /* terminou */
        if (_kbhit()) {
            int c = _getch();
            if (c == 27) {                              /* ESC: pular o resto */
                g_pular_narracao = 1;
                mciSendStringA("stop coroasnd", NULL, 0, NULL);
                break;
            } else if (c == 0 || c == 0xE0) {
                _getch();                               /* descarta tecla especial */
            }
            /* outras teclas durante a fala sao ignoradas */
        }
        Sleep(30);
    }
    mciSendStringA("close coroasnd", NULL, 0, NULL);
    /* limpa o que tenha sido digitado durante a fala, evitando dessincronia */
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}
#else
static void montar_comando(char *cmd, size_t n, const char *path) {
#if defined(__APPLE__)
    snprintf(cmd, n, "afplay '%s'", path);
#else
    snprintf(cmd, n, "aplay '%s' 2>/dev/null || paplay '%s' 2>/dev/null", path, path);
#endif
}

/* Modo cru do terminal: mantido ligado durante toda a fala (sintese + audio),
   para que o ESC seja capturado tambem enquanto o piper ainda esta gerando o
   audio (a etapa mais lenta, principalmente na primeira fala). */
static struct termios g_termios_velho;
static int g_raw_ativo = 0;

static void raw_on(void) {
    if (g_raw_ativo) return;
    if (!isatty(STDIN_FILENO)) return;          /* entrada redirecionada (pipe) */
    if (tcgetattr(STDIN_FILENO, &g_termios_velho) != 0) return;
    struct termios cru = g_termios_velho;
    cru.c_lflag &= ~(ICANON | ECHO);
    cru.c_cc[VMIN]  = 0;
    cru.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &cru);
    g_raw_ativo = 1;
}

static void raw_off(void) {
    if (!g_raw_ativo) return;
    tcflush(STDIN_FILENO, TCIFLUSH);            /* descarta teclas digitadas durante a fala */
    tcsetattr(STDIN_FILENO, TCSANOW, &g_termios_velho);
    g_raw_ativo = 0;
}

/* Executa um comando em sub-processo, vigiando o teclado. Se o ESC for
   pressionado, mata o processo e retorna 1. Sem terminal interativo, apenas
   espera o comando terminar. */
static int rodar_vigiando_esc(const char *cmd) {
    pid_t pid = fork();
    if (pid < 0) { system(cmd); return 0; }
    if (pid == 0) {
        setpgid(0, 0);                           /* grupo proprio: shell + player juntos */
        freopen("/dev/null", "w", stderr);
        freopen("/dev/null", "w", stdout);
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }
    setpgid(pid, pid);                           /* idem no pai, evitando a corrida */

    if (!g_raw_ativo) {                          /* nao interativo: so aguarda */
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }

    for (;;) {
        int status;
        if (waitpid(pid, &status, WNOHANG) == pid) return 0;   /* terminou */

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv = {0, 30000};                        /* 30 ms */
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            unsigned char c;
            while (read(STDIN_FILENO, &c, 1) == 1) {            /* drena a sequencia */
                if (c == 27) {                                 /* ESC: abortar */
                    kill(-pid, SIGKILL);                       /* mata o grupo inteiro */
                    waitpid(pid, &status, 0);
                    return 1;
                }
            }
        }
    }
}
#endif

/* Toca o beep prefabricado (models/beep.wav) para sinalizar a vez de digitar.
   Curto e nao interrompivel; nao mexe no estado de narracao. */
void audio_beep(void) {
#ifdef _WIN32
    /* assincrono: nao trava o prompt e e descartado se algo novo tocar */
    PlaySoundA(BEEP_WAV, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
#else
    char cmd[512];
    montar_comando(cmd, sizeof(cmd), BEEP_WAV);
    system(cmd);
#endif
}

void tts_speak(const char *text) {
    /* se o jogador pediu para pular, ignora as narracoes restantes do prompt */
    if (g_pular_narracao) return;

    FILE *f = fopen("piper_in.txt", "w");
    if (!f) { fprintf(stderr, "Erro ao criar arquivo de texto\n"); return; }
    fputs(text, f);
    fclose(f);

    char cmd[8192];
    double escala = escalas[g_vel];

#ifdef _WIN32
    snprintf(cmd, sizeof(cmd),
        "cd piper && piper.exe --model \"..\\%s\" --length_scale %.2f"
        " --output_file \"..\\%s\" < \"..\\piper_in.txt\" >nul 2>nul",
        PIPER_MODEL, escala, TEMP_WAV);
#else
    snprintf(cmd, sizeof(cmd),
        "cd piper && ./piper --model '../%s' --length_scale %.2f"
        " --output_file '../%s' < '../piper_in.txt' >/dev/null 2>&1",
        PIPER_MODEL, escala, TEMP_WAV);
#endif

#ifdef _WIN32
    int ret = system(cmd);
    remove("piper_in.txt");
    if (ret != 0) {
        fprintf(stderr, "Erro ao executar piper\n");
        return;
    }
    play_wav(TEMP_WAV);
    remove(TEMP_WAV);
#else
    /* Vigia o ESC durante a sintese E durante a reproducao. */
    raw_on();
    int abortou = rodar_vigiando_esc(cmd);       /* piper gera o WAV */
    remove("piper_in.txt");
    if (abortou) { g_pular_narracao = 1; raw_off(); return; }

    char play_cmd[512];
    montar_comando(play_cmd, sizeof(play_cmd), TEMP_WAV);
    if (rodar_vigiando_esc(play_cmd))            /* toca o WAV */
        g_pular_narracao = 1;
    remove(TEMP_WAV);
    raw_off();
#endif
}
