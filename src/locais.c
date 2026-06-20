#include "locais.h"
#include "jogo.h"
#include "ui.h"
#include "entrada.h"
#include "npcs.h"
#include "inventario.h"
#include "salvamento.h"
#include "../audio/audio.h"
#include <stdio.h>
#include <string.h>

extern int g_audio_ativado;

/* ================================================================
   SALÃO DO TRONO
   ================================================================ */
static void mostrar_sala_trono(void) {
    ui_titulo("SALÃO DO TRONO - Castelo de Avalon");
    ui_narrar(
        "Grandes colunas de pedra flanqueiam o salão principal do castelo. "
        "O trono de Avalon, entalhado em carvalho e ornamentado com ouro, domina o centro da sala. "
        "Mapas e relatórios cobrem a mesa do conselho. "
        "Pela janela, você avista os campos férteis do seu reino."
    );
    ui_prompt_menu("O que deseja fazer?");
    if (!g_jogador.conselheiro_preso)
        ui_opcao(1, "Convocar Lord Aldric (Conselheiro)");
    ui_opcao(2, "Convocar General Marcus");
    ui_opcao(3, "Receber o Emissário de Drakmar");
    ui_opcao(4, "Ir para a Masmorra");
    ui_opcao(5, "Ir para o Mercado");
    ui_opcao(6, "Ir para as Muralhas");
    ui_opcao(7, "Ir para a Biblioteca");
    ui_opcao(8, "Ir para os Jardins");
    ui_opcao(9, "Ir para a Vila");
    if (g_jogador.exercito >= 60)
        ui_opcao(10, "Declarar Guerra a Drakmar");
    if (g_jogador.prisioneiro_libertado && g_jogador.conselheiro_preso)
        ui_opcao(11, "Negociar Paz com Drakmar via Erik");
    if (g_jogador.povo_ajudado && g_jogador.popularidade >= 75)
        ui_opcao(12, "Convocar o Povo em Defesa do Reino");
    ui_opcao(0, "Salvar jogo");
    ui_falar_opcoes();
}

static void processar_sala_trono(const char *cmd) {
    int n = entrada_eh_numero(cmd) ? entrada_to_numero(cmd) : -1;

    if (n==1 || !strcmp(cmd,"conselheiro") || !strcmp(cmd,"aldric")) {
        if (!g_jogador.conselheiro_preso) npc_conselheiro(&g_jogador, &g_inventario);
        else ui_msg("Lord Aldric está preso na masmorra.");

    } else if (n==2 || !strcmp(cmd,"general") || !strcmp(cmd,"marcus")) {
        npc_general(&g_jogador, &g_inventario);

    } else if (n==3 || !strcmp(cmd,"emissario") || !strcmp(cmd,"cravus")) {
        npc_emissario(&g_jogador, &g_inventario);

    } else if (n==4 || !strcmp(cmd,"masmorra")) {
        g_jogador.local_atual = LOCAL_MASMORRA;
        ui_msg("Você desce as escadas em direção à masmorra.");

    } else if (n==5 || !strcmp(cmd,"mercado")) {
        g_jogador.local_atual = LOCAL_MERCADO;
        ui_msg("Você se dirige ao mercado do castelo.");

    } else if (n==6 || !strcmp(cmd,"muralhas")) {
        g_jogador.local_atual = LOCAL_MURALHAS;
        ui_msg("Você sobe as escadas até as muralhas.");

    } else if (n==7 || !strcmp(cmd,"biblioteca")) {
        g_jogador.local_atual = LOCAL_BIBLIOTECA;
        ui_msg("Você entra na silenciosa biblioteca real.");

    } else if (n==8 || !strcmp(cmd,"jardins")) {
        g_jogador.local_atual = LOCAL_JARDINS;
        ui_msg("Você caminha até os jardins do castelo.");

    } else if (n==9 || !strcmp(cmd,"vila")) {
        g_jogador.local_atual = LOCAL_VILA;
        ui_msg("Você atravessa os portões em direção à vila.");

    } else if ((n==10 || strstr(cmd,"guerra")) && g_jogador.exercito >= 60) {
        if (entrada_confirmar("Declarar guerra é uma decisão irreversível. O exército marchará contra Drakmar. Tem certeza?")) {
            g_jogador.batalha_travada = 1;
            g_jogador.final_obtido   = 1;
            g_jogador.jogo_encerrado = 1;
        } else {
            ui_msg("Você recua da decisão por ora.");
        }

    } else if ((n==11 || strstr(cmd,"paz")) &&
               g_jogador.prisioneiro_libertado && g_jogador.conselheiro_preso) {
        if (entrada_confirmar("Iniciar negociações de paz via Erik e a Rainha Serafina de Drakmar?")) {
            g_jogador.final_obtido   = 2;
            g_jogador.jogo_encerrado = 1;
        }

    } else if ((n==12 || strstr(cmd,"povo")) &&
               g_jogador.povo_ajudado && g_jogador.popularidade >= 75) {
        if (entrada_confirmar("Convocar o povo para defender Avalon é uma decisão que mudará o reino. Confirma?")) {
            g_jogador.final_obtido   = 3;
            g_jogador.jogo_encerrado = 1;
        }

    } else if (n==0 || !strcmp(cmd,"salvar")) {
        salvar_jogo(&g_jogador, &g_inventario);

    } else {
        ui_erro("Opção inválida. Digite 'opcoes' para ver o que está disponível.");
    }
}

/* ================================================================
   MASMORRA
   ================================================================ */
static void mostrar_masmorra(void) {
    ui_titulo("MASMORRA - Castelo de Avalon");
    if (g_jogador.prisioneiro_libertado) {
        ui_narrar("O ar úmido e frio da masmorra. As tochas crepitam. A cela onde Erik esteve preso está agora vazia e aberta.");
    } else {
        ui_narrar(
            "O ar úmido e frio da masmorra arrepia a pele. Tochas crepitam nas paredes de pedra. "
            "No fundo, atrás das grades enferrujadas, um homem de aparência nobre mas maltratada "
            "observa você com olhos determinados."
        );
    }
    ui_prompt_menu("O que deseja fazer?");
    if (!g_jogador.prisioneiro_libertado) {
        ui_opcao(1, "Falar com o prisioneiro");
        if (inventario_tem(&g_inventario, ITEM_CHAVE_MASMORRA))
            ui_opcao(2, "Libertar o prisioneiro com a chave");
    }
    ui_opcao(9, "Voltar ao Salão do Trono");
    ui_falar_opcoes();
}

static void processar_masmorra(const char *cmd) {
    int n = entrada_eh_numero(cmd) ? entrada_to_numero(cmd) : -1;

    if ((n==1 || !strcmp(cmd,"falar") || !strcmp(cmd,"prisioneiro") || !strcmp(cmd,"erik"))
        && !g_jogador.prisioneiro_libertado) {
        npc_prisioneiro(&g_jogador, &g_inventario);

    } else if ((n==2 || !strcmp(cmd,"libertar")) && inventario_tem(&g_inventario, ITEM_CHAVE_MASMORRA)) {
        npc_prisioneiro(&g_jogador, &g_inventario);

    } else if (n==9 || !strcmp(cmd,"voltar") || !strcmp(cmd,"trono")) {
        g_jogador.local_atual = LOCAL_SALA_TRONO;
        ui_msg("Você sobe as escadas de volta ao salão do trono.");

    } else {
        ui_erro("Opção inválida. Digite 'opcoes' para ver o que está disponível.");
    }
}

/* ================================================================
   MERCADO
   ================================================================ */
static void mostrar_mercado(void) {
    ui_titulo("MERCADO - Castelo de Avalon");
    ui_narrar(
        "O mercado interno do castelo está movimentado. "
        "Barracas coloridas oferecem desde provisões até equipamentos militares. "
        "O cheiro de pão fresco e o barulho das negociações enchem o ar. "
        "O mercador Tobias acena ao vê-lo chegar."
    );
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Ouro disponível: %d moedas.", g_jogador.ouro);
        ui_msg(buf);
    }
    ui_prompt_menu("O que deseja fazer?");
    ui_opcao(1, "Falar com o Mercador Tobias");
    ui_opcao(9, "Voltar ao Salão do Trono");
    ui_falar_opcoes();
}

static void processar_mercado(const char *cmd) {
    int n = entrada_eh_numero(cmd) ? entrada_to_numero(cmd) : -1;

    if (n==1 || !strcmp(cmd,"tobias") || !strcmp(cmd,"mercador") || !strcmp(cmd,"falar")) {
        npc_comerciante(&g_jogador, &g_inventario);

    } else if (n==9 || !strcmp(cmd,"voltar") || !strcmp(cmd,"trono")) {
        g_jogador.local_atual = LOCAL_SALA_TRONO;
        ui_msg("Você retorna ao salão do trono.");

    } else {
        ui_erro("Opção inválida. Digite 'opcoes' para ver o que está disponível.");
    }
}

/* ================================================================
   MURALHAS
   ================================================================ */
static void mostrar_muralhas(void) {
    ui_titulo("MURALHAS - Castelo de Avalon");
    ui_narrar(
        "Do alto das muralhas, você avista o horizonte. "
        "Soldados treinam no pátio abaixo com espadas e escudos. "
        "Ao longe, a floresta densa que separa Avalon do território de Drakmar. "
        "O vento carrega a tensão da iminente ameaça."
    );
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Força do exército: %d de cem.", g_jogador.exercito);
        ui_msg(buf);
    }
    ui_prompt_menu("O que deseja fazer?");
    ui_opcao(1, "Inspecionar as defesas");
    ui_opcao(2, "Ordenar treinamento de emergência (20 de ouro, +15 exército)");
    ui_opcao(3, "Observar o horizonte em busca de sinais de Drakmar");
    if (inventario_tem(&g_inventario, ITEM_ESPADA_REAL))
        ui_opcao(4, "Discursar para os soldados com a Espada Real (+10 exército)");
    ui_opcao(9, "Voltar ao Salão do Trono");
    ui_falar_opcoes();
}

static void processar_muralhas(const char *cmd) {
    int n = entrada_eh_numero(cmd) ? entrada_to_numero(cmd) : -1;

    if (n==1 || !strcmp(cmd,"inspecionar") || !strcmp(cmd,"defesas")) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Relatório das defesas. Exército: %d de cem.", g_jogador.exercito);
        printf("\n[Relatório das Defesas]\n  Exército: %d/100\n", g_jogador.exercito);
        tts_speak(buf);
        if (g_jogador.exercito < 40)
            ui_narrar("As muralhas são sólidas, mas os soldados são poucos e mal treinados. Uma invasão seria difícil de repelir agora.");
        else if (g_jogador.exercito < 70)
            ui_narrar("As defesas estão razoáveis. Com mais preparo, Avalon poderia resistir a um ataque de Drakmar.");
        else
            ui_narrar("As muralhas estão bem guarnecidas e os soldados estão em ótima forma. Avalon está pronta para a guerra, se necessário.");
        g_jogador.turno++;

    } else if (n==2 || strstr(cmd,"treinar") || strstr(cmd,"treinamento")) {
        if (g_jogador.ouro < 20) {
            ui_msg("Ouro insuficiente. São necessários 20 moedas para o treinamento.");
        } else {
            jogador_alterar_ouro(&g_jogador, -20);
            jogador_alterar_exercito(&g_jogador, 15);
            g_jogador.exercito_reforcado = 1;
            ui_narrar("Você ordena um treinamento intensivo. Os soldados suam e se esfalfam pelo pátio por dias. O exército está claramente mais forte e disciplinado.");
            g_jogador.turno++;
        }

    } else if (n==3 || !strcmp(cmd,"horizonte") || !strcmp(cmd,"observar")) {
        if (g_jogador.emissario_recebido)
            ui_narrar("Ao longe, você vê poeira levantada na direção de Drakmar. Os exploradores relatam movimentação de tropas. O tempo está se esgotando.");
        else
            ui_narrar("O horizonte está calmo por enquanto, mas seus exploradores relatam movimentação suspeita nas fronteiras de Drakmar.");
        g_jogador.turno++;

    } else if ((n==4 || strstr(cmd,"espada") || !strcmp(cmd,"discursar"))
               && inventario_tem(&g_inventario, ITEM_ESPADA_REAL)) {
        jogador_alterar_exercito(&g_jogador, 10);
        ui_narrar("Você ergue a Espada Real diante dos soldados. Um rugido de determinação ecoa pelo pátio. O moral das tropas dispara.");
        g_jogador.turno++;

    } else if (n==9 || !strcmp(cmd,"voltar") || !strcmp(cmd,"trono")) {
        g_jogador.local_atual = LOCAL_SALA_TRONO;
        ui_msg("Você desce das muralhas de volta ao salão do trono.");

    } else {
        ui_erro("Opção inválida. Digite 'opcoes' para ver o que está disponível.");
    }
}

/* ================================================================
   BIBLIOTECA
   ================================================================ */
static void mostrar_biblioteca(void) {
    ui_titulo("BIBLIOTECA REAL - Castelo de Avalon");
    ui_narrar(
        "Prateleiras repletas de pergaminhos e livros antigos cobrem as paredes do teto ao chão. "
        "Um escriba idoso trabalha em silêncio à luz de uma vela. "
        "O cheiro de papel antigo e tinta permeia o ar. "
        "Séculos de conhecimento aguardam aqui."
    );
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Sabedoria atual: %d de cem.", g_jogador.sabedoria);
        ui_msg(buf);
    }
    ui_prompt_menu("O que deseja fazer?");
    ui_opcao(1, "Estudar táticas de guerra e diplomacia (+10 sabedoria)");
    ui_opcao(2, "Pesquisar o histórico de Drakmar");
    if (!inventario_tem(&g_inventario, ITEM_CHAVE_MASMORRA))
        ui_opcao(3, "Vasculhar as prateleiras em busca de itens úteis");
    if (!inventario_tem(&g_inventario, ITEM_ESPADA_REAL))
        ui_opcao(4, "Examinar o armário de relíquias reais");
    if (!inventario_tem(&g_inventario, ITEM_PERGAMINHO))
        ui_opcao(5, "Ler o Pergaminho Antigo sobre negociação");
    ui_opcao(9, "Voltar ao Salão do Trono");
    ui_falar_opcoes();
}

static void processar_biblioteca(const char *cmd) {
    int n = entrada_eh_numero(cmd) ? entrada_to_numero(cmd) : -1;

    if (n==1 || !strcmp(cmd,"estudar") || !strcmp(cmd,"taticas")) {
        jogador_alterar_sabedoria(&g_jogador, 10);
        ui_narrar("Você passa horas mergulhado em tratados de guerra e diplomacia. Sua mente se expande com estratégias e conhecimentos valiosos.");
        g_jogador.turno++;

    } else if (n==2 || strstr(cmd,"pesquisar") || strstr(cmd,"drakmar")) {
        jogador_alterar_sabedoria(&g_jogador, 8);
        ui_narrar(
            "Nos arquivos, você encontra registros antigos sobre Drakmar. "
            "Há cinquenta anos, Avalon e Drakmar tinham um tratado de paz que durou décadas. "
            "O tratado foi quebrado não por guerra, mas por uma série de mal-entendidos diplomáticos. "
            "Isso sugere que a paz ainda é possível, se houver vontade."
        );
        g_jogador.turno++;

    } else if ((n==3 || !strcmp(cmd,"vasculhar") || !strcmp(cmd,"prateleiras"))
               && !inventario_tem(&g_inventario, ITEM_CHAVE_MASMORRA)) {
        inventario_adicionar(&g_inventario, ITEM_CHAVE_MASMORRA);
        ui_narrar("Entre mapas velhos e documentos empoeirados, você encontra uma chave antiga. A etiqueta diz: Cela três, Masmorra. Isso pode ser útil.");
        g_jogador.turno++;

    } else if ((n==4 || strstr(cmd,"armario") || strstr(cmd,"reliquia"))
               && !inventario_tem(&g_inventario, ITEM_ESPADA_REAL)) {
        inventario_adicionar(&g_inventario, ITEM_ESPADA_REAL);
        ui_narrar("No armário de relíquias, você encontra a lendária Espada Real de Avalon, usada pelo seu pai em batalha. Ela ainda brilha. Seus soldados a reconheceriam em qualquer lugar.");
        g_jogador.turno++;

    } else if ((n==5 || strstr(cmd,"pergaminho"))
               && !inventario_tem(&g_inventario, ITEM_PERGAMINHO)) {
        inventario_adicionar(&g_inventario, ITEM_PERGAMINHO);
        jogador_alterar_sabedoria(&g_jogador, 15);
        ui_narrar("O Pergaminho Antigo contém as memórias diplomáticas do Rei Aldric, o Primeiro, fundador de Avalon. Sua sabedoria sobre negociação e resolução de conflitos é inestimável. Você sente sua mente se expandir.");
        g_jogador.turno++;

    } else if (n==9 || !strcmp(cmd,"voltar") || !strcmp(cmd,"trono")) {
        g_jogador.local_atual = LOCAL_SALA_TRONO;
        ui_msg("Você fecha os livros e retorna ao salão do trono.");

    } else {
        ui_erro("Opção inválida. Digite 'opcoes' para ver o que está disponível.");
    }
}

/* ================================================================
   JARDINS
   ================================================================ */
static void mostrar_jardins(void) {
    ui_titulo("JARDINS REAIS - Castelo de Avalon");
    ui_narrar(
        "Os jardins reais são um refúgio de paz em meio ao caos. "
        "Fontes de mármore borbulham entre canteiros de flores coloridas. "
        "O canto dos pássaros e o cheiro das rosas criam uma atmosfera de tranquilidade. "
        "Por um momento, você consegue esquecer o peso da coroa."
    );
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Vida atual: %d de cem.", g_jogador.vida);
        ui_msg(buf);
    }
    ui_prompt_menu("O que deseja fazer?");
    ui_opcao(1, "Descansar e recuperar as forças (+25 de vida)");
    ui_opcao(2, "Refletir sobre a situação do reino");
    ui_opcao(3, "Meditar sobre suas escolhas (+5 sabedoria)");
    ui_opcao(9, "Voltar ao Salão do Trono");
    ui_falar_opcoes();
}

static void processar_jardins(const char *cmd) {
    int n = entrada_eh_numero(cmd) ? entrada_to_numero(cmd) : -1;

    if (n==1 || !strcmp(cmd,"descansar") || !strcmp(cmd,"recuperar")) {
        jogador_alterar_vida(&g_jogador, 25);
        ui_narrar("Você se senta à beira da fonte e fecha os olhos. O barulho do castelo some. Quando se levanta, sente o corpo e a mente renovados.");
        g_jogador.turno++;

    } else if (n==2 || !strcmp(cmd,"refletir") || !strcmp(cmd,"situacao")) {
        const char *drakmar  = g_jogador.emissario_recebido ? "Emissário já recebido" : "Aguardando contato";
        const char *erik_str = g_jogador.prisioneiro_libertado ? "Libertado, Erik auxilia nas negociações" :
                               g_jogador.erik_falou_aldric    ? "Preso, revelou a traição de Aldric" : "Preso, ainda não interrogado";
        const char *aldric   = g_jogador.conselheiro_preso ? "Preso" :
                               g_jogador.traidor_revelado  ? "Revelado como traidor, ainda solto" : "Ativo como conselheiro";
        const char *alianca  = g_jogador.alianca_formada ? "Formada" : "Não estabelecida";
        const char *povo_str = g_jogador.povo_ajudado ? "Leal e grato" :
                               g_jogador.missao_povo_iniciada ? "Necessitando de ajuda" : "Preocupado";
        printf("\n[Reflexão Real - Turno %d]\n", g_jogador.turno);
        printf("  Ameaça de Drakmar: %s\n", drakmar);
        printf("  Prisioneiro na masmorra: %s\n", erik_str);
        printf("  Lord Aldric: %s\n", aldric);
        printf("  Aliança com Veira: %s\n", alianca);
        printf("  Situação do povo: %s\n\n", povo_str);
        char resumo[1024];
        snprintf(resumo, sizeof(resumo),
            "Reflexão real, turno %d. "
            "Ameaça de Drakmar: %s. "
            "Prisioneiro: %s. "
            "Lord Aldric: %s. "
            "Aliança com Veira: %s. "
            "Situação do povo: %s.",
            g_jogador.turno, drakmar, erik_str, aldric, alianca, povo_str);
        if (g_audio_ativado) tts_speak(resumo);
        ui_msg("Use essas informações para decidir seu próximo passo.");
        g_jogador.turno++;

    } else if (n==3 || !strcmp(cmd,"meditar")) {
        jogador_alterar_sabedoria(&g_jogador, 5);
        ui_narrar("Sentado em silêncio, você reflete sobre cada decisão tomada. A clareza mental que emerge é valiosa. A sabedoria não vem apenas dos livros, mas também do silêncio.");
        g_jogador.turno++;

    } else if (n==9 || !strcmp(cmd,"voltar") || !strcmp(cmd,"trono")) {
        g_jogador.local_atual = LOCAL_SALA_TRONO;
        ui_msg("Você deixa os jardins e retorna ao salão do trono.");

    } else {
        ui_erro("Opção inválida. Digite 'opcoes' para ver o que está disponível.");
    }
}

/* ================================================================
   VILA
   ================================================================ */
static void mostrar_vila(void) {
    ui_titulo("VILA DE AVALON");
    if (!g_jogador.missao_povo_iniciada) {
        g_jogador.missao_povo_iniciada = 1;
        ui_narrar(
            "Além dos portões do castelo, a vila de Avalon se estende por vales verdes. "
            "Os aldeões param seu trabalho quando o rei aparece. "
            "Um ancião se aproxima, chapéu na mão: "
            "\"Majestade... a colheita falhou este ano. "
            "Muitas famílias estão passando fome. Precisamos de ajuda.\""
        );
    } else {
        ui_narrar("Os aldeões da vila olham para você com esperança e preocupação.");
    }
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "Popularidade: %d de cem. Ouro: %d moedas.", g_jogador.popularidade, g_jogador.ouro);
        ui_msg(buf);
    }
    ui_prompt_menu("O que deseja fazer?");
    ui_opcao(1, "Conversar com os aldeões sobre a situação");
    if (!g_jogador.povo_ajudado)
        ui_opcao(2, "Distribuir ouro para a vila (30 moedas, +25 popularidade)");
    if (g_jogador.popularidade >= 40)
        ui_opcao(3, "Recrutar voluntários para o exército (+15 exército, -10 popularidade)");
    if (g_jogador.povo_ajudado)
        ui_opcao(4, "Falar com o líder da vila");
    ui_opcao(9, "Voltar ao Salão do Trono");
    ui_falar_opcoes();
}

static void processar_vila(const char *cmd) {
    int n = entrada_eh_numero(cmd) ? entrada_to_numero(cmd) : -1;

    if (n==1 || !strcmp(cmd,"conversar") || !strcmp(cmd,"aldeoes")) {
        if (g_jogador.povo_ajudado)
            ui_narrar("Os aldeões falam com carinho sobre o rei que os ajudou. \"Majestade, estamos com o senhor, seja o que for. Avalon é nosso lar tanto quanto o seu.\"");
        else
            ui_narrar("Os aldeões falam de colheitas ruins, de crianças com fome e de medo da guerra. \"O senhor é nosso rei... por favor, não nos abandone.\"");
        g_jogador.turno++;

    } else if ((n==2 || strstr(cmd,"distribuir") || strstr(cmd,"ouro"))
               && !g_jogador.povo_ajudado) {
        if (g_jogador.ouro < 30) {
            ui_msg("Ouro insuficiente. São necessários 30 moedas para ajudar a vila.");
        } else {
            jogador_alterar_ouro(&g_jogador, -30);
            jogador_alterar_popularidade(&g_jogador, 25);
            g_jogador.povo_ajudado = 1;
            ui_narrar("Você distribui ouro e mantimentos entre as famílias. O ancião chora de gratidão. \"O senhor é um rei verdadeiro! Avalon tem sorte de ter alguém assim no trono.\"");
            g_jogador.turno++;
        }

    } else if ((n==3 || strstr(cmd,"recrutar")) && g_jogador.popularidade >= 40) {
        jogador_alterar_exercito(&g_jogador, 15);
        jogador_alterar_popularidade(&g_jogador, -10);
        ui_narrar("Dezenas de jovens se apresentam voluntariamente. \"Lutaremos pelo nosso lar!\" O exército cresce, mas o povo sente a perda dos filhos e maridos.");
        g_jogador.turno++;

    } else if ((n==4 || !strcmp(cmd,"lider")) && g_jogador.povo_ajudado) {
        ui_narrar("O líder da vila, o ancião Bertold, fala em voz baixa: \"Majestade, se o senhor precisar de nós, basta chamar. Não somos soldados, mas somos Avalon. Lutaremos por este reino até o fim.\"");
        if (g_jogador.popularidade >= 75)
            ui_msg("  >> Popularidade alta! Opção especial disponível no Salão do Trono: Convocar o Povo.");
        g_jogador.turno++;

    } else if (n==9 || !strcmp(cmd,"voltar") || !strcmp(cmd,"trono")) {
        g_jogador.local_atual = LOCAL_SALA_TRONO;
        ui_msg("Você retorna ao castelo.");

    } else {
        ui_erro("Opção inválida. Digite 'opcoes' para ver o que está disponível.");
    }
}

/* ================================================================
   DISPATCH PRINCIPAL
   ================================================================ */
void local_mostrar(void) {
    switch (g_jogador.local_atual) {
        case LOCAL_SALA_TRONO: mostrar_sala_trono();  break;
        case LOCAL_MASMORRA:   mostrar_masmorra();    break;
        case LOCAL_MERCADO:    mostrar_mercado();     break;
        case LOCAL_MURALHAS:   mostrar_muralhas();    break;
        case LOCAL_BIBLIOTECA: mostrar_biblioteca();  break;
        case LOCAL_JARDINS:    mostrar_jardins();     break;
        case LOCAL_VILA:       mostrar_vila();        break;
        default: ui_msg("Local desconhecido.");
    }
}

void local_processar(const char *cmd) {
    switch (g_jogador.local_atual) {
        case LOCAL_SALA_TRONO: processar_sala_trono(cmd);  break;
        case LOCAL_MASMORRA:   processar_masmorra(cmd);    break;
        case LOCAL_MERCADO:    processar_mercado(cmd);     break;
        case LOCAL_MURALHAS:   processar_muralhas(cmd);    break;
        case LOCAL_BIBLIOTECA: processar_biblioteca(cmd);  break;
        case LOCAL_JARDINS:    processar_jardins(cmd);     break;
        case LOCAL_VILA:       processar_vila(cmd);        break;
        default: ui_msg("Local desconhecido.");
    }
}
