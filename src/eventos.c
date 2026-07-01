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

/* ================================================================
   EVENTOS DE COMBATE (saques, extorsão, feras)
   Padrão: pagar para evitar o ataque OU arriscar a defesa. Se a defesa
   falha, perde-se ainda MAIS do que custaria pagar; se vence, apenas se
   evita o prejuízo — nada de bom acontece. É onde a guarda ganha utilidade
   fora da guerra contra Drakmar.
   ================================================================ */

/* Chance (%) de a guarda repelir um ataque, conforme soldados e muralhas.
   Sem exército, defender é uma aposta ruim; uma boa guarda a torna segura. */
static int chance_defesa(const Reino *r) {
    int c = 25 + r->soldados * 3 + r->muralhas * 5;
    if (c > 90) c = 90;
    return c;
}

/* ---- A Companhia Livre (mercenários extorquem ouro) ---- */
static void ev_mercenarios(Reino *r) {
    int exige = utils_rand(30, 55);
    ui_titulo("A Companhia Livre");
    char intro[240], buf[300];
    snprintf(intro, sizeof(intro),
        "Uma companhia de mercenários sem contrato acampa nos arredores e exige %d moedas "
        "para partir sem tocar em Avalon.", exige);
    ui_narrar(intro);
    ui_prompt_menu("Como o rei responde?");
    ui_opcao(1, "Pagar para que sigam em paz");
    ui_opcao(2, "Chamar a guarda e expulsá-los à força");
    ui_falar_opcoes();
    if (ler_escolha(2) == 1) {
        if (r->ouro >= exige) {
            r->ouro -= exige;
            snprintf(buf, sizeof(buf), "As moedas trocam de mãos e a companhia levanta acampamento sem violência. Tesouro: %d.", r->ouro);
        } else {
            int leva_ouro = r->ouro;         r->ouro = 0;
            int leva_com  = r->comida / 6;    r->comida -= leva_com;
            snprintf(buf, sizeof(buf), "O tesouro não cobre a exigência. Furiosos, os mercenários levam as %d moedas que há e %d medidas do celeiro antes de sumir.", leva_ouro, leva_com);
        }
        ui_narrar(buf);
    } else {
        if (utils_chance(chance_defesa(r))) {
            ui_narrar("A guarda fecha as fileiras. Sem apetite para um cerco caro, os mercenários "
                      "recuam para outras terras. Avalon nada ganha além da paz.");
        } else {
            int leva_ouro = r->ouro / 3;
            int leva_com  = r->comida / 4;
            int mortos    = r->soldados > 0 ? utils_rand(1, r->soldados / 3 + 1) : 0;
            r->ouro     -= leva_ouro;
            r->comida   -= leva_com;
            r->soldados -= mortos;
            char extra[80] = "";
            if (mortos > 0) snprintf(extra, sizeof(extra), " e deixam %d soldados caídos", mortos);
            snprintf(buf, sizeof(buf), "O confronto corre mal: os mercenários rompem a linha, saqueiam %d moedas e %d medidas do celeiro%s. Ter saído para a luta custou bem mais que pagar.", leva_ouro, leva_com, extra);
            ui_narrar(buf);
        }
    }
}

/* ---- Cavaleiros do Norte (saqueadores exigem tributo em gado) ---- */
static void ev_barbaros(Reino *r) {
    if (r->gado < 4) return;
    int pede = utils_rand(4, 9);
    if (pede > r->gado) pede = r->gado;
    ui_titulo("Cavaleiros do Norte");
    char intro[260], buf[300];
    snprintf(intro, sizeof(intro),
        "Cavaleiros do norte, endurecidos por invernos de fome, cruzam a fronteira. Prometem "
        "poupar as aldeias em troca de %d cabeças de gado.", pede);
    ui_narrar(intro);
    ui_prompt_menu("O que o rei decide?");
    ui_opcao(1, "Entregar o gado e evitar o derramamento de sangue");
    ui_opcao(2, "Postar a guarda e enfrentá-los");
    ui_falar_opcoes();
    if (ler_escolha(2) == 1) {
        r->gado -= pede;
        snprintf(buf, sizeof(buf), "O gado é entregue e os cavaleiros partem satisfeitos. Rebanho: %d.", r->gado);
        ui_narrar(buf);
    } else {
        if (utils_chance(chance_defesa(r))) {
            ui_narrar("A guarda repele os cavaleiros nas cercanias e eles somem na poeira. Os "
                      "currais estão a salvo — Avalon apenas evitou o prejuízo.");
        } else {
            int perde_gado = r->gado / 3 + pede;
            if (perde_gado > r->gado) perde_gado = r->gado;
            int mortos = utils_rand(2, 7);
            if (mortos > r->populacao) mortos = r->populacao;
            r->gado      -= perde_gado;
            r->populacao -= mortos;
            snprintf(buf, sizeof(buf), "A defesa falha: os cavaleiros rompem as cercas, tocam %d cabeças de gado e queimam uma aldeia, matando %d habitantes. Resistir custou muito mais.", perde_gado, mortos);
            ui_narrar(buf);
        }
    }
}

/* ---- Matilha faminta (feras atacam o rebanho) ---- */
static void ev_feras(Reino *r) {
    if (r->gado < 4) return;
    int custo = 20;
    ui_titulo("Matilha Faminta");
    ui_narrar("Uma grande matilha de lobos desce das montanhas e ronda os currais de Avalon, "
              "faminta pelo rebanho.");
    char buf[280];
    ui_prompt_menu("Como proteger os rebanhos?");
    ui_opcao(1, "Contratar caçadores e armar armadilhas (20 de ouro)");
    ui_opcao(2, "Enviar soldados para vigiar os currais");
    ui_falar_opcoes();
    if (ler_escolha(2) == 1) {
        if (r->ouro >= custo) {
            r->ouro -= custo;
            snprintf(buf, sizeof(buf), "Os caçadores emboscam a matilha e a afugentam. O rebanho fica a salvo. Tesouro: %d.", r->ouro);
        } else {
            int perde = r->gado / 5;
            r->gado -= perde;
            snprintf(buf, sizeof(buf), "Sem ouro para caçadores, os lobos investem e levam %d cabeças na noite. Rebanho: %d.", perde, r->gado);
        }
        ui_narrar(buf);
    } else {
        if (utils_chance(chance_defesa(r))) {
            ui_narrar("Os soldados montam guarda com fogueiras e lanças. Os lobos rondam, mas não "
                      "passam. O rebanho amanhece intacto — nada além do prejuízo evitado.");
        } else {
            int perde   = r->gado / 3;
            int feridos = (r->soldados > 0 && utils_chance(40)) ? 1 : 0;
            r->gado     -= perde;
            r->soldados -= feridos;
            char extra[90] = "";
            if (feridos > 0) snprintf(extra, sizeof(extra), " e um soldado é dilacerado na escuridão");
            snprintf(buf, sizeof(buf), "A vigília falha: os lobos rompem o cerco e matam %d cabeças%s. A noite cobra caro.", perde, extra);
            ui_narrar(buf);
        }
    }
}

/* ---- Praga de ratos no celeiro ---- */
static void ev_ratos(Reino *r) {
    if (r->comida < 20) return;
    int perde = r->comida / 7;
    if (perde < 8) perde = 8;
    if (perde > r->comida) perde = r->comida;
    r->comida -= perde;
    ui_titulo("Praga no Celeiro");
    char buf[200];
    snprintf(buf, sizeof(buf), "Ratos infestam o celeiro real e roem os grãos guardados. Perdem-se %d medidas, restando %d.", perde, r->comida);
    ui_narrar(buf);
}

/* ================================================================
   ARCO DE DRAKMAR
   ================================================================ */
static int forca_defensiva(const Reino *r) {
    int defesa = r->soldados * 2
               + r->muralhas * 15
               + r->populacao / 10;
    if (r->comida >= reino_comida_necessaria(r) * 2)
        defesa += 15;   /* reservas para aguentar um cerco */
    return defesa;
}

/* Descricao vaga da ameaca: o jogador sente a sombra crescer, mas nunca
   conhece o numero exato (e tem de decidir no escuro quanto se preparar). */
const char *drakmar_ameaca_desc(const Reino *r) {
    if (r->fase_drakmar < 1) return "nenhuma";
    if (r->ameaca < 70)  return "distante";
    if (r->ameaca < 100) return "crescente";
    if (r->ameaca < 135) return "grave";
    return "iminente";
}

static void drakmar_guerra(Reino *r) {
    r->fase_drakmar = 3;
    int defesa  = forca_defensiva(r);
    int ataque  = r->ameaca;

    ui_separador();
    ui_titulo("A Guerra de Drakmar");
    ui_narrar("O exército do Rei Malachar de Drakmar surge no horizonte e marcha sobre "
              "Avalon. Os portões se fecham, os soldados tomam posição nas muralhas. "
              "O destino do reino será decidido nos próximos dias.");
    {
        char buf[220];
        snprintf(buf, sizeof(buf),
            "Força de defesa de Avalon: %d. O exército de Drakmar cobre o horizonte — "
            "ninguém sabe ao certo quantos são. Serão três assaltos.",
            defesa);
        ui_msg(buf);
    }

    int vitorias = 0;
    const char *nomes[3] = {"Primeiro assalto", "Segundo assalto", "Assalto final"};
    for (int i = 0; i < 3; i++) {
        /* Drakmar leva vantagem: rola mais alto que a defesa e parte de um
           patamar maior, exigindo bem mais preparo do jogador para vencer. */
        int meu  = defesa + utils_rand(0, 18);
        int dele = ataque + utils_rand(8, 32);
        char buf[200];
        if (meu > dele) {
            vitorias++;
            snprintf(buf, sizeof(buf), "%s: Avalon repele o inimigo das muralhas. Vitórias: %d de 3.", nomes[i], vitorias);
        } else {
            snprintf(buf, sizeof(buf), "%s: a linha de defesa cede sob o ataque. Vitórias de Avalon: %d de 3.", nomes[i], vitorias);
        }
        ui_msg(buf);
    }

    /* baixas */
    int baixas = utils_rand(r->soldados/4 + 1, r->soldados/2 + 5);
    if (baixas > r->soldados) baixas = r->soldados;
    r->soldados -= baixas;

    r->jogo_encerrado = 1;
    if (vitorias >= 2) {
        int prospero = (r->populacao >= 180 && r->ouro >= 250);
        r->final_obtido = prospero ? 2 : 1;
    } else {
        r->final_obtido = 4;
    }
}

static void drakmar_tributo(Reino *r) {
    int pede_ouro   = r->ameaca * 3;
    int pede_comida = r->ameaca * 4;
    ui_titulo("Emissário de Drakmar");
    char intro[300];
    snprintf(intro, sizeof(intro),
        "Um emissário de armadura negra entra na corte: \"Rei de Avalon, o grande Malachar "
        "exige tributo: %d moedas de ouro, ou %d medidas de grão. Pague, e Avalon terá mais "
        "um ano de paz. Recuse, e nossos exércitos decidirão a questão.\"",
        pede_ouro, pede_comida);
    ui_narrar(intro);
    ui_prompt_menu("Como o rei responde?");
    ui_opcao(1, "Pagar o tributo em ouro");
    ui_opcao(2, "Pagar o tributo em comida");
    ui_opcao(3, "Recusar e enfrentar Drakmar na guerra");
    ui_opcao(4, "Submeter-se a Drakmar como vassalo");
    ui_falar_opcoes();

    char buf[220];
    switch (ler_escolha(4)) {
        case 1:
            if (r->ouro >= pede_ouro) {
                r->ouro -= pede_ouro;
                r->tributos_pagos++;
                snprintf(buf, sizeof(buf), "O tributo de %d moedas é pago. Drakmar recua por este ano, "
                         "mas voltará a exigir mais. Tesouro: %d.", pede_ouro, r->ouro);
                ui_narrar(buf);
            } else {
                int leva = r->ouro;
                r->ouro = 0;
                r->ameaca += 8;
                snprintf(buf, sizeof(buf), "O tesouro não cobre o tributo. O emissário leva as %d moedas "
                         "que há e parte furioso. Drakmar fica ainda mais ameaçador.", leva);
                ui_narrar(buf);
            }
            break;
        case 2:
            if (r->comida >= pede_comida) {
                r->comida -= pede_comida;
                r->tributos_pagos++;
                snprintf(buf, sizeof(buf), "O tributo de %d medidas de grão é entregue. Drakmar recua por "
                         "este ano. Celeiro: %d.", pede_comida, r->comida);
                ui_narrar(buf);
            } else {
                int leva = r->comida;
                r->comida = 0;
                r->ameaca += 8;
                snprintf(buf, sizeof(buf), "O celeiro não cobre o tributo. O emissário leva as %d medidas "
                         "que há e parte furioso. Drakmar fica ainda mais ameaçador.", leva);
                ui_narrar(buf);
            }
            break;
        case 3:
            ui_narrar("\"Diga a Malachar que Avalon não se curva.\" O emissário parte em silêncio. "
                      "A guerra é inevitável agora.");
            drakmar_guerra(r);
            break;
        default:
            ui_titulo("Fim: Reino Vassalo");
            ui_narrar("Avalon dobra os joelhos diante de Drakmar. O reino sobrevive, mas paga tributo "
                      "eterno e perde sua independência. Seu povo viverá, porém sob a sombra de outro rei. "
                      "Houve quem chamasse isso de prudência, e quem chamasse de covardia.");
            r->final_obtido   = 3;
            r->jogo_encerrado = 1;
    }
}

/* ---- Eventos menores de Drakmar, ao longo das estacoes ----
   Mantem a sombra de Drakmar presente o ano inteiro, e nao so na troca de ano.
   Cada um deixa o jogador agir: gastar tropas, ouro ou suportar prejuizo,
   afetando (sem revelar) o quanto a ameaca cresce. */

static void dk_ev_batedores(Reino *r) {
    ui_titulo("Batedores na Fronteira");
    ui_narrar("Sentinelas avistam batedores de Drakmar medindo as defesas de Avalon à beira "
              "da floresta. Não atacam — apenas observam, contam soldados e anotam as brechas.");
    ui_prompt_menu("O que o rei ordena?");
    ui_opcao(1, "Enviar soldados para expulsá-los");
    ui_opcao(2, "Subornar um deles por informações (30 de ouro)");
    ui_opcao(3, "Ignorar e deixá-los ir");
    ui_falar_opcoes();
    char buf[240];
    switch (ler_escolha(3)) {
        case 1:
            if (r->soldados >= 3) {
                int perda = utils_rand(0, 2);
                r->soldados -= perda;
                r->ameaca   -= 7;
                snprintf(buf, sizeof(buf), "Seus soldados perseguem os batedores mata adentro. Drakmar "
                         "perde os olhos sobre Avalon e repensa o ataque. Baixas: %d. Exército: %d.",
                         perda, r->soldados);
            } else {
                r->ameaca += 6;
                snprintf(buf, sizeof(buf), "Sem tropas para persegui-los, os batedores vagam à vontade "
                         "e voltam para Drakmar com um mapa das brechas de Avalon.");
            }
            ui_narrar(buf);
            break;
        case 2:
            if (r->ouro >= 30) {
                r->ouro   -= 30;
                r->ameaca -= 10;
                snprintf(buf, sizeof(buf), "Por 30 moedas, um batedor ganancioso revela os planos de "
                         "Malachar. Avalon ganha um tempo precioso para se preparar. Tesouro: %d.", r->ouro);
            } else {
                snprintf(buf, sizeof(buf), "O tesouro não junta as 30 moedas pedidas. Os batedores somem "
                         "na floresta sem dizer palavra.");
            }
            ui_narrar(buf);
            break;
        default:
            r->ameaca += 4;
            ui_narrar("Os batedores partem em paz — levando consigo tudo o que viram das muralhas de Avalon.");
    }
}

static void dk_ev_saque(Reino *r) {
    ui_titulo("Saque na Fronteira");
    ui_narrar("Cavaleiros de Drakmar cruzam a fronteira e tocam fogo num povoado de Avalon, testando "
              "a coragem do rei. Camponeses apavorados correm para a corte pedindo proteção.");
    ui_prompt_menu("Como Avalon responde?");
    ui_opcao(1, "Revidar com a guarda (arrisca soldados)");
    ui_opcao(2, "Comprar a retirada com ouro (40 de ouro)");
    ui_opcao(3, "Recolher o povo e suportar o prejuízo");
    ui_falar_opcoes();
    char buf[240];
    switch (ler_escolha(3)) {
        case 1:
            if (r->soldados >= 4) {
                int perda = utils_rand(1, 3);
                r->soldados -= perda;
                r->ameaca   -= 8;
                snprintf(buf, sizeof(buf), "A guarda de Avalon repele os cavaleiros e os faz sangrar na "
                         "retirada. Drakmar aprende a temer estas muralhas. Baixas: %d. Exército: %d.",
                         perda, r->soldados);
            } else {
                int mortos = utils_rand(2, 6);
                if (mortos > r->populacao) mortos = r->populacao;
                r->populacao -= mortos;
                r->ameaca    += 7;
                snprintf(buf, sizeof(buf), "Com tão poucos soldados, o revide vira massacre. Avalon perde "
                         "%d almas e Drakmar ri da fraqueza do reino. População: %d.", mortos, r->populacao);
            }
            ui_narrar(buf);
            break;
        case 2:
            if (r->ouro >= 40) {
                r->ouro -= 40;
                snprintf(buf, sizeof(buf), "Por 40 moedas, os saqueadores recuam sem mais sangue. Mas "
                         "Malachar anota que Avalon prefere pagar a lutar. Tesouro: %d.", r->ouro);
            } else {
                int leva = r->ouro;
                r->ouro    = 0;
                r->ameaca += 6;
                snprintf(buf, sizeof(buf), "O tesouro só tem %d moedas; os saqueadores levam tudo e queimam "
                         "mais um celeiro por desprezo.", leva);
            }
            ui_narrar(buf);
            break;
        default: {
            int perda = utils_rand(8, 20);
            if (perda > r->comida) perda = r->comida;
            r->comida -= perda;
            snprintf(buf, sizeof(buf), "O rei recolhe o povo atrás das muralhas e deixa o povoado arder. "
                     "Vidas são poupadas, mas o celeiro perde %d medidas no saque.", perda);
            ui_narrar(buf);
        }
    }
}

static void dk_ev_desertor(Reino *r) {
    ui_titulo("Um Desertor de Drakmar");
    ui_narrar("Um soldado desertor de Drakmar chega esfomeado aos portões. Jura conhecer os planos de "
              "Malachar e oferece o que sabe — em troca de abrigo e algumas moedas.");
    ui_prompt_menu("O rei o recebe?");
    ui_opcao(1, "Acolhê-lo e pagar por informações (25 de ouro)");
    ui_opcao(2, "Alistá-lo na guarda de Avalon");
    ui_opcao(3, "Expulsá-lo como possível espião");
    ui_falar_opcoes();
    char buf[240];
    switch (ler_escolha(3)) {
        case 1:
            if (r->ouro >= 25) {
                r->ouro   -= 25;
                r->ameaca -= 11;
                snprintf(buf, sizeof(buf), "O desertor revela rotas, senhas e as fraquezas do exército "
                         "inimigo. Avalon se prepara muito melhor para o que vem. Tesouro: %d.", r->ouro);
            } else {
                snprintf(buf, sizeof(buf), "Sem moedas a oferecer, o desertor desconfia e guarda o que "
                         "sabe. Some na primeira noite.");
            }
            ui_narrar(buf);
            break;
        case 2:
            r->soldados += 1;
            r->ameaca   -= 4;
            snprintf(buf, sizeof(buf), "O desertor jura lealdade a Avalon e empunha a lança nas muralhas. "
                     "Exército: %d.", r->soldados);
            ui_narrar(buf);
            break;
        default:
            ui_narrar("Por precaução, o rei manda expulsá-lo. Talvez fosse mesmo um espião — ou talvez "
                      "Avalon tenha acabado de perder um aliado.");
    }
}

/* Talvez dispare um encontro com Drakmar no inicio de uma estacao, enquanto a
   tensao cresce (fase 1 ou 2, antes da guerra aberta). */
void drakmar_evento_estacao(Reino *r) {
    if (r->jogo_encerrado) return;
    if (r->fase_drakmar < 1 || r->fase_drakmar >= 3) return;
    if (getenv("COROA_SEM_EVENTOS")) return;
    if (!utils_chance(35)) return;

    switch (utils_rand(0, 2)) {
        case 0:  dk_ev_batedores(r); break;
        case 1:  dk_ev_saque(r);     break;
        default: dk_ev_desertor(r);  break;
    }
    if (r->ameaca < 30) r->ameaca = 30;   /* a sombra nunca some por completo */
}

void drakmar_inicio_de_ano(Reino *r) {
    if (r->jogo_encerrado) return;

    if (r->fase_drakmar == 0) {
        if (r->ano >= 2) {
            r->fase_drakmar = 1;
            r->ameaca = 45;
            ui_titulo("Rumores do Leste");
            ui_narrar("Mercadores trazem notícias sombrias: a leste, o reino de Drakmar arma seus "
                      "exércitos e olha com cobiça para as terras férteis de Avalon. Ainda é só um "
                      "rumor, mas seria sábio começar a pensar na defesa do reino desde já.");
            ui_msg("Novas ações surgem: recrutar soldados e reforçar as muralhas.");
        }
        return;
    }

    /* fase >= 1: Drakmar cresce a cada ano. Como a ameaca surge cedo, o reino tem
       mais anos para se preparar — e Drakmar tambem tem mais tempo para crescer. */
    r->ameaca += utils_rand(9, 14);

    if (r->fase_drakmar == 1 && r->ano >= 6) {
        r->fase_drakmar = 2;
        ui_titulo("A Sombra se Aproxima");
        ui_narrar("Os rumores viram certeza: Drakmar exige agora que Avalon reconheça seu poder. "
                  "Um emissário está a caminho da corte.");
    }

    if (r->fase_drakmar == 2)
        drakmar_tributo(r);
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
        ev_veio_ouro, ev_bezerros, ev_geada, ev_boa_safra,
        ev_mercenarios, ev_barbaros, ev_feras, ev_ratos
    };
    static int ultimo = -1;
    const int total = (int)(sizeof(pool) / sizeof(pool[0]));

    int idx;
    do { idx = utils_rand(0, total - 1); } while (idx == ultimo && total > 1);
    ultimo = idx;
    pool[idx](r);
}
