#include "jogo.h"
#include "locais.h"
#include "salvamento.h"
#include "entrada.h"
#include "ui.h"
#include "../audio/audio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Jogador    g_jogador;
Inventario g_inventario;
int        g_audio_ativado = 1;

static int audio_disponivel(void) {
    FILE *f = fopen("piper/piper.exe", "rb");
    if (!f) return 0;
    fclose(f);
    f = fopen("piper/pt_BR-faber-medium.onnx", "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void mostrar_intro_novo_jogo(void) {
    ui_titulo("O PESO DA COROA");
    ui_narrar(
        "Os sinos do castelo ressoam na aurora fria. "
        "Você é o Rei de Avalon, soberano de terras férteis e povo leal. "
        "Mas a paz que seu pai construiu está em risco. "
        "A leste, o poderoso reino de Drakmar move suas tropas em direção às suas fronteiras. "
        "Na corte, rumores de traição circulam em sussurros. "
        "O destino de Avalon está em suas mãos."
    );
    printf("\n");
}

static void mostrar_final(void) {
    ui_separador();
    switch (g_jogador.final_obtido) {
        case 1:
            ui_titulo("FIM - VITÓRIA MILITAR");
            ui_narrar(
                "Com o exército de Avalon no auge de sua força, você declara guerra a Drakmar. "
                "A batalha é brutal, mas seus soldados lutam com o coração. "
                "O General Marcus lidera a carga final. "
                "Drakmar recua. O Rei Malachar assina uma rendição humilhante. "
                "Avalon é salva pela espada. Seu nome será cantado por gerações."
            );
            break;
        case 2:
            ui_titulo("FIM - PAZ DIPLOMÁTICA");
            ui_narrar(
                "Com o traidor preso e Erik como intermediário, você inicia negociações com a Rainha Serafina de Drakmar. "
                "As conversas são tensas, mas a sabedoria prevalece. "
                "Um tratado de paz é assinado. As fronteiras são garantidas. "
                "O comércio entre os reinos prospera. "
                "Você passará à história como o rei que escolheu a sabedoria em vez da guerra."
            );
            break;
        case 3:
            ui_titulo("FIM - HERÓI DO POVO");
            ui_narrar(
                "O povo de Avalon, grato por tudo que você fez por eles, responde ao seu chamado. "
                "Milhares de civis se erguem em defesa do reino. "
                "Drakmar, ao ver um povo unido e determinado, recua sem lutar. "
                "Nenhum rei com tal amor do povo pode ser derrubado. "
                "Avalon se torna um exemplo para todos os reinos vizinhos."
            );
            break;
        case 4:
            ui_titulo("FIM - QUEDA DE AVALON");
            ui_narrar(
                "Sem recursos, sem exército ou sem saúde para governar, "
                "Avalon não consegue resistir à ameaça de Drakmar. "
                "As tropas inimigas cruzam as fronteiras sem resistência. "
                "O povo sofre as consequências. "
                "O peso da coroa, no fim, foi pesado demais."
            );
            break;
        case 5:
            ui_titulo("FIM SECRETO - A VOZ DA RAINHA");
            ui_narrar(
                "A carta de Erik chega à Rainha Serafina de Drakmar. "
                "Ela confronta o Rei Malachar com as evidências da corrupção de Lord Aldric, "
                "que provocava a guerra para seu próprio benefício. "
                "Malachar, furioso com a traição, ordena a retirada das tropas. "
                "Uma aliança inesperada nasce entre Avalon e Drakmar. "
                "Você descobriu que o maior inimigo não estava além das fronteiras, "
                "mas dentro do próprio castelo."
            );
            break;
        default:
            ui_msg("Fim de jogo.");
    }
    printf("\nObrigado por jogar O Peso da Coroa!\n");
    printf("Turno final: %d | Ouro: %d | Exército: %d | Popularidade: %d\n",
        g_jogador.turno, g_jogador.ouro, g_jogador.exercito, g_jogador.popularidade);
    ui_separador();
}

int jogo_verificar_fim(void) {
    if (g_jogador.jogo_encerrado) return 1;
    if (g_jogador.vida <= 0 || (g_jogador.ouro <= 0 && g_jogador.exercito <= 10)) {
        g_jogador.final_obtido   = 4;
        g_jogador.jogo_encerrado = 1;
        return 1;
    }
    return 0;
}

int jogo_processar_global(const char *cmd) {
    if (!strcmp(cmd, "ajuda")) {
        ui_ajuda(); return 1;
    }
    if (!strcmp(cmd, "status")) {
        jogador_mostrar_status(&g_jogador); return 1;
    }
    if (!strcmp(cmd, "inventario") || !strcmp(cmd, "inv")) {
        inventario_mostrar(&g_inventario); return 1;
    }
    if (!strcmp(cmd, "examinar")) {
        inventario_examinar_todos(&g_inventario); return 1;
    }
    if (!strcmp(cmd, "repetir")) {
        ui_repetir(); return 1;
    }
    if (!strcmp(cmd, "opcoes")) {
        ui_opcoes_globais(); return 1;
    }
    if (!strcmp(cmd, "salvar")) {
        salvar_jogo(&g_jogador, &g_inventario); return 1;
    }
    if (!strcmp(cmd, "audio")) {
        g_audio_ativado = !g_audio_ativado;
        printf("Narração por voz: %s\n", g_audio_ativado ? "ATIVADA" : "DESATIVADA");
        return 1;
    }
    if (!strcmp(cmd, "sair")) {
        if (entrada_confirmar("Deseja realmente sair do jogo? O progresso não salvo será perdido.")) {
            printf("Saindo do jogo. Até a próxima, Majestade.\n");
            exit(0);
        }
        return 1;
    }
    return 0;
}

static void loop_jogo(void) {
    char cmd[CMD_MAX];
    local_mostrar();

    while (!jogo_verificar_fim()) {
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);

        if (cmd[0] == '\0') continue;
        if (jogo_processar_global(cmd)) continue;

        Local anterior = g_jogador.local_atual;
        local_processar(cmd);

        if (g_jogador.local_atual != anterior && !g_jogador.jogo_encerrado)
            local_mostrar();
    }

    mostrar_final();
}

static void novo_jogo(void) {
    char nome[NOME_MAX];
    if (g_audio_ativado) tts_speak("Qual é o seu nome, Majestade?");
    printf("\nQual é o seu nome, Majestade? ");
    fflush(stdout);
    if (!fgets(nome, NOME_MAX, stdin)) strcpy(nome, "Sem Nome");
    int len = (int)strlen(nome);
    if (len > 0 && nome[len-1] == '\n') nome[len-1] = '\0';
    if (nome[0] == '\0') strcpy(nome, "Avalon");

    jogador_inicializar(&g_jogador, nome);
    inventario_inicializar(&g_inventario);

    mostrar_intro_novo_jogo();
    ui_msg("Dica: Digite 'ajuda' a qualquer momento para ver os comandos disponíveis.\n");
    loop_jogo();
}

static void carregar_e_continuar(void) {
    jogador_inicializar(&g_jogador, "");
    inventario_inicializar(&g_inventario);
    if (carregar_jogo(&g_jogador, &g_inventario))
        loop_jogo();
    else
        printf("Retornando ao menu principal.\n");
}

static void perguntar_audio(void) {
    printf("\nNARRACAO POR VOZ\n\n");
    printf("Deseja ativar a narracao por voz?\n");
    printf("  1 - Sim, ativar narracao\n");
    printf("  2 - Nao, jogar sem audio\n");
    printf("\n> ");
    fflush(stdout);
    char buf[16];
    if (!fgets(buf, sizeof(buf), stdin)) return;
    if (buf[0] == '2') {
        g_audio_ativado = 0;
        printf("\nNarração desativada. Você pode ativar a qualquer momento digitando 'audio'.\n\n");
    } else {
        g_audio_ativado = 1;
        tts_speak("Narração ativada. Bem-vindo a O Peso da Coroa.");
        printf("\nNarração ativada.\n\n");
    }
}

void jogo_iniciar(void) {
    if (!audio_disponivel()) {
        g_audio_ativado = 0;
        printf("Arquivos de audio nao encontrados. Narracao por voz desativada.\n");
        printf("O jogo funcionara normalmente sem audio.\n\n");
    } else {
        perguntar_audio();
    }

    while (1) {
        ui_titulo("O PESO DA COROA - RPG Textual");
        ui_narrar("Bem-vindo a O Peso da Coroa, um RPG textual sobre governar um reino em crise.");
        printf("\n");
        ui_limpar_opcoes();
        ui_opcao(1, "Novo jogo");
        ui_opcao(2, "Carregar jogo salvo");
        ui_opcao(3, "Ajuda");
        ui_opcao(0, "Sair");
        printf("\n");
        ui_falar_opcoes();

        char cmd[CMD_MAX];
        entrada_ler(cmd, CMD_MAX);
        entrada_normalizar(cmd);

        if (!strcmp(cmd,"1") || !strcmp(cmd,"novo") || strstr(cmd,"novo")) {
            novo_jogo();
        } else if (!strcmp(cmd,"2") || !strcmp(cmd,"carregar")) {
            carregar_e_continuar();
        } else if (!strcmp(cmd,"3") || !strcmp(cmd,"ajuda")) {
            ui_ajuda();
        } else if (!strcmp(cmd,"0") || !strcmp(cmd,"sair")) {
            printf("Até logo!\n");
            break;
        } else {
            ui_erro("Opção inválida. Digite 1, 2, 3 ou 0.");
        }
    }
}
