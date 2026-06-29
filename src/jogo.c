#include "jogo.h"
#include "reino.h"
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
int   g_audio_ativado = 1;

/* precos do mercado e custos das obras */
#define GADO_COMPRA   9
#define GADO_VENDA    6
#define GADO_ABATE    10   /* comida obtida por cabeca abatida */
#define CAMPO_CUSTO   35
#define COMIDA_COMPRA 2    /* ouro por medida comprada */
#define COMIDA_VENDA  1    /* ouro por medida vendida */

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
    printf("  Clima:      %s\n", clima_desc(r->clima));
    if (r->fase_drakmar >= 1) {
        printf("  Soldados:   %d (muralhas nível %d)\n", r->soldados, r->muralhas);
        printf("  Ameaça de Drakmar: %d\n", r->ameaca);
    }

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
        char mil[160] = "";
        if (r->fase_drakmar >= 1)
            snprintf(mil, sizeof(mil),
                "Soldados: %d. Muralhas nível %d. Ameaça de Drakmar: %d. ",
                r->soldados, r->muralhas, r->ameaca);
        char resumo[760];
        snprintf(resumo, sizeof(resumo),
            "Ano %d de reinado, %s. "
            "População: %d habitantes. Comida: %d medidas, e o inverno exige %d. "
            "Gado: %d cabeças. Campos: %d. Ouro: %d moedas. Imposto: %d por cento. Clima: %s. %s%s",
            r->ano, reino_nome_estacao(r->estacao),
            r->populacao, r->comida, necessario, r->gado, r->campos, r->ouro,
            r->imposto, clima_desc(r->clima), mil, aviso ? aviso : "");
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

/* ---- acoes militares (surgem com a ameaca de Drakmar) ---- */
#define SOLDADO_CUSTO 5
#define MURALHA_CUSTO 40
#define MURALHA_MAX   5

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
    if (r->muralhas >= MURALHA_MAX) { ui_msg("As muralhas de Avalon já estão no máximo."); return; }
    char buf[160];
    snprintf(buf, sizeof(buf), "Reforçar as muralhas custa %d moedas. Você tem %d. Nível atual: %d de %d.",
             MURALHA_CUSTO, r->ouro, r->muralhas, MURALHA_MAX);
    ui_msg(buf);
    if (r->ouro < MURALHA_CUSTO) { ui_msg("Ouro insuficiente para a obra."); return; }
    if (!entrada_confirmar("Iniciar a obra das muralhas?")) { ui_msg("Obra adiada."); return; }
    r->ouro     -= MURALHA_CUSTO;
    r->muralhas += 1;
    snprintf(buf, sizeof(buf), "As muralhas são reforçadas. Nível de fortificação: %d. A defesa de Avalon está mais forte.", r->muralhas);
    ui_msg(buf);
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
    ui_opcao(8, "Rever a situação do reino");
    if (g_reino.fase_drakmar >= 1) {
        ui_opcao(9, "Recrutar soldados");
        ui_opcao(10, "Reforçar as muralhas");
        ui_opcao(11, "Dispensar soldados");
    }
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
            ui_titulo("Drakmar");
            ui_msg("Depois de alguns anos surgem rumores de guerra e, com eles, as ações de "
                   "recrutar soldados e reforçar muralhas. Soldados também comem do celeiro "
                   "no inverno. Mais tarde, um emissário exige tributo: você pode pagar para "
                   "adiar, submeter-se, ou recusar e ir à guerra. Quanto mais cedo se "
                   "preparar, mais forte estará na batalha final.");
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
        ui_opcao(7, "Drakmar e a guerra");
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
        ui_msg("Comandos: digite o número da ação, ou as palavras situação, opções, repetir, "
               "salvar, áudio, sair. As ações de mercado pedem uma quantidade em seguida. "
               "Digite guia para entender as regras do reino. "
               "Com a narração ligada, aperte ESC para pular a fala atual.");
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
            case 8: mostrar_relatorio(&g_reino); break;
            case 9: acao_recrutar(&g_reino); break;
            case 10: acao_fortificar(&g_reino); break;
            case 11: acao_dispensar(&g_reino); break;
            case 0: {
                int ano_antes = g_reino.ano;
                reino_avancar_estacao(&g_reino);
                if (g_reino.ano != ano_antes)
                    drakmar_inicio_de_ano(&g_reino);
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
        g_reino.final_obtido   = 5;   /* colapso por despovoamento */
        return 1;
    }
    return 0;
}

static void mostrar_final(const Reino *r) {
    ui_separador();
    switch (r->final_obtido) {
        case 1:
            ui_titulo("Fim: Triunfo Militar");
            ui_narrar("As muralhas de Avalon resistem e o exército de Drakmar é repelido com "
                      "pesadas perdas. O Rei Malachar recua e pede trégua. Avalon sobrevive pela "
                      "força das armas que você teve a sabedoria de preparar a tempo.");
            break;
        case 2:
            ui_titulo("Fim: Idade de Ouro");
            ui_narrar("Avalon não apenas repele Drakmar como sai da guerra forte e próspera. "
                      "Celeiros cheios, povo numeroso e cofres fartos: o reino que você construiu "
                      "ao longo dos anos resistiu à maior das provações. Seu reinado será lembrado "
                      "como a idade de ouro de Avalon.");
            break;
        case 3:
            ui_titulo("Fim: Reino Vassalo");
            ui_narrar("Avalon vive, mas curvada sob o jugo de Drakmar. A coroa pesou demais para "
                      "ser defendida, e a independência foi o preço da sobrevivência.");
            break;
        case 4:
            ui_titulo("Fim: Queda de Avalon");
            ui_narrar("As muralhas cedem e as tropas de Drakmar tomam o reino. O que você construiu "
                      "passa para as mãos do inimigo. Faltou preparo para a hora decisiva.");
            break;
        case 5:
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
    if (!fgets(nome, NOME_MAX, stdin)) strcpy(nome, "Avalon");
    int len = (int)strlen(nome);
    if (len > 0 && nome[len-1] == '\n') nome[len-1] = '\0';
    if (nome[0] == '\0') strcpy(nome, "Avalon");

    reino_inicializar(&g_reino, nome);
    g_reino.clima = utils_rand(60, 140);

    ui_titulo("O Peso da Coroa");
    ui_narrar(
        "Vossa Majestade assume o trono de Avalon, um reino agrário de campos verdes e "
        "rebanhos fartos. Cabe ao rei zelar pelo povo: plantar, criar gado, encher o celeiro "
        "e governar com sabedoria estação após estação. Os anos de paz são seus para construir "
        "um reino próspero. Que tipo de soberano você será?");
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
    if (!fgets(buf, sizeof(buf), stdin)) return;
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
        ui_opcao(3, "Ajuda");
        ui_opcao(0, "Sair");
        ui_falar_opcoes();

        char cmd[CMD_MAX];
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);

        if (!strcmp(cmd, "1") || strstr(cmd, "novo")) {
            novo_jogo();
        } else if (!strcmp(cmd, "2") || strstr(cmd, "carregar")) {
            carregar_e_continuar();
        } else if (!strcmp(cmd, "3") || !strcmp(cmd, "ajuda")) {
            ui_msg("Em cada estação você vê o estado do reino e escolhe ações pelo número. "
                   "Encha o celeiro antes do inverno, equilibre os impostos para o povo crescer, "
                   "e use o gado como reserva. Avance as estações para o reino prosperar.");
            mostrar_guia();
        } else if (!strcmp(cmd, "0") || !strcmp(cmd, "sair")) {
            printf("Até logo!\n");
            break;
        } else {
            ui_erro("Opção inválida. Digite 1, 2, 3 ou 0.");
        }
    }
}
