#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <string.h>

#pragma comment(lib, "winmm.lib")

#define PIPER_EXE  "piper-windows\\piper.exe"
#define PIPER_MODEL "piper-windows\\pt_BR-faber-medium.onnx"
#define TEMP_WAV   "piper_out.wav"

void tts_speak(const char *text) {
    FILE *f = fopen("piper_in.txt", "w");
    if (!f) { fprintf(stderr, "Erro ao criar arquivo de texto\n"); return; }
    fputs(text, f);
    fclose(f);

    char cmd[8192];
    snprintf(cmd, sizeof(cmd),
        "cd piper-windows && piper.exe --model \"..\\models\\pt_BR-faber-medium.onnx\" --length_scale 0.8 --output_file \"..\\%s\" < \"..\\piper_in.txt\" >nul 2>nul",
        TEMP_WAV);

    int ret = system(cmd);
    DeleteFileA("piper_in.txt");

    if (ret != 0) {
        fprintf(stderr, "Erro ao executar piper (código %d)\n", ret);
        return;
    }

    PlaySoundA(TEMP_WAV, NULL, SND_FILENAME | SND_SYNC);
    DeleteFileA(TEMP_WAV);
}
