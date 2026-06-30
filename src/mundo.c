#include "mundo.h"
#include "ui.h"
#include "entrada.h"
#include "utils.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int g_audio_ativado;

/* custos espelham os do jogador, para os NPCs jogarem pelas mesmas regras */
#define NPC_SOLDADO_CUSTO 5
#define NPC_MINA_BASE     120
#define NPC_MINA_PASSO    60
#define NPC_MURALHA_CUSTO 40
#define MINA_RENDA_MAX    22
#define MINA_CHANCE       70

#define ATAQUE_CHANCE     22   /* chance de um NPC iniciar um ataque por estacao */
#define INFO_CUSTO        30   /* ouro para espionar um reino */

static const char *NOMES_NPC[NUM_NPCS] = {
    "Valdária", "Korthal", "Sílvania", "Montfort", "Ravenna"
};

/* ================================================================
   INICIALIZACAO
   ================================================================ */
void mundo_inicializar(Mundo *m) {
    memset(m, 0, sizeof(Mundo));
    for (int i = 0; i < NUM_NPCS; i++) {
        ReinoNPC *n = &m->npc[i];
        strncpy(n->nome, NOMES_NPC[i], NOME_MAX - 1);
        n->ativo        = 1;
        n->populacao    = utils_rand(100, 140);
        n->soldados     = utils_rand(10, 28);
        n->muralha_nivel = utils_rand(0, 1);
        n->muralha_vida  = reino_muralha_vida_max(n->muralha_nivel);
        n->ouro         = utils_rand(100, 200);
        n->comida       = utils_rand(150, 250);
        n->gado         = utils_rand(10, 25);
        n->minas        = utils_rand(0, 1);
    }
    m->jornal_n = 0;
}

int mundo_reinos_ativos(const Mundo *m) {
    int total = 1; /* o jogador */
    for (int i = 0; i < NUM_NPCS; i++)
        if (m->npc[i].ativo) total++;
    return total;
}

/* ================================================================
   JORNAL
   ================================================================ */
void mundo_jornal_add(Mundo *m, const char *texto) {
    if (m->jornal_n < JORNAL_MAX) {
        strncpy(m->jornal[m->jornal_n], texto, sizeof(m->jornal[0]) - 1);
        m->jornal[m->jornal_n][sizeof(m->jornal[0]) - 1] = '\0';
        m->jornal_n++;
    } else {
        /* rola tudo uma posicao para cima e grava no fim */
        for (int i = 1; i < JORNAL_MAX; i++)
            strcpy(m->jornal[i - 1], m->jornal[i]);
        strncpy(m->jornal[JORNAL_MAX - 1], texto, sizeof(m->jornal[0]) - 1);
        m->jornal[JORNAL_MAX - 1][sizeof(m->jornal[0]) - 1] = '\0';
    }
}

/* ================================================================
   COMBATE (compartilhado por jogador e NPCs)
   ================================================================ */
/* Aplica 'dano' a uma defesa: corrói a muralha primeiro, depois os soldados. */
static void aplicar_dano(int *soldados, int *m_nivel, int *m_vida, int dano) {
    if (dano <= 0) return;
    if (*m_vida > 0) {
        if (dano <= *m_vida) { *m_vida -= dano; dano = 0; }
        else                 { dano -= *m_vida; *m_vida = 0; }
        if (*m_vida == 0) *m_nivel = 0;
    }
    if (dano > 0) {
        *soldados -= dano;
        if (*soldados < 0) *soldados = 0;
    }
}

/* Resolve um ataque de 'tropas' contra a defesa apontada.
   Retorna >= 0 (sobreviventes do atacante) se conquistou; -1 se o ataque falhou.
   Em ambos os casos a defesa do alvo e ajustada. */
static int resolver_combate(int tropas, int *d_sold, int *d_mnivel, int *d_mvida) {
    int defesa = *d_sold + *d_mvida;
    if (tropas > defesa) {
        *d_sold = 0; *d_mvida = 0; *d_mnivel = 0;
        return tropas - defesa;
    }
    aplicar_dano(d_sold, d_mnivel, d_mvida, tropas);
    return -1;
}

/* ================================================================
   EVOLUCAO DOS NPCS
   ================================================================ */
static int npc_mina_custo(const ReinoNPC *n) {
    return NPC_MINA_BASE + NPC_MINA_PASSO * n->minas;
}

static void npc_evoluir(ReinoNPC *n) {
    if (!n->ativo) return;

    /* impostos e minas (rende ouro) */
    n->ouro += n->populacao / 8;
    for (int i = 0; i < n->minas; i++)
        if (utils_chance(MINA_CHANCE)) n->ouro += utils_rand(1, MINA_RENDA_MAX);

    /* crescimento populacional */
    n->populacao += n->populacao * 3 / 100 + utils_rand(0, 3);

    /* lavoura e rebanho seguem por conta propria */
    n->comida += utils_rand(20, 60);
    n->gado   += n->gado * 10 / 100;

    /* investimentos: mina, tropas e muralha */
    if (utils_chance(30) && n->ouro >= npc_mina_custo(n)) {
        n->ouro -= npc_mina_custo(n);
        n->minas++;
    }
    if (utils_chance(60) && n->populacao > 40) {
        int verba = n->ouro / 2;
        int q = verba / NPC_SOLDADO_CUSTO;
        if (q > n->populacao - 40) q = n->populacao - 40;
        if (q > 0) {
            n->ouro      -= q * NPC_SOLDADO_CUSTO;
            n->populacao -= q;
            n->soldados  += q;
        }
    }
    if (n->muralha_nivel < MURALHA_MAX) {
        int custo = NPC_MURALHA_CUSTO * (n->muralha_nivel + 1);
        if (utils_chance(25) && n->ouro >= custo) {
            n->ouro -= custo;
            n->muralha_nivel++;
            n->muralha_vida = reino_muralha_vida_max(n->muralha_nivel);
        }
    } else if (n->muralha_vida < reino_muralha_vida_max(n->muralha_nivel) && utils_chance(40)) {
        n->muralha_vida = reino_muralha_vida_max(n->muralha_nivel); /* reparo */
    }
}

/* Um NPC absorve um reino rival conquistado (1/3 dos recursos). */
static void npc_absorve_npc(ReinoNPC *vencedor, ReinoNPC *alvo) {
    vencedor->populacao += alvo->populacao / 3;
    vencedor->ouro      += alvo->ouro / 3;
    vencedor->comida    += alvo->comida / 3;
    vencedor->gado      += alvo->gado / 3;
    alvo->ativo = 0;
}

/* ================================================================
   ATAQUES DOS NPCS
   ================================================================ */
/* Um NPC ataca o jogador. Pode conquistá-lo (fim de jogo). */
static void npc_ataca_jogador(Mundo *m, ReinoNPC *ag, Reino *j) {
    int tropas = ag->soldados * utils_rand(60, 100) / 100;
    if (tropas < 1) return;
    ag->soldados -= tropas;

    char manchete[220];
    snprintf(manchete, sizeof(manchete), "%s declarou guerra e marchou sobre o seu reino!", ag->nome);
    mundo_jornal_add(m, manchete);

    ui_separador();
    ui_titulo("Invasão!");
    char buf[220];
    snprintf(buf, sizeof(buf), "O exército de %s marcha sobre o seu reino com %d soldados! "
             "Sua defesa: %d soldados e muralha com %d de vida.",
             ag->nome, tropas, j->soldados, j->muralha_vida);
    ui_narrar(buf);

    int sobrev = resolver_combate(tropas, &j->soldados, &j->muralha_nivel, &j->muralha_vida);
    if (sobrev >= 0) {
        /* o jogador foi conquistado */
        snprintf(buf, sizeof(buf), "As muralhas cedem e %s toma o seu reino. Seu reinado chega ao fim.", ag->nome);
        ui_narrar(buf);
        snprintf(manchete, sizeof(manchete), "%s conquistou o reino do jogador.", ag->nome);
        mundo_jornal_add(m, manchete);
        ag->soldados += sobrev;
        j->jogo_encerrado = 1;
        j->final_obtido   = 2;   /* derrota por conquista */
    } else {
        ui_narrar("O ataque é repelido! O inimigo recua com pesadas perdas, mas sua defesa também sangrou.");
        snprintf(manchete, sizeof(manchete), "O reino do jogador repeliu o ataque de %s.", ag->nome);
        mundo_jornal_add(m, manchete);
    }
}

/* Um NPC ataca outro NPC. */
static void npc_ataca_npc(Mundo *m, ReinoNPC *ag, ReinoNPC *alvo) {
    int tropas = ag->soldados * utils_rand(60, 100) / 100;
    if (tropas < 1) return;
    ag->soldados -= tropas;

    char manchete[220];
    int sobrev = resolver_combate(tropas, &alvo->soldados, &alvo->muralha_nivel, &alvo->muralha_vida);
    if (sobrev >= 0) {
        ag->soldados += sobrev;
        snprintf(manchete, sizeof(manchete), "%s conquistou e anexou o reino de %s.", ag->nome, alvo->nome);
        npc_absorve_npc(ag, alvo);
    } else {
        snprintf(manchete, sizeof(manchete), "%s atacou %s, mas foi repelido nas muralhas.", ag->nome, alvo->nome);
    }
    mundo_jornal_add(m, manchete);
}

/* Sorteia um alvo para o agressor entre os reinos ativos (NPCs e jogador).
   Retorna o indice do NPC alvo, -2 para o jogador, ou -1 se nao houver alvo. */
static int sortear_alvo(const Mundo *m, int idx_ag) {
    int candidatos[NUM_NPCS + 1];
    int n = 0;
    for (int i = 0; i < NUM_NPCS; i++)
        if (i != idx_ag && m->npc[i].ativo) candidatos[n++] = i;
    candidatos[n++] = -2; /* o jogador sempre e um alvo possivel */
    if (n == 0) return -1;
    return candidatos[utils_rand(0, n - 1)];
}

void mundo_nova_estacao(Mundo *m, Reino *jogador) {
    /* 1) economia dos NPCs */
    for (int i = 0; i < NUM_NPCS; i++)
        npc_evoluir(&m->npc[i]);

    /* 2) ataques (cada NPC ativo pode iniciar um) */
    for (int i = 0; i < NUM_NPCS; i++) {
        ReinoNPC *ag = &m->npc[i];
        if (!ag->ativo || ag->soldados < 10) continue;
        if (jogador->jogo_encerrado) break;
        if (!utils_chance(ATAQUE_CHANCE)) continue;

        int alvo = sortear_alvo(m, i);
        if (alvo == -1) continue;
        if (alvo == -2) {
            if (jogador->ano < 2) continue;   /* ano 1: tregua para o jogador */
            npc_ataca_jogador(m, ag, jogador);
        } else {
            npc_ataca_npc(m, ag, &m->npc[alvo]);
        }
    }
}

/* ================================================================
   SUBMENU "REINOS"
   ================================================================ */
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

/* Lista os reinos ativos numerados e devolve o indice escolhido, ou -1. */
static int escolher_reino(const Mundo *m, const char *titulo) {
    int mapa[NUM_NPCS];
    int n = 0;
    ui_prompt_menu(titulo);
    for (int i = 0; i < NUM_NPCS; i++) {
        if (!m->npc[i].ativo) continue;
        n++;
        mapa[n - 1] = i;
        char linha[120];
        snprintf(linha, sizeof(linha), "%s", m->npc[i].nome);
        ui_opcao(n, linha);
    }
    if (n == 0) { ui_msg("Não restam reinos rivais no mapa."); return -1; }
    ui_opcao(0, "Voltar");
    ui_falar_opcoes();

    char cmd[CMD_MAX];
    entrada_ler(cmd, CMD_MAX);
    entrada_normalizar(cmd);
    if (!entrada_eh_numero(cmd)) return -1;
    int e = atoi(cmd);
    if (e < 1 || e > n) return -1;
    return mapa[e - 1];
}

static void acao_atacar(Mundo *m, Reino *j) {
    int idx = escolher_reino(m, "Qual reino atacar?");
    if (idx < 0) return;
    ReinoNPC *alvo = &m->npc[idx];

    char buf[260];
    snprintf(buf, sizeof(buf),
        "Você tem %d soldados. As tropas que enviar não voltam se o ataque falhar. "
        "Para conquistar %s, é preciso enviar mais tropas do que a defesa dele somada (soldados mais muralha).",
        j->soldados, alvo->nome);
    ui_msg(buf);

    int tropas = ler_quantidade("Quantos soldados deseja enviar?");
    if (tropas <= 0) { ui_msg("Ataque cancelado."); return; }
    if (tropas > j->soldados) { ui_msg("Você não tem tantos soldados."); return; }

    j->soldados -= tropas;

    int sobrev = resolver_combate(tropas, &alvo->soldados, &alvo->muralha_nivel, &alvo->muralha_vida);
    if (sobrev >= 0) {
        /* conquista: espolios de 1/3 (gado limitado ao pasto) */
        int p = alvo->populacao / 3, o = alvo->ouro / 3, c = alvo->comida / 3, g = alvo->gado / 3;
        j->populacao += p;
        j->ouro      += o;
        j->comida    += c;
        int cap = reino_capacidade_pasto(j);
        if (j->gado + g > cap) g = cap - j->gado;
        if (g < 0) g = 0;
        j->gado += g;
        j->soldados += sobrev;   /* sobreviventes voltam */

        char nome_alvo[NOME_MAX];
        strncpy(nome_alvo, alvo->nome, NOME_MAX - 1);
        nome_alvo[NOME_MAX - 1] = '\0';
        alvo->ativo = 0;

        ui_titulo("Vitória!");
        snprintf(buf, sizeof(buf),
            "O reino de %s é conquistado e anexado! Você perdeu %d soldados na batalha. "
            "Pilhagem: %d habitantes, %d de ouro, %d de comida e %d de gado.",
            nome_alvo, tropas - sobrev, p, o, c, g);
        ui_narrar(buf);

        char manchete[220];
        snprintf(manchete, sizeof(manchete), "O reino do jogador conquistou e anexou %s.", nome_alvo);
        mundo_jornal_add(m, manchete);
    } else {
        ui_titulo("Ataque repelido");
        snprintf(buf, sizeof(buf),
            "As muralhas de %s resistem. Você perdeu todos os %d soldados enviados, "
            "mas o inimigo também sofreu baixas pesadas.", alvo->nome, tropas);
        ui_narrar(buf);

        char manchete[220];
        snprintf(manchete, sizeof(manchete), "O reino do jogador atacou %s e foi repelido.", alvo->nome);
        mundo_jornal_add(m, manchete);
    }
}

static void acao_informacoes(Mundo *m, Reino *j) {
    int idx = escolher_reino(m, "Sobre qual reino deseja obter informações?");
    if (idx < 0) return;
    ReinoNPC *alvo = &m->npc[idx];

    char buf[200];
    snprintf(buf, sizeof(buf), "Espionar %s custa %d moedas de ouro. Você tem %d.",
             alvo->nome, INFO_CUSTO, j->ouro);
    ui_msg(buf);
    if (j->ouro < INFO_CUSTO) { ui_msg("Ouro insuficiente para pagar os espiões."); return; }
    if (!entrada_confirmar("Enviar os espiões?")) { ui_msg("Missão cancelada."); return; }
    j->ouro -= INFO_CUSTO;

    /* revela um dado aleatorio sobre o reino */
    switch (utils_rand(0, 5)) {
        case 0: snprintf(buf, sizeof(buf), "Espiões relatam que %s tem cerca de %d habitantes.", alvo->nome, alvo->populacao); break;
        case 1: snprintf(buf, sizeof(buf), "Espiões relatam que %s mantém %d soldados em armas.", alvo->nome, alvo->soldados); break;
        case 2: snprintf(buf, sizeof(buf), "Espiões relatam que a muralha de %s está no nível %d (vida %d).", alvo->nome, alvo->muralha_nivel, alvo->muralha_vida); break;
        case 3: snprintf(buf, sizeof(buf), "Espiões relatam que o tesouro de %s guarda cerca de %d moedas.", alvo->nome, alvo->ouro); break;
        case 4: snprintf(buf, sizeof(buf), "Espiões relatam que %s opera %d minas de ouro.", alvo->nome, alvo->minas); break;
        default: snprintf(buf, sizeof(buf), "Espiões relatam que %s tem %d de comida no celeiro e %d cabeças de gado.", alvo->nome, alvo->comida, alvo->gado); break;
    }
    ui_narrar(buf);
}

static void acao_jornal(const Mundo *m) {
    ui_titulo("O Arauto do Reino");
    if (m->jornal_n == 0) {
        ui_msg("Nenhuma notícia de guerras chegou à corte ainda. O mundo está em paz... por ora.");
        return;
    }
    /* mostra da mais recente para a mais antiga */
    for (int i = m->jornal_n - 1; i >= 0; i--)
        ui_msg(m->jornal[i]);
}

void mundo_menu_reinos(Mundo *m, Reino *jogador) {
    for (;;) {
        ui_prompt_menu("Reinos do mapa. O que deseja fazer?");
        ui_opcao(1, "Atacar um reino");
        ui_opcao(2, "Obter informações (espionar)");
        ui_opcao(3, "Jornal: guerras do mundo");
        ui_opcao(0, "Voltar");
        ui_falar_opcoes();

        char cmd[CMD_MAX];
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);
        if (!entrada_eh_numero(cmd)) { ui_msg("Digite o número da opção."); continue; }
        switch (atoi(cmd)) {
            case 1: acao_atacar(m, jogador);
                    if (jogador->jogo_encerrado) return;
                    break;
            case 2: acao_informacoes(m, jogador); break;
            case 3: acao_jornal(m); break;
            case 0: return;
            default: ui_msg("Opção inválida.");
        }
    }
}
