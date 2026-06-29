#include "audio.h"
#include "entrada.h"
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
            if (c == 27) {                              /* ESC: pular */
                mciSendStringA("stop coroasnd", NULL, 0, NULL);
                break;
            } else if (c == 0 || c == 0xE0) {
                _getch();                               /* descarta tecla especial */
            } else {
                if (c == '\r') c = '\n';
                entrada_pushback_byte((char)c);         /* preserva type-ahead */
            }
        }
        Sleep(30);
    }
    mciSendStringA("close coroasnd", NULL, 0, NULL);
}
#else
static void montar_comando(char *cmd, size_t n, const char *path) {
#if defined(__APPLE__)
    snprintf(cmd, n, "afplay '%s'", path);
#else
    snprintf(cmd, n, "aplay '%s' 2>/dev/null || paplay '%s' 2>/dev/null", path, path);
#endif
}

static void play_wav(const char *path) {
    pid_t pid = fork();
    if (pid < 0) return;
    if (pid == 0) {
        /* filho: silencia saidas e reproduz */
        freopen("/dev/null", "w", stderr);
        freopen("/dev/null", "w", stdout);
        char cmd[512];
        montar_comando(cmd, sizeof(cmd), path);
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }

    /* So vigiamos o teclado quando a entrada e um terminal real. Com a
       entrada redirecionada (pipe), apenas esperamos o audio terminar, para
       nao consumir os comandos que estao por vir. */
    struct termios velho, cru;
    int interativo = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &velho) == 0;
    if (!interativo) {
        int status;
        waitpid(pid, &status, 0);
        return;
    }

    cru = velho;
    cru.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &cru);

    for (;;) {
        int status;
        if (waitpid(pid, &status, WNOHANG) == pid) break;   /* terminou */

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv = {0, 30000};                     /* 30 ms */
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            unsigned char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                if (c == 27) {                           /* ESC: pular */
                    kill(pid, SIGTERM);
                    waitpid(pid, &status, 0);
                    break;
                }
                entrada_pushback_byte((char)c);          /* preserva type-ahead */
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &velho);
}
#endif

void tts_speak(const char *text) {
    FILE *f = fopen("piper_in.txt", "w");
    if (!f) { fprintf(stderr, "Erro ao criar arquivo de texto\n"); return; }
    fputs(text, f);
    fclose(f);

    char cmd[8192];

#ifdef _WIN32
    snprintf(cmd, sizeof(cmd),
        "cd piper && piper.exe --model \"..\\%s\" --length_scale 0.8"
        " --output_file \"..\\%s\" < \"..\\piper_in.txt\" >nul 2>nul",
        PIPER_MODEL, TEMP_WAV);
#else
    snprintf(cmd, sizeof(cmd),
        "cd piper && ./piper --model '../%s' --length_scale 0.8"
        " --output_file '../%s' < '../piper_in.txt' >/dev/null 2>&1",
        PIPER_MODEL, TEMP_WAV);
#endif

    int ret = system(cmd);
    remove("piper_in.txt");

    if (ret != 0) {
        fprintf(stderr, "Erro ao executar piper\n");
        return;
    }

    play_wav(TEMP_WAV);
    remove(TEMP_WAV);
}
