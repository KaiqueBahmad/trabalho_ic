#include "eventos.h"
#include "ui.h"
#include "entrada.h"
#include "utils.h"
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>

extern int g_audio_ativado;

/* Le uma quantidade numerica simples (0 se invalida). */
static int ler_quantidade(const char *pergunta) {
    char buf[CMD_MAX];
    printf("%s ", pergunta);
    if (g_audio_ativado) tts_speak(pergunta);
    fflush(stdout);
    entrada_ler(buf, CMD_MAX);
    entrada_normalizar(buf);
    if (!entrada_eh_numero(buf)) return 0;
    return atoi(buf);
}

/* Le uma escolha numerica de 1 a max, reapresentando ate ser valida. */
static int ler_escolha(int max) {
    char cmd[CMD_MAX];
    for (;;) {
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);
        if (entrada_eh_numero(cmd)) {
            int n = atoi(cmd);
            if (n >= 1 && n <= max) return n;
        }
        char buf[80];
        snprintf(buf, sizeof(buf), "Escolha um número de 1 a %d.", max);
        ui_msg(buf);
    }
}

/* ---- Peste no gado ---- */
static void ev_peste_gado(Reino *r) {
    if (r->gado < 6) return;  /* sem rebanho relevante, sem evento */
    ui_titulo("Peste no Rebanho");
    ui_narrar("Uma doença se espalha entre o gado de Avalon. Os vaqueiros pedem ordens "
              "antes que o rebanho inteiro seja contaminado.");
    ui_prompt_menu("O que o rei decide?");
    ui_opcao(1, "Contratar curandeiros e isolar os doentes (custa 25 de ouro)");
    ui_opcao(2, "Abater os infectados depressa (perde menos gado)");
    ui_opcao(3, "Não fazer nada e esperar");
    ui_falar_opcoes();
    char buf[160];
    switch (ler_escolha(3)) {
        case 1:
            if (r->ouro >= 25) {
                r->ouro -= 25;
                int perda = r->gado * 8 / 100;
                r->gado -= perda;
                snprintf(buf, sizeof(buf), "Os curandeiros contêm a doença. Perdeu apenas %d cabeças. Rebanho: %d.", perda, r->gado);
            } else {
                int perda = r->gado * 30 / 100;
                r->gado -= perda;
                snprintf(buf, sizeof(buf), "Sem ouro para curandeiros, a peste avança. Perdeu %d cabeças. Rebanho: %d.", perda, r->gado);
            }
            ui_narrar(buf);
            break;
        case 2: {
            int perda = r->gado * 18 / 100;
            int ganho = perda * 4;       /* carne aproveitada */
            r->gado   -= perda;
            r->comida += ganho;
            snprintf(buf, sizeof(buf), "O abate rápido contém o surto. Perdeu %d cabeças, mas rendeu %d medidas de carne. Rebanho: %d.", perda, ganho, r->gado);
            ui_narrar(buf);
            break;
        }
        default: {
            int perda = r->gado * 35 / 100;
            r->gado -= perda;
            snprintf(buf, sizeof(buf), "A peste corre solta pelo pasto. Perdeu %d cabeças. Rebanho: %d.", perda, r->gado);
            ui_narrar(buf);
        }
    }
}

/* ---- Refugiados ---- */
static void ev_refugiados(Reino *r) {
    int grupo = utils_rand(8, 20);
    ui_titulo("Refugiados na Estrada");
    char intro[200];
    snprintf(intro, sizeof(intro),
        "Um grupo de %d camponeses de um reino vizinho chega aos portões de Avalon, "
        "fugindo da miséria. Pedem abrigo e trabalho.", grupo);
    ui_narrar(intro);
    ui_prompt_menu("O rei os recebe?");
    ui_opcao(1, "Acolher a todos (mais gente, mas mais bocas para alimentar)");
    ui_opcao(2, "Aceitar apenas alguns trabalhadores");
    ui_opcao(3, "Recusar e mandá-los seguir caminho");
    ui_falar_opcoes();
    char buf[160];
    switch (ler_escolha(3)) {
        case 1:
            r->populacao += grupo;
            r->comida    -= grupo;          /* gasto imediato de mantimentos */
            if (r->comida < 0) r->comida = 0;
            snprintf(buf, sizeof(buf), "Avalon acolhe os %d recém-chegados. População: %d. Mais mãos para a lavoura, mais bocas no inverno.", grupo, r->populacao);
            ui_narrar(buf);
            break;
        case 2: {
            int parte = grupo / 2;
            r->populacao += parte;
            snprintf(buf, sizeof(buf), "Você acolhe %d trabalhadores e dispensa o resto. População: %d.", parte, r->populacao);
            ui_narrar(buf);
            break;
        }
        default:
            ui_narrar("Os portões permanecem fechados. Os refugiados seguem adiante. Avalon "
                      "poupa mantimentos, mas perde braços que poderiam lavrar a terra.");
    }
}

/* ---- Bandidos ---- */
static void ev_bandidos(Reino *r) {
    ui_titulo("Saqueadores");
    ui_narrar("Um bando de saqueadores ronda as estradas e ameaça o celeiro e o tesouro de Avalon.");
    ui_prompt_menu("Como responder?");
    ui_opcao(1, "Pagar a milícia para expulsá-los (20 de ouro)");
    ui_opcao(2, "Reunir camponeses e enfrentá-los (arriscado)");
    ui_opcao(3, "Trancar tudo e torcer para que passem");
    ui_falar_opcoes();
    char buf[160];
    switch (ler_escolha(3)) {
        case 1:
            if (r->ouro >= 20) {
                r->ouro -= 20;
                ui_narrar("A milícia patrulha as estradas e os saqueadores desaparecem sem causar danos.");
            } else {
                int perda = r->comida / 5;
                r->comida -= perda;
                snprintf(buf, sizeof(buf), "Sem ouro para a milícia, os saqueadores levam %d medidas do celeiro.", perda);
                ui_narrar(buf);
            }
            break;
        case 2:
            if (utils_chance(55)) {
                int saque = utils_rand(10, 25);
                r->ouro += saque;
                snprintf(buf, sizeof(buf), "Os camponeses repelem os bandidos e tomam %d moedas do esconderijo deles.", saque);
                ui_narrar(buf);
            } else {
                int mortos = utils_rand(2, 6);
                r->populacao -= mortos;
                if (r->populacao < 0) r->populacao = 0;
                snprintf(buf, sizeof(buf), "O confronto é sangrento. Avalon expulsa os bandidos, mas perde %d camponeses.", mortos);
                ui_narrar(buf);
            }
            break;
        default: {
            int perda = r->comida / 4;
            r->comida -= perda;
            snprintf(buf, sizeof(buf), "Os saqueadores arrombam o celeiro e levam %d medidas de comida.", perda);
            ui_narrar(buf);
        }
    }
}

/* ---- Caravana de mercadores ---- */
static void ev_caravana(Reino *r) {
    ui_titulo("Caravana de Mercadores");
    ui_narrar("Uma caravana rica acampa perto do castelo. O mestre mercador oferece um bom "
              "preço por gado e propõe vender mantimentos baratos.");
    ui_prompt_menu("O que negociar?");
    ui_opcao(1, "Vender gado a ótimo preço (12 moedas por cabeça)");
    ui_opcao(2, "Comprar comida barata (1 moeda por medida)");
    ui_opcao(3, "Apenas trocar notícias e seguir");
    ui_falar_opcoes();
    char buf[160];
    switch (ler_escolha(3)) {
        case 1: {
            if (r->gado <= 0) { ui_msg("Não há gado para vender."); break; }
            int q = ler_quantidade("Quantas cabeças vender?");
            if (q <= 0) { ui_msg("Negócio desfeito."); break; }
            if (q > r->gado) q = r->gado;
            int receita = q * 12;
            r->gado -= q; r->ouro += receita;
            snprintf(buf, sizeof(buf), "Vendidas %d cabeças por %d moedas. Rebanho: %d. Tesouro: %d.", q, receita, r->gado, r->ouro);
            ui_narrar(buf);
            break;
        }
        case 2: {
            int q = ler_quantidade("Quantas medidas de comida comprar?");
            if (q <= 0) { ui_msg("Negócio desfeito."); break; }
            if (q > r->ouro) q = r->ouro;     /* 1 moeda por medida */
            r->ouro -= q; r->comida += q;
            snprintf(buf, sizeof(buf), "Compradas %d medidas por %d moedas. Celeiro: %d. Tesouro: %d.", q, q, r->comida, r->ouro);
            ui_narrar(buf);
            break;
        }
        default:
            ui_narrar("Você conversa com os mercadores sobre terras distantes e os despede em paz.");
    }
}

/* ---- Veio de ouro ---- */
static void ev_veio_ouro(Reino *r) {
    int achado = utils_rand(30, 60);
    r->ouro += achado;
    ui_titulo("Veio de Ouro");
    char buf[160];
    snprintf(buf, sizeof(buf), "Mineiros encontram um veio de ouro nas colinas de Avalon. O tesouro real recebe %d moedas. Total: %d.", achado, r->ouro);
    ui_narrar(buf);
}

/* ---- Boa safra de bezerros ---- */
static void ev_bezerros(Reino *r) {
    if (r->gado <= 0) return;
    int cap = reino_capacidade_pasto(r);
    int crias = r->gado * 20 / 100;
    if (crias < 2) crias = 2;
    r->gado += crias;
    if (r->gado > cap) r->gado = cap;
    ui_titulo("Estação Fértil");
    char buf[160];
    snprintf(buf, sizeof(buf), "Os bezerros nascem fortes nesta estação. O rebanho ganha %d cabeças. Total: %d.", crias, r->gado);
    ui_narrar(buf);
}

/* ---- Frente fria ---- */
static void ev_geada(Reino *r) {
    int perda = r->comida / 8;
    if (perda < 5) perda = 5;
    if (perda > r->comida) perda = r->comida;
    r->comida -= perda;
    ui_titulo("Frente Fria");
    char buf[160];
    snprintf(buf, sizeof(buf), "Uma frente fria estraga parte dos grãos guardados. O celeiro perde %d medidas, restando %d.", perda, r->comida);
    ui_narrar(buf);
}

/* ---- Pomar generoso ---- */
static void ev_boa_safra(Reino *r) {
    int ganho = utils_rand(25, 60);
    r->comida += ganho;
    ui_titulo("Pomar Generoso");
    char buf[160];
    snprintf(buf, sizeof(buf), "Os pomares e hortas rendem além do esperado. O celeiro ganha %d medidas, somando %d.", ganho, r->comida);
    ui_narrar(buf);
}

typedef void (*FnEvento)(Reino *);

void eventos_talvez(Reino *r) {
    if (r->jogo_encerrado) return;
    /* nada no primeiro periodo, para o jogador se ambientar */
    if (r->ano == 1 && r->estacao <= EST_VERAO) return;

    if (getenv("COROA_SEM_EVENTOS")) return;
    if (!utils_chance(40)) return;

    static FnEvento pool[] = {
        ev_peste_gado, ev_refugiados, ev_bandidos, ev_caravana,
        ev_veio_ouro, ev_bezerros, ev_geada, ev_boa_safra
    };
    static int ultimo = -1;
    const int total = (int)(sizeof(pool) / sizeof(pool[0]));

    int idx;
    do { idx = utils_rand(0, total - 1); } while (idx == ultimo && total > 1);
    ultimo = idx;
    pool[idx](r);
}
