#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

#define TEMP_WAV    "piper_out.wav"
#define PIPER_MODEL "models/pt_BR-faber-medium.onnx"

static void play_wav(const char *path) {
#ifdef _WIN32
    PlaySoundA(path, NULL, SND_FILENAME | SND_SYNC);
#elif defined(__APPLE__)
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "afplay '%s' 2>/dev/null", path);
    system(cmd);
#else
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "aplay '%s' 2>/dev/null || paplay '%s' 2>/dev/null", path, path);
    system(cmd);
#endif
}

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
