#include "npcs.h"
#include "ui.h"
#include "entrada.h"
#include "audio.h"
#include <stdio.h>
#include <string.h>

extern int g_audio_ativado;

/* ---- LORD ALDRIC (Conselheiro) ---- */
void npc_conselheiro(Jogador *j, Inventario *inv) {
    (void)inv;
    if (j->conselheiro_preso) {
        ui_narrar("Lord Aldric está preso na masmorra aguardando julgamento.");
        return;
    }

    ui_titulo("LORD ALDRIC - Conselheiro Real");

    if (j->traidor_revelado) {
        ui_narrar("Lord Aldric está pálido. O prisioneiro revelou seus verdadeiros planos. Ele evita seu olhar.");
        ui_prompt_menu("O que deseja fazer?");
        ui_opcao(1, "Prender Lord Aldric imediatamente");
        ui_opcao(2, "Ouvir a defesa dele");
        ui_opcao(3, "Deixar por ora");
        ui_falar_opcoes();
        char cmd[CMD_MAX];
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);
        if (!strcmp(cmd,"1")) {
            j->conselheiro_preso = 1;
            ui_narrar("\"Isso é uma calúnia!\" grita Aldric enquanto a guarda real o arrasta. Com o traidor preso, o caminho para a paz com Drakmar está aberto.");
        } else if (!strcmp(cmd,"2")) {
            ui_narrar("\"Eu apenas protegia Avalon à minha maneira...\" Suas palavras soam vazias diante das evidências apresentadas por Erik.");
        } else {
            ui_msg("Você decide deixar o assunto por ora.");
        }
        j->turno++;
        return;
    }

    ui_narrar("Lord Aldric se inclina formalmente. \"Majestade, o tempo é precioso. Drakmar avança. Precisamos fortalecer nosso exército imediatamente. Qualquer tentativa de negociação é fraqueza.\"");
    ui_prompt_menu("O que deseja perguntar?");
    ui_opcao(1, "Perguntar sobre a ameaça de Drakmar");
    ui_opcao(2, "Perguntar sobre o prisioneiro na masmorra");
    ui_opcao(3, "Pedir conselho sobre alianças");
    ui_opcao(4, "Dispensar o conselheiro");
    ui_falar_opcoes();

    char cmd[CMD_MAX];
    entrada_ler(cmd, CMD_MAX);
    entrada_normalizar(cmd);

    if (!strcmp(cmd,"1")) {
        ui_narrar("\"O Rei Malachar de Drakmar é impiedoso. Ele quer nossas terras férteis. Não há espaço para diplomacia, apenas para a espada. Invista no exército, Majestade!\"");
    } else if (!strcmp(cmd,"2")) {
        ui_narrar("Aldric pisca nervosamente. \"Ah, aquele? Um espião de Drakmar, sem dúvida. Ignorem-no. Qualquer coisa que diga é mentira para desestabilizar nosso reino.\"");
        if (!j->prisioneiro_libertado)
            ui_msg("Algo no comportamento do conselheiro parece suspeito.");
    } else if (!strcmp(cmd,"3")) {
        ui_narrar("\"Alianças? Nenhum reino nos apoiaria agora. Nossa única opção é a força militar. O Reino de Veira é fraco e o de Solheim é distante demais.\"");
        if (!j->info_reino_veira)
            ui_msg("Interessante. Ele mencionou o Reino de Veira.");
    } else {
        ui_msg("Você dispensa o conselheiro.");
    }
    j->turno++;
}

/* ---- GENERAL MARCUS ---- */
void npc_general(Jogador *j, Inventario *inv) {
    (void)inv;
    ui_titulo("GENERAL MARCUS - Comandante do Exército Real");
    ui_narrar("O General Marcus bate o punho no peito em saudação. \"Meu rei. O exército está pronto para cumprir suas ordens. Qual é a sua vontade?\"");
    ui_prompt_menu("O que deseja?");
    ui_opcao(1, "Solicitar relatório do exército");
    ui_opcao(2, "Ordenar treinamento intensivo (custa 25 de ouro)");
    ui_opcao(3, "Discutir estratégia de defesa");
    ui_opcao(4, "Dispensar o general");
    ui_falar_opcoes();

    char cmd[CMD_MAX];
    entrada_ler(cmd, CMD_MAX);
    entrada_normalizar(cmd);

    if (!strcmp(cmd,"1")) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Força atual do exército: %d de cem.", j->exercito);
        printf("\nRelatorio do General Marcus. Forca atual do exercito: %d de 100.\n", j->exercito);
        if (g_audio_ativado) tts_speak(buf);
        if (j->exercito < 40)
            ui_narrar("\"Majestade, nossa situação é preocupante. Precisamos de mais recrutas e treinamento urgente.\"");
        else if (j->exercito < 70)
            ui_narrar("\"Temos uma defesa razoável, mas para enfrentar Drakmar de frente precisamos de mais preparação.\"");
        else
            ui_narrar("\"Majestade, nosso exército está em excelente forma! Podemos enfrentar Drakmar quando o senhor ordenar.\"");
    } else if (!strcmp(cmd,"2")) {
        if (j->ouro < 25) {
            ui_msg("Ouro insuficiente. São necessários 25 moedas de ouro para o treinamento.");
        } else {
            jogador_alterar_ouro(j, -25);
            jogador_alterar_exercito(j, 20);
            j->exercito_reforcado = 1;
            ui_narrar("\"Excelente decisão, Majestade!\" Os soldados treinam por dias exaustivos. O exército está mais forte e preparado.");
        }
    } else if (!strcmp(cmd,"3")) {
        jogador_alterar_sabedoria(j, 5);
        ui_narrar("Marcus desenha no mapa as posições de Drakmar. \"Se eles avançarem pelo vale norte, podemos usar as muralhas leste como armadilha. Mas precisamos de um exército forte o suficiente.\"");
    } else {
        ui_msg("Você dispensa o general.");
    }
    j->turno++;
}

/* ---- MERCADOR TOBIAS ---- */
void npc_comerciante(Jogador *j, Inventario *inv) {
    ui_titulo("TOBIAS - Mercador do Castelo");
    ui_narrar("O mercador Tobias sorri ao vê-lo. \"Majestade! Tenho os melhores suprimentos do reino. O que posso oferecer?\"");
    ui_prompt_menu("O que deseja?");
    ui_opcao(1, "Comprar suprimentos para o exército (30 de ouro, +15 exército)");
    ui_opcao(2, "Perguntar sobre outros reinos (20 de ouro por informações)");
    ui_opcao(3, "Comprar o Anel de Aliança do Reino de Veira (50 de ouro)");
    ui_opcao(4, "Ir embora");
    ui_falar_opcoes();

    char cmd[CMD_MAX];
    entrada_ler(cmd, CMD_MAX);
    entrada_normalizar(cmd);

    if (!strcmp(cmd,"1")) {
        if (j->ouro < 30) {
            ui_msg("Ouro insuficiente. São necessários 30 moedas.");
        } else {
            jogador_alterar_ouro(j, -30);
            jogador_alterar_exercito(j, 15);
            ui_narrar("\"Ótima escolha, Majestade!\" Caixas de flechas, espadas e armaduras chegam ao quartel. O exército fica mais equipado.");
        }
    } else if (!strcmp(cmd,"2")) {
        if (j->ouro < 20) {
            ui_msg("São necessários 20 moedas para essas informações.");
        } else {
            jogador_alterar_ouro(j, -20);
            jogador_alterar_sabedoria(j, 10);
            j->info_reino_veira = 1;
            ui_narrar("Tobias se aproxima e sussurra: \"O Reino de Veira está com medo de Drakmar também. Se você enviar um gesto de boa vontade, eles podem se aliar a Avalon. Procure pelo Anel de Aliança, é um símbolo de pacto entre os reinos.\"");
        }
    } else if (!strcmp(cmd,"3")) {
        if (inventario_tem(inv, ITEM_ANEL_ALIANCA)) {
            ui_msg("Você já possui o Anel de Aliança.");
        } else if (j->ouro < 50) {
            ui_msg("São necessários 50 moedas de ouro para o Anel de Aliança.");
        } else {
            jogador_alterar_ouro(j, -50);
            inventario_adicionar(inv, ITEM_ANEL_ALIANCA);
            j->alianca_formada = 1;
            ui_narrar("\"Uma escolha sábia, Majestade.\" Tobias entrega o anel com cuidado. \"Com isso, Veira será seu aliado. Drakmar pensará duas vezes antes de atacar.\"");
        }
    } else {
        ui_msg("Você deixa o mercador.");
    }
    j->turno++;
}

/* ---- PRISIONEIRO ERIK ---- */
void npc_prisioneiro(Jogador *j, Inventario *inv) {
    if (j->prisioneiro_libertado) {
        ui_narrar("A cela está vazia. Erik, o emissário da rainha de Drakmar, foi libertado e agora auxilia nas negociações de paz.");
        return;
    }

    ui_titulo("ERIK - O Prisioneiro Misterioso");
    ui_narrar("O homem de aparência nobre, mas com roupas surradas, levanta os olhos ao ver o rei. \"Majestade... finalmente. Eu precisava falar com o senhor há dias.\"");
    ui_prompt_menu("O que deseja fazer?");
    ui_opcao(1, "Perguntar quem ele é");
    ui_opcao(2, "Perguntar sobre Drakmar");
    ui_opcao(3, "Perguntar sobre Lord Aldric");
    if (inventario_tem(inv, ITEM_CHAVE_MASMORRA))
        ui_opcao(4, "Libertar o prisioneiro com a chave");
    ui_opcao(5, "Ir embora");
    ui_falar_opcoes();

    char cmd[CMD_MAX];
    entrada_ler(cmd, CMD_MAX);
    entrada_normalizar(cmd);

    if (!strcmp(cmd,"1")) {
        ui_narrar("\"Meu nome é Erik, emissário secreto da Rainha Serafina de Drakmar. Ela me enviou para propor uma paz duradoura entre nossos reinos. Mas fui interceptado e preso antes de chegar ao senhor.\"");
        ui_msg("Ele parece sincero. Seus olhos revelam determinação, não malícia.");
    } else if (!strcmp(cmd,"2")) {
        ui_narrar("\"O Rei Malachar é belicoso, é verdade. Mas a Rainha Serafina tem influência sobre ele. Ela acredita que uma guerra destruiria ambos os reinos. Eu carregava uma carta dela propondo comércio e paz.\"");
        jogador_alterar_sabedoria(j, 5);
    } else if (!strcmp(cmd,"3")) {
        j->erik_falou_aldric = 1;
        j->traidor_revelado  = 1;
        ui_narrar("Erik se aproxima das grades com urgência. \"Lord Aldric é um espião de Drakmar! Foi ele quem me interceptou e me prendeu para impedir a paz. Ele quer a guerra porque o Rei Malachar o paga para enfraquecer Avalon por dentro!\"");
        ui_msg("O traidor foi revelado! Agora voce pode confrontar Lord Aldric.");
    } else if ((!strcmp(cmd,"4") || strstr(cmd,"libertar")) && inventario_tem(inv, ITEM_CHAVE_MASMORRA)) {
        if (!j->traidor_revelado) {
            j->traidor_revelado  = 1;
            j->erik_falou_aldric = 1;
            ui_narrar("Antes de sair, Erik agarra sua mão. \"Majestade, preciso que saiba: Lord Aldric é um traidor, pago por Drakmar para impedir a paz!\"");
        }
        inventario_remover(inv, ITEM_CHAVE_MASMORRA);
        j->prisioneiro_libertado = 1;
        ui_narrar("A fechadura cede com um clique. Erik sai da cela e se ajoelha. \"Obrigado, Majestade. Posso agora atuar como intermediário para a paz com Drakmar. A Rainha Serafina ficará satisfeita.\"");
        ui_msg("Erik foi libertado! Novo caminho disponivel: negociar paz com Drakmar.");
    } else if (!strcmp(cmd,"4") || !strcmp(cmd,"5")) {
        if (!strcmp(cmd,"4") && !inventario_tem(inv, ITEM_CHAVE_MASMORRA))
            ui_msg("Você não tem a chave para abrir a cela.");
        else
            ui_msg("Você deixa o prisioneiro por ora.");
    } else {
        ui_msg("Você deixa o prisioneiro por ora.");
    }
    j->turno++;
}

/* ---- EMISSÁRIO CRAVUS ---- */
void npc_emissario(Jogador *j, Inventario *inv) {
    (void)inv;
    ui_titulo("CRAVUS - Emissário do Rei Malachar de Drakmar");

    if (!j->emissario_recebido) {
        j->emissario_recebido = 1;
        ui_narrar("Um homem de armadura negra com o brasão de Drakmar entra no salão com passos firmes. \"Rei de Avalon. O Grande Rei Malachar exige 500 moedas de ouro como tributo. Caso contrário, seus exércitos marcharão sobre suas terras em dez dias.\"");
    } else {
        ui_narrar("Cravus aguarda com impaciência a resposta de Avalon.");
    }

    ui_prompt_menu("Como responder ao emissário?");
    ui_opcao(1, "Recusar e desafiar Drakmar");
    ui_opcao(2, "Pedir mais tempo para considerar");
    if (j->ouro >= 500)
        ui_opcao(3, "Pagar o tributo (500 de ouro)");
    if (j->alianca_formada)
        ui_opcao(4, "Mostrar o Anel de Aliança com Veira");
    if (j->prisioneiro_libertado && j->conselheiro_preso)
        ui_opcao(5, "Propor paz via mensagem de Erik à Rainha Serafina");
    ui_falar_opcoes();

    char cmd[CMD_MAX];
    entrada_ler(cmd, CMD_MAX);
    entrada_normalizar(cmd);

    if (!strcmp(cmd,"1")) {
        ui_narrar("\"Levem esta resposta ao seu rei: Avalon não se curva!\" Cravus parte furioso. A guerra parece inevitável. Reforce o exército.");
        jogador_alterar_popularidade(j, 10);
    } else if (!strcmp(cmd,"2")) {
        ui_narrar("\"Você tem cinco dias, rei de Avalon. Não mais.\" Cravus recua, mas a ameaça permanece.");
    } else if (!strcmp(cmd,"3") && j->ouro >= 500) {
        jogador_alterar_ouro(j, -500);
        ui_narrar("Cravus aceita o ouro com um sorriso frio. \"Uma escolha sábia.\" Ele parte. O tributo compra tempo, mas o tesouro real está comprometido.");
        jogador_alterar_popularidade(j, -20);
    } else if (!strcmp(cmd,"4") && j->alianca_formada) {
        ui_narrar("Você exibe o Anel de Aliança de Veira. Cravus recua visivelmente. \"Veira... Então você tem apoio.\" Ele parte para consultar o rei. A ameaça imediata recua.");
        jogador_alterar_sabedoria(j, 10);
        jogador_alterar_popularidade(j, 15);
    } else if (!strcmp(cmd,"5") && j->prisioneiro_libertado && j->conselheiro_preso) {
        ui_narrar("Você entrega a Cravus uma carta escrita por Erik, emissário da Rainha Serafina. O rosto do emissário muda completamente. \"A Rainha... ela quer paz?\" Confuso, ele parte para confirmar com Drakmar.");
        j->final_obtido  = 5;
        j->jogo_encerrado = 1;
    } else {
        ui_erro("Opção inválida ou indisponível.");
        ui_msg("Use 'opcoes' para ver as opções disponíveis.");
    }
    j->turno++;
}
