#include "jogo.h"
#include "reino.h"
#include "mundo.h"
#include "salvamento.h"
#include "entrada.h"
#include "ui.h"
#include "audio.h"
#include "utils.h"
#include "eventos.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Reino g_reino;
Mundo g_mundo;
int   g_audio_ativado = 1;

/* precos do mercado e custos das obras */
#define GADO_COMPRA   9
#define GADO_VENDA    6
#define GADO_ABATE    10   /* comida obtida por cabeca abatida */
#define CAMPO_CUSTO   35
#define COMIDA_COMPRA 2    /* ouro por medida comprada */
#define COMIDA_VENDA  1    /* ouro por medida vendida */

/* minas: cada nova mina custa mais que a anterior */
#define MINA_BASE     120
#define MINA_PASSO    60
static int mina_custo(const Reino *r) { return MINA_BASE + MINA_PASSO * r->minas; }

/* ================================================================
   AUDIO
   ================================================================ */
static int audio_disponivel(void) {
#ifdef _WIN32
    FILE *f = fopen("piper/piper.exe", "rb");
#else
    FILE *f = fopen("piper/piper", "rb");
#endif
    if (!f) return 0;
    fclose(f);
    f = fopen("models/pt_BR-faber-medium.onnx", "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void falar(const char *t) { if (g_audio_ativado) tts_speak(t); }

/* ================================================================
   RELATORIO DO REINO
   ================================================================ */
static const char *clima_desc(int clima) {
    if (clima < 75)  return "ano de seca e geadas, lavoura fraca";
    if (clima < 90)  return "ano abaixo do normal para a lavoura";
    if (clima <= 110) return "ano normal para a lavoura";
    if (clima <= 130) return "ano bom, chuvas na medida";
    return "ano excelente, colheita farta a caminho";
}

static void mostrar_relatorio(const Reino *r) {
    int necessario = reino_comida_necessaria(r);
    int colheita   = reino_colheita_prevista(r);
    int trab       = reino_campos_trabalhaveis(r);
    int pasto      = reino_capacidade_pasto(r);

    printf("\nAno %d de reinado. %s.\n\n", r->ano, reino_nome_estacao(r->estacao));
    printf("  População:  %d habitantes\n", r->populacao);
    printf("  Comida:     %d medidas no celeiro (o inverno exige %d)\n", r->comida, necessario);
    printf("  Gado:       %d cabeças (o pasto sustenta até %d)\n", r->gado, pasto);
    printf("  Campos:     %d (lavráveis com a mão de obra atual: %d)\n", r->campos, trab);
    printf("  Ouro:       %d moedas no tesouro\n", r->ouro);
    printf("  Imposto:    %d por cento\n", r->imposto);
    printf("  Minas:      %d (próxima custa %d de ouro)\n", r->minas, mina_custo(r));
    printf("  Clima:      %s\n", clima_desc(r->clima));
    printf("  Soldados:   %d\n", r->soldados);
    printf("  Muralha:    nível %d (vida %d de %d)\n",
           r->muralha_nivel, r->muralha_vida, reino_muralha_vida_max(r->muralha_nivel));
    printf("  Reinos no mapa: %d (anexe todos para vencer)\n", mundo_reinos_ativos(&g_mundo));

    /* aviso de planejamento */
    const char *aviso = NULL;
    if (r->estacao == EST_PRIMAVERA || r->estacao == EST_VERAO) {
        if (r->comida + colheita < necessario)
            aviso = "Atenção: as reservas mais a colheita prevista podem não bastar para o inverno.";
    } else {
        if (r->comida < necessario)
            aviso = "Atenção: o celeiro está abaixo do que o inverno vai exigir.";
    }
    if (aviso) printf("\n  %s\n", aviso);
    printf("\n");

    if (g_audio_ativado) {
        char resumo[820];
        snprintf(resumo, sizeof(resumo),
            "Ano %d de reinado, %s. "
            "População: %d habitantes. Comida: %d medidas, e o inverno exige %d. "
            "Gado: %d cabeças. Campos: %d. Ouro: %d moedas. Imposto: %d por cento. "
            "Minas: %d. Soldados: %d. Muralha nível %d com %d de vida. "
            "Reinos no mapa: %d. Clima: %s. %s",
            r->ano, reino_nome_estacao(r->estacao),
            r->populacao, r->comida, necessario, r->gado, r->campos, r->ouro,
            r->imposto, r->minas, r->soldados, r->muralha_nivel, r->muralha_vida,
            mundo_reinos_ativos(&g_mundo), clima_desc(r->clima), aviso ? aviso : "");
        tts_speak(resumo);
    }
}

/* ================================================================
   ENTRADA AUXILIAR
   ================================================================ */
static int ler_quantidade(const char *pergunta) {
    char buf[CMD_MAX];
    printf("%s ", pergunta);
    falar(pergunta);
    fflush(stdout);
    entrada_ler(buf, CMD_MAX);
    entrada_normalizar(buf);
    if (!entrada_eh_numero(buf)) return 0;
    return atoi(buf);
}

/* ================================================================
   ACOES DE GESTAO
   ================================================================ */
static void acao_impostos(Reino *r) {
    char buf[180];
    snprintf(buf, sizeof(buf),
        "O imposto atual é de %d por cento. Abaixo de %d por cento o reino atrai novos moradores; "
        "acima de 20 por cento, parte do povo migra para outras terras.", r->imposto, 15);
    ui_msg(buf);
    int novo = ler_quantidade("Defina a nova alíquota de imposto, de 0 a 60 por cento:");
    if (novo < 0) novo = 0;
    if (novo > 60) novo = 60;
    r->imposto = novo;
    snprintf(buf, sizeof(buf), "Imposto definido em %d por cento.", r->imposto);
    ui_msg(buf);
}

static void acao_comprar_gado(Reino *r) {
    char buf[160];
    snprintf(buf, sizeof(buf), "Cada cabeça de gado custa %d moedas. Você tem %d moedas.", GADO_COMPRA, r->ouro);
    ui_msg(buf);
    int q = ler_quantidade("Quantas cabeças deseja comprar?");
    if (q <= 0) { ui_msg("Compra cancelada."); return; }
    int custo = q * GADO_COMPRA;
    if (custo > r->ouro) { ui_msg("Ouro insuficiente para essa compra."); return; }
    int cap = reino_capacidade_pasto(r);
    if (r->gado + q > cap)
        ui_msg("Aviso: parte do rebanho excede o pasto e não vai se reproduzir até haver mais campos.");
    r->ouro -= custo;
    r->gado += q;
    snprintf(buf, sizeof(buf), "Compradas %d cabeças por %d moedas. Rebanho: %d. Tesouro: %d.", q, custo, r->gado, r->ouro);
    ui_msg(buf);
}

static void acao_vender_gado(Reino *r) {
    if (r->gado <= 0) { ui_msg("Não há gado para vender."); return; }
    char buf[160];
    snprintf(buf, sizeof(buf), "Cada cabeça rende %d moedas na venda. Você tem %d cabeças.", GADO_VENDA, r->gado);
    ui_msg(buf);
    int q = ler_quantidade("Quantas cabeças deseja vender?");
    if (q <= 0) { ui_msg("Venda cancelada."); return; }
    if (q > r->gado) q = r->gado;
    int receita = q * GADO_VENDA;
    r->gado -= q;
    r->ouro += receita;
    snprintf(buf, sizeof(buf), "Vendidas %d cabeças por %d moedas. Rebanho: %d. Tesouro: %d.", q, receita, r->gado, r->ouro);
    ui_msg(buf);
}

static void acao_abater_gado(Reino *r) {
    if (r->gado <= 0) { ui_msg("Não há gado para abater."); return; }
    char buf[160];
    snprintf(buf, sizeof(buf), "Cada cabeça abatida rende %d medidas de comida. Você tem %d cabeças.", GADO_ABATE, r->gado);
    ui_msg(buf);
    int q = ler_quantidade("Quantas cabeças deseja abater?");
    if (q <= 0) { ui_msg("Abate cancelado."); return; }
    if (q > r->gado) q = r->gado;
    int ganho = q * GADO_ABATE;
    r->gado   -= q;
    r->comida += ganho;
    snprintf(buf, sizeof(buf), "Abatidas %d cabeças, rendendo %d medidas. Celeiro: %d. Rebanho: %d.", q, ganho, r->comida, r->gado);
    ui_msg(buf);
}

static void acao_rocar_campo(Reino *r) {
    char buf[160];
    snprintf(buf, sizeof(buf), "Roçar um novo campo custa %d moedas. Você tem %d.", CAMPO_CUSTO, r->ouro);
    ui_msg(buf);
    int q = ler_quantidade("Quantos campos novos deseja roçar?");
    if (q <= 0) { ui_msg("Nenhum campo roçado."); return; }
    int custo = q * CAMPO_CUSTO;
    if (custo > r->ouro) {
        int possivel = r->ouro / CAMPO_CUSTO;
        if (possivel <= 0) { ui_msg("Ouro insuficiente para roçar campos."); return; }
        q = possivel; custo = q * CAMPO_CUSTO;
    }
    r->ouro   -= custo;
    r->campos += q;
    snprintf(buf, sizeof(buf), "Roçados %d campos por %d moedas. Campos: %d. Eles renderão na próxima colheita.", q, custo, r->campos);
    ui_msg(buf);
}

static void acao_comprar_comida(Reino *r) {
    char buf[160];
    snprintf(buf, sizeof(buf), "Cada medida de comida custa %d moedas. Você tem %d.", COMIDA_COMPRA, r->ouro);
    ui_msg(buf);
    int q = ler_quantidade("Quantas medidas deseja comprar?");
    if (q <= 0) { ui_msg("Compra cancelada."); return; }
    int custo = q * COMIDA_COMPRA;
    if (custo > r->ouro) {
        q = r->ouro / COMIDA_COMPRA;
        if (q <= 0) { ui_msg("Ouro insuficiente."); return; }
        custo = q * COMIDA_COMPRA;
    }
    r->ouro   -= custo;
    r->comida += q;
    snprintf(buf, sizeof(buf), "Compradas %d medidas por %d moedas. Celeiro: %d. Tesouro: %d.", q, custo, r->comida, r->ouro);
    ui_msg(buf);
}

static void acao_vender_comida(Reino *r) {
    if (r->comida <= 0) { ui_msg("Não há comida no celeiro para vender."); return; }
    char buf[160];
    snprintf(buf, sizeof(buf), "Cada medida vendida rende %d moeda. Você tem %d medidas.", COMIDA_VENDA, r->comida);
    ui_msg(buf);
    int q = ler_quantidade("Quantas medidas deseja vender?");
    if (q <= 0) { ui_msg("Venda cancelada."); return; }
    if (q > r->comida) q = r->comida;
    int receita = q * COMIDA_VENDA;
    r->comida -= q;
    r->ouro   += receita;
    snprintf(buf, sizeof(buf), "Vendidas %d medidas por %d moedas. Celeiro: %d. Tesouro: %d.", q, receita, r->comida, r->ouro);
    ui_msg(buf);
}

/* ---- mina ---- */
static void acao_construir_mina(Reino *r) {
    int custo = mina_custo(r);
    char buf[200];
    snprintf(buf, sizeof(buf),
        "Construir uma nova mina custa %d moedas (cada mina é mais cara que a anterior). "
        "Você tem %d e opera %d minas. As minas rendem ouro ao fim de cada estação.",
        custo, r->ouro, r->minas);
    ui_msg(buf);
    if (r->ouro < custo) { ui_msg("Ouro insuficiente para abrir a mina."); return; }
    if (!entrada_confirmar("Iniciar a escavação da mina?")) { ui_msg("Obra adiada."); return; }
    r->ouro  -= custo;
    r->minas += 1;
    snprintf(buf, sizeof(buf), "A nova mina é aberta. Minas em operação: %d. Elas renderão ouro a cada estação.", r->minas);
    ui_msg(buf);
}

/* ---- acoes militares (no Quartel) ---- */
#define SOLDADO_CUSTO 5
#define MURALHA_CUSTO 40

static void acao_recrutar(Reino *r) {
    char buf[200];
    snprintf(buf, sizeof(buf), "Cada soldado exige 1 habitante e %d moedas para equipar. "
             "Você tem %d moedas e %d habitantes. Soldados atuais: %d.",
             SOLDADO_CUSTO, r->ouro, r->populacao, r->soldados);
    ui_msg(buf);
    int q = ler_quantidade("Quantos soldados deseja recrutar?");
    if (q <= 0) { ui_msg("Recrutamento cancelado."); return; }
    int max_ouro = r->ouro / SOLDADO_CUSTO;
    int max_pop  = r->populacao - 20;       /* deixa gente para a lavoura */
    if (max_pop < 0) max_pop = 0;
    if (q > max_ouro) q = max_ouro;
    if (q > max_pop)  q = max_pop;
    if (q <= 0) { ui_msg("Recursos ou população insuficientes para recrutar."); return; }
    r->ouro      -= q * SOLDADO_CUSTO;
    r->populacao -= q;
    r->soldados  += q;
    snprintf(buf, sizeof(buf), "Recrutados %d soldados. Exército: %d. População: %d. "
             "Lembre-se: soldados também comem do celeiro no inverno.", q, r->soldados, r->populacao);
    ui_msg(buf);
}

static void acao_dispensar(Reino *r) {
    if (r->soldados <= 0) { ui_msg("Não há soldados para dispensar."); return; }
    int q = ler_quantidade("Quantos soldados deseja dispensar de volta à lavoura?");
    if (q <= 0) { ui_msg("Nenhum soldado dispensado."); return; }
    if (q > r->soldados) q = r->soldados;
    r->soldados  -= q;
    r->populacao += q;
    char buf[140];
    snprintf(buf, sizeof(buf), "Dispensados %d soldados. Exército: %d. População: %d.", q, r->soldados, r->populacao);
    ui_msg(buf);
}

static void acao_fortificar(Reino *r) {
    char buf[220];
    int vida_max = reino_muralha_vida_max(r->muralha_nivel);

    /* muralha danificada mas no maximo de nivel: oferece reparo */
    if (r->muralha_nivel >= MURALHA_MAX) {
        if (r->muralha_vida >= vida_max) { ui_msg("A muralha já está no nível máximo e intacta."); return; }
        snprintf(buf, sizeof(buf), "A muralha está no nível máximo, mas danificada (vida %d de %d). "
                 "Reparar custa %d moedas. Você tem %d.", r->muralha_vida, vida_max, MURALHA_CUSTO, r->ouro);
        ui_msg(buf);
        if (r->ouro < MURALHA_CUSTO) { ui_msg("Ouro insuficiente para o reparo."); return; }
        if (!entrada_confirmar("Reparar a muralha?")) { ui_msg("Reparo adiado."); return; }
        r->ouro       -= MURALHA_CUSTO;
        r->muralha_vida = vida_max;
        ui_msg("A muralha é reparada e volta à vida cheia.");
        return;
    }

    int custo = MURALHA_CUSTO * (r->muralha_nivel + 1);
    int prox_max = reino_muralha_vida_max(r->muralha_nivel + 1);
    snprintf(buf, sizeof(buf),
        "Subir a muralha para o nível %d custa %d moedas. Você tem %d. "
        "Nível atual: %d (vida %d). No novo nível a muralha terá %d de vida.",
        r->muralha_nivel + 1, custo, r->ouro, r->muralha_nivel, r->muralha_vida, prox_max);
    ui_msg(buf);
    if (r->ouro < custo) { ui_msg("Ouro insuficiente para a obra."); return; }
    if (!entrada_confirmar("Iniciar a obra da muralha?")) { ui_msg("Obra adiada."); return; }
    r->ouro        -= custo;
    r->muralha_nivel += 1;
    r->muralha_vida   = reino_muralha_vida_max(r->muralha_nivel);
    snprintf(buf, sizeof(buf), "A muralha sobe ao nível %d, com %d de vida. A defesa do reino está mais forte.",
             r->muralha_nivel, r->muralha_vida);
    ui_msg(buf);
}

/* Submenu do Quartel: tudo que é militar fica aqui. */
static void menu_quartel(Reino *r) {
    for (;;) {
        ui_prompt_menu("Quartel. O que o rei ordena?");
        ui_opcao(1, "Recrutar soldados");
        ui_opcao(2, "Dispensar soldados");
        ui_opcao(3, "Reforçar a muralha");
        ui_opcao(0, "Voltar");
        ui_falar_opcoes();

        char cmd[CMD_MAX];
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);
        if (!entrada_eh_numero(cmd)) { ui_msg("Digite o número da opção."); continue; }
        switch (atoi(cmd)) {
            case 1: acao_recrutar(r); break;
            case 2: acao_dispensar(r); break;
            case 3: acao_fortificar(r); break;
            case 0: return;
            default: ui_msg("Opção inválida.");
        }
    }
}

/* ================================================================
   MENU DE GESTAO
   ================================================================ */
static void mostrar_menu_acoes(void) {
    ui_prompt_menu("O que o rei deseja fazer nesta estação?");
    ui_opcao(1, "Ajustar os impostos");
    ui_opcao(2, "Comprar gado");
    ui_opcao(3, "Vender gado");
    ui_opcao(4, "Abater gado para comida");
    ui_opcao(5, "Roçar novos campos");
    ui_opcao(6, "Comprar comida no mercado");
    ui_opcao(7, "Vender comida no mercado");
    ui_opcao(8, "Construir uma mina");
    ui_opcao(9, "Ir para o Quartel (militar)");
    ui_opcao(10, "Reinos (atacar, espionar, jornal)");
    ui_opcao(11, "Rever a situação do reino");
    ui_opcao(0, "Avançar para a próxima estação");
    ui_falar_opcoes();
}

/* Explica um topico do guia. */
static void guia_topico(int t) {
    switch (t) {
        case 1:
            ui_titulo("Estações");
            ui_msg("O ano tem quatro estações. Na primavera o gado procria e a terra é "
                   "preparada; no verão a lavoura amadurece; no outono vem a colheita; no "
                   "inverno o povo come do celeiro e os impostos são recolhidos.");
            break;
        case 2:
            ui_titulo("Comida");
            ui_msg("Cada habitante e cada soldado consome 2 medidas de comida no inverno. "
                   "Se o celeiro não bastar, há fome e parte do povo morre. No relatório, "
                   "veja a frase 'o inverno exige' para saber quanto vai precisar.");
            break;
        case 3:
            ui_titulo("Colheita");
            ui_msg("Cada campo rende grãos no outono, conforme o clima do ano e a mão de "
                   "obra. Cada campo exige cerca de 6 habitantes; se faltar gente, parte "
                   "dos campos fica sem colher.");
            break;
        case 4:
            ui_titulo("Gado");
            ui_msg("O rebanho cresce cerca de 25 por cento toda primavera, mas só até o "
                   "limite do pasto, que é de 5 cabeças por campo. Roce mais campos para "
                   "criar mais gado. Você pode vender gado por ouro ou abater por comida.");
            break;
        case 5:
            ui_titulo("Impostos");
            ui_msg("Perto de 15 por cento o reino fica equilibrado. Abaixo disso, novos "
                   "moradores chegam e a população cresce. Acima de 20 por cento, parte do "
                   "povo migra para outras terras: rende mais ouro, mas encolhe o reino.");
            break;
        case 6:
            ui_titulo("Campos");
            ui_msg("Roçar novos campos aumenta a colheita e o pasto para o gado, mas eles "
                   "só rendem se houver gente suficiente para lavrá-los.");
            break;
        case 7:
            ui_titulo("Reinos e a guerra");
            ui_msg("O mapa tem seis reinos, incluindo o seu. Para vencer, anexe todos os outros. "
                   "No menu Reinos você pode atacar, espionar (paga ouro por uma informação) e ler "
                   "o jornal das guerras do mundo. Ao atacar, você escolhe quantos soldados enviar. "
                   "Se mandar mais tropas do que a defesa do inimigo (soldados mais a vida da muralha), "
                   "você o conquista, perde tropas iguais à defesa dele e leva um terço dos recursos. "
                   "Se mandar de menos, perde todas as tropas enviadas. Os reinos rivais também "
                   "crescem, se atacam e podem invadir você: se tomarem o seu reino, o jogo acaba.");
            break;
        case 8:
            ui_titulo("Quartel e muralhas");
            ui_msg("No Quartel você recruta e dispensa soldados e reforça a muralha. Cada soldado "
                   "exige 1 habitante e come do celeiro no inverno. A muralha tem vida: ela defende, "
                   "mas não pode atacar. Quanto maior o nível, mais vida. Um ataque que falha corrói a "
                   "vida da muralha antes de atingir os soldados; se a vida zera, a muralha é destruída "
                   "e precisa ser reerguida.");
            break;
        case 9:
            ui_titulo("Minas");
            ui_msg("Minas exigem alto investimento e cada nova mina custa mais que a anterior. Em troca, "
                   "rendem ouro ao fim de cada estação, somado direto ao tesouro. Quanto mais minas, "
                   "mais ouro para sustentar exército e muralhas.");
            break;
        default:
            ui_erro("Tópico inválido.");
    }
}

/* Guia interativo: o jogador escolhe sobre o que quer saber. */
static void mostrar_guia(void) {
    for (;;) {
        ui_prompt_menu("Guia. Sobre o que deseja saber?");
        ui_opcao(1, "Estações");
        ui_opcao(2, "Comida e fome");
        ui_opcao(3, "Colheita");
        ui_opcao(4, "Gado");
        ui_opcao(5, "Impostos");
        ui_opcao(6, "Campos");
        ui_opcao(7, "Reinos e a guerra");
        ui_opcao(8, "Quartel e muralhas");
        ui_opcao(9, "Minas");
        ui_opcao(0, "Voltar");
        ui_falar_opcoes();

        char cmd[CMD_MAX];
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);
        if (!entrada_eh_numero(cmd)) {
            ui_msg("Digite o número do tópico, ou 0 para voltar.");
            continue;
        }
        int n = atoi(cmd);
        if (n == 0) return;
        guia_topico(n);
    }
}

/* Retorna 1 se um comando global foi tratado. */
static int processar_global(const char *cmd) {
    if (!strcmp(cmd, "ajuda")) {
        ui_msg("Comandos a qualquer momento: guia, que explica as regras do reino; "
               "situação; opções; repetir; salvar; áudio; velocidade, que acelera a "
               "narração; e sair. Digite o número da ação ou o nome dela. As ações de "
               "mercado pedem uma quantidade em seguida. Com a narração ligada, aperte "
               "ESC para pular a fala atual.");
        return 1;
    }
    if (!strcmp(cmd, "guia") || !strcmp(cmd, "manual") || !strcmp(cmd, "regras") || !strcmp(cmd, "dicas")) {
        mostrar_guia();
        /* o guia usa o mesmo buffer de opcoes do menu da estacao; ao sair,
           re-exibe o menu para o jogador nao confundir as duas listas */
        mostrar_menu_acoes();
        return 1;
    }
    if (!strcmp(cmd, "situacao") || !strcmp(cmd, "status") || !strcmp(cmd, "conselho")) {
        mostrar_relatorio(&g_reino); return 1;
    }
    if (!strcmp(cmd, "opcoes")) { ui_opcoes_globais(); return 1; }
    if (!strcmp(cmd, "repetir")) { ui_repetir(); return 1; }
    if (!strcmp(cmd, "salvar")) { salvar_reino(&g_reino); return 1; }
    if (!strcmp(cmd, "audio")) {
        g_audio_ativado = !g_audio_ativado;
        printf("Narração por voz: %s\n", g_audio_ativado ? "ATIVADA" : "DESATIVADA");
        return 1;
    }
    if (!strcmp(cmd, "velocidade") || !strcmp(cmd, "vel")) {
        audio_proxima_velocidade();
        salvar_config();
        char buf[80];
        snprintf(buf, sizeof(buf), "Velocidade da narração: %s.", audio_velocidade_nome());
        ui_msg(buf);
        return 1;
    }
    if (!strcmp(cmd, "sair")) {
        if (entrada_confirmar("Deseja realmente sair? O progresso não salvo será perdido.")) {
            printf("Até a próxima, Majestade.\n");
            exit(0);
        }
        return 1;
    }
    return 0;
}

/* Processa uma estacao inteira: relatorio, acoes ate avancar, e a simulacao.
   Retorna quando o jogador escolhe avancar a estacao. */
static void jogar_estacao(void) {
    eventos_talvez(&g_reino);
    mostrar_relatorio(&g_reino);
    mostrar_menu_acoes();

    char cmd[CMD_MAX];
    for (;;) {
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);
        if (cmd[0] == '\0') continue;
        if (processar_global(cmd)) continue;

        int n = entrada_eh_numero(cmd) ? atoi(cmd) : -1;
        switch (n) {
            case 1: acao_impostos(&g_reino); break;
            case 2: acao_comprar_gado(&g_reino); break;
            case 3: acao_vender_gado(&g_reino); break;
            case 4: acao_abater_gado(&g_reino); break;
            case 5: acao_rocar_campo(&g_reino); break;
            case 6: acao_comprar_comida(&g_reino); break;
            case 7: acao_vender_comida(&g_reino); break;
            case 8: acao_construir_mina(&g_reino); break;
            case 9: menu_quartel(&g_reino); mostrar_menu_acoes(); break;
            case 10:
                mundo_menu_reinos(&g_mundo, &g_reino);
                if (g_reino.jogo_encerrado) return;
                mostrar_menu_acoes();
                break;
            case 11: mostrar_relatorio(&g_reino); break;
            case 0: {
                reino_avancar_estacao(&g_reino);
                mundo_nova_estacao(&g_mundo, &g_reino);
                return;
            }
            default:
                ui_erro("Ação inválida. Digite um número do menu ou 'opcoes' para ouvi-las de novo.");
        }
    }
}

static int verificar_fim(void) {
    if (g_reino.jogo_encerrado) return 1;
    if (g_reino.populacao <= 0) {
        g_reino.jogo_encerrado = 1;
        g_reino.final_obtido   = 3;   /* colapso por despovoamento */
        return 1;
    }
    if (mundo_reinos_ativos(&g_mundo) <= 1) {
        g_reino.jogo_encerrado = 1;
        g_reino.final_obtido   = 1;   /* anexou todos os reinos */
        return 1;
    }
    return 0;
}

static void mostrar_final(const Reino *r) {
    ui_separador();
    switch (r->final_obtido) {
        case 1:
            ui_titulo("Fim: Imperador de Avalon");
            ui_narrar("Um por um, os reinos rivais caíram diante de suas tropas. Não resta no mapa "
                      "bandeira que não seja a sua. Avalon não é mais um reino entre muitos: é o "
                      "império que unificou todas as terras. Seu nome será lembrado por gerações.");
            break;
        case 2:
            ui_titulo("Fim: Reino Conquistado");
            ui_narrar("As muralhas cedem e as tropas inimigas tomam o reino. O que você construiu "
                      "passa para as mãos de outro rei. Faltou força, ou preparo, na hora decisiva.");
            break;
        case 3:
            ui_titulo("Fim: Reino Despovoado");
            ui_narrar("Avalon esvaziou-se. Sem povo para lavrar a terra, o reino deixou de existir. "
                      "A fome e a má gestão fizeram o que nenhum exército conseguiria.");
            break;
        default:
            ui_titulo("Fim");
            ui_narrar("O reinado chega ao fim.");
    }
    char buf[200];
    snprintf(buf, sizeof(buf),
        "Reinado encerrado no ano %d. População final: %d. Ouro: %d. Gado: %d. Campos: %d.",
        r->ano, r->populacao, r->ouro, r->gado, r->campos);
    ui_msg(buf);
    ui_separador();
}

/* ================================================================
   PARTIDA
   ================================================================ */
static void novo_jogo(void) {
    char nome[NOME_MAX];
    falar("Qual é o seu nome, Majestade?");
    printf("\nQual é o seu nome, Majestade? ");
    fflush(stdout);
    if (!entrada_ler_linha(nome, NOME_MAX)) strcpy(nome, "Avalon");
    if (nome[0] == '\0') strcpy(nome, "Avalon");

    reino_inicializar(&g_reino, nome);
    g_reino.clima = utils_rand(60, 140);
    mundo_inicializar(&g_mundo);

    ui_titulo("O Peso da Coroa");
    ui_narrar(
        "Vossa Majestade assume o trono de Avalon, um entre seis reinos que disputam estas "
        "terras. Cuide do povo, da lavoura e do gado, encha os cofres com minas de ouro, "
        "erga muralhas e forje um exército. Os reinos rivais crescem, se espionam e guerreiam "
        "entre si. Só haverá paz quando uma única coroa reinar sobre todas as terras: a sua. "
        "Anexe todos os reinos e torne-se imperador.");
    ui_msg("Dica: digite 'ajuda' a qualquer momento para ver os comandos.");

    while (!verificar_fim())
        jogar_estacao();
    mostrar_final(&g_reino);
}

static void carregar_e_continuar(void) {
    if (carregar_reino(&g_reino)) {
        while (!verificar_fim())
            jogar_estacao();
        mostrar_final(&g_reino);
    } else {
        printf("Retornando ao menu principal.\n");
    }
}

static void perguntar_audio(void) {
    printf("\nNarração por voz\n\n");
    printf("Deseja ativar a narração por voz?\n");
    printf("  1 - Sim, ativar narração\n");
    printf("  2 - Não, jogar sem áudio\n\n> ");
    fflush(stdout);
    tts_speak("Deseja ativar a narração por voz? Digite 1 para sim, ou 2 para não.");
    char buf[16];
    if (!entrada_ler_linha(buf, sizeof(buf))) return;
    if (buf[0] == '2') {
        g_audio_ativado = 0;
        printf("\nNarração desativada. Ative quando quiser digitando 'audio'.\n\n");
    } else {
        g_audio_ativado = 1;
        tts_speak("Narração ativada. A qualquer momento, aperte a tecla ESC para pular a narração atual. Bem-vindo a O Peso da Coroa.");
        printf("\nNarração ativada. Aperte ESC para pular uma narração.\n\n");
    }
}

void jogo_iniciar(void) {
    utils_semear();
    carregar_config();   /* restaura a velocidade da narracao escolhida antes */
    if (!audio_disponivel()) {
        g_audio_ativado = 0;
        printf("Arquivos de áudio não encontrados. Narração por voz desativada.\n");
        printf("O jogo funciona normalmente sem áudio.\n\n");
    } else {
        perguntar_audio();
    }

    while (1) {
        ui_titulo("O Peso da Coroa - Reino de Avalon");
        ui_narrar("Um jogo de governar um reino: cuide do povo, da terra e do gado, "
                  "estação após estação.");
        ui_limpar_opcoes();
        ui_opcao(1, "Novo reinado");
        ui_opcao(2, "Carregar reino salvo");
        ui_opcao(0, "Sair");
        ui_falar_opcoes();

        char cmd[CMD_MAX];
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);

        if (!strcmp(cmd, "1") || strstr(cmd, "novo")) {
            novo_jogo();
        } else if (!strcmp(cmd, "2") || strstr(cmd, "carregar")) {
            carregar_e_continuar();
        } else if (!strcmp(cmd, "0") || !strcmp(cmd, "sair")) {
            printf("Até logo!\n");
            break;
        } else if (!strcmp(cmd, "ajuda") || !strcmp(cmd, "guia")) {
            mostrar_guia();
        } else if (!strcmp(cmd, "velocidade") || !strcmp(cmd, "vel")) {
            audio_proxima_velocidade();
            salvar_config();
            char buf[80];
            snprintf(buf, sizeof(buf), "Velocidade da narração: %s.", audio_velocidade_nome());
            ui_msg(buf);
        } else if (!strcmp(cmd, "audio")) {
            g_audio_ativado = !g_audio_ativado;
            printf("Narração por voz: %s\n", g_audio_ativado ? "ATIVADA" : "DESATIVADA");
        } else {
            ui_erro("Opção inválida. Digite 1, 2 ou 0.");
        }
    }
}
