/**
 * @file main.c
 * @brief Ponto de entrada do jogo
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 * Trabalho Pratico - Programacao Concorrente
 * IDP 2025/2
 *
 * Este arquivo contem:
 * - Funcao main()
 * - Loop principal do jogo
 * - Menu e configuracoes
 * - Tratamento de entrada do usuario
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <ncurses.h>

#include "../include/tipos.h"
#include "../include/jogo.h"
#include "../include/display.h"
#include "../include/modulos.h"
#include "../include/tedax.h"
#include "../include/bancada.h"

/* Variavel global do jogo */
EstadoJogoCompleto* jogo = NULL;

/* Flag para tratamento de sinais */
static volatile sig_atomic_t sinal_recebido = 0;

/**
 * @brief Handler para sinais de interrupcao
 */
void handler_sinal(int sig) {
    (void)sig; /* Evita warning */
    sinal_recebido = 1;
    if (jogo) {
        jogo->executando = false;
    }
}

/**
 * @brief Exibe e processa o menu principal
 * @param config Configuracoes do jogo
 * @return Opcao selecionada (0=jogar, 1=config, 2=ajuda, 3=sair)
 */
int menu_principal(ConfigJogo* config) {
    (void)config; /* Evita warning de parametro nao usado */
    int opcao = 0;
    int tecla;

    nodelay(stdscr, FALSE); /* Menu usa getch bloqueante */

    while (1) {
        display_menu(opcao);

        tecla = getch();

        switch (tecla) {
            case KEY_UP:
            case 'w':
            case 'W':
                opcao = (opcao - 1 + 4) % 4;
                break;

            case KEY_DOWN:
            case 's':
            case 'S':
                opcao = (opcao + 1) % 4;
                break;

            case '1':
                return 0; /* Iniciar jogo */
            case '2':
                return 1; /* Configuracoes */
            case '3':
                return 2; /* Ajuda */
            case '4':
            case 'q':
            case 'Q':
                return 3; /* Sair */

            case '\n':
            case KEY_ENTER:
                return opcao;

            default:
                break;
        }

        if (sinal_recebido) return 3;
    }
}

/**
 * @brief Exibe e processa a tela de configuracoes
 * @param config Configuracoes a serem editadas
 */
void menu_configuracoes(ConfigJogo* config) {
    if (!config) return;

    int campo = 0;
    int num_campos = 7; /* 6 campos + botao voltar */
    int tecla;

    nodelay(stdscr, FALSE); /* Configuracoes usa getch bloqueante */

    while (1) {
        display_configuracao(config, campo);

        tecla = getch();

        switch (tecla) {
            case KEY_UP:
            case 'w':
            case 'W':
                campo = (campo - 1 + num_campos) % num_campos;
                break;

            case KEY_DOWN:
            case 's':
            case 'S':
                campo = (campo + 1) % num_campos;
                break;

            case KEY_LEFT:
            case 'a':
            case 'A':
                switch (campo) {
                    case 0: /* Tedax */
                        if (config->num_tedax > 1) config->num_tedax--;
                        break;
                    case 1: /* Bancadas */
                        if (config->num_bancadas > 1) config->num_bancadas--;
                        break;
                    case 2: /* Tempo */
                        if (config->tempo_partida > 30) config->tempo_partida -= 30;
                        break;
                    case 3: /* Dificuldade */
                        if (config->dificuldade > 1) config->dificuldade--;
                        break;
                    case 4: /* Modulos para vencer */
                        if (config->modulos_para_vencer > 5) config->modulos_para_vencer -= 5;
                        break;
                    case 5: /* Modo infinito */
                        config->modo_infinito = !config->modo_infinito;
                        break;
                }
                break;

            case KEY_RIGHT:
            case 'd':
            case 'D':
                switch (campo) {
                    case 0: /* Tedax */
                        if (config->num_tedax < MAX_TEDAX) config->num_tedax++;
                        break;
                    case 1: /* Bancadas */
                        if (config->num_bancadas < MAX_BANCADAS) config->num_bancadas++;
                        break;
                    case 2: /* Tempo */
                        if (config->tempo_partida < 300) config->tempo_partida += 30;
                        break;
                    case 3: /* Dificuldade */
                        if (config->dificuldade < 3) config->dificuldade++;
                        break;
                    case 4: /* Modulos para vencer */
                        if (config->modulos_para_vencer < 50) config->modulos_para_vencer += 5;
                        break;
                    case 5: /* Modo infinito */
                        config->modo_infinito = !config->modo_infinito;
                        break;
                }
                break;

            case '\n':
            case KEY_ENTER:
                if (campo == 6) { /* Botao voltar */
                    return;
                }
                break;

            case 27: /* ESC */
            case 'q':
            case 'Q':
                return;

            default:
                break;
        }

        if (sinal_recebido) return;
    }
}

/**
 * @brief Loop principal durante uma partida
 * @return true se deve voltar ao menu, false se deve sair
 */
bool loop_partida(void) {
    int tecla;

    nodelay(stdscr, TRUE); /* Jogo usa getch nao-bloqueante */

    while (jogo->executando) {
        /* Verifica estado do jogo */
        EstadoJogo estado = jogo_obter_estado(jogo);

        if (estado == JOGO_VITORIA || estado == JOGO_DERROTA) {
            /* Aguarda um pouco para mostrar mensagens finais */
            usleep(500000);

            /* Para a partida */
            jogo_parar_partida(jogo);

            /* Mostra tela de fim */
            display_fim_jogo(jogo);
            nodelay(stdscr, FALSE);
            getch();
            nodelay(stdscr, TRUE);

            return true; /* Volta ao menu */
        }

        /* Processa entrada do usuario */
        tecla = getch();

        if (tecla != ERR) {
            switch (tecla) {
                case 'q':
                case 'Q':
                    /* Confirma saida */
                    jogo_feedback(jogo, "Saindo do jogo...");
                    jogo_parar_partida(jogo);
                    return true;

                case 'p':
                case 'P':
                    jogo_pausar(jogo);
                    break;

                case 'h':
                case 'H':
                    /* Pausa o jogo e mostra ajuda */
                    if (jogo_obter_estado(jogo) == JOGO_RODANDO) {
                        jogo_pausar(jogo);
                    }
                    display_ajuda();
                    nodelay(stdscr, FALSE);
                    getch();
                    nodelay(stdscr, TRUE);
                    if (jogo_obter_estado(jogo) == JOGO_PAUSADO) {
                        jogo_pausar(jogo); /* Despausa */
                    }
                    break;

                case 27: /* ESC */
                    jogo_limpar_comando(jogo);
                    break;

                case KEY_BACKSPACE:
                case 127:
                case '\b':
                    jogo_remover_char_comando(jogo);
                    break;

                case '\n':
                case KEY_ENTER:
                    /* Executa o comando */
                    if (jogo_obter_estado(jogo) == JOGO_RODANDO) {
                        jogo_executar_comando(jogo);
                    }
                    break;

                default:
                    /* Adiciona caractere ao buffer de comando */
                    if (tecla >= 32 && tecla < 127) {
                        jogo_adicionar_char_comando(jogo, (char)tecla);
                    }
                    break;
            }
        }

        /* Pequena pausa para nao sobrecarregar CPU */
        usleep(10000); /* 10ms */

        if (sinal_recebido) {
            jogo_parar_partida(jogo);
            return false;
        }
    }

    return false;
}

/**
 * @brief Funcao principal
 */
int main(int argc, char* argv[]) {
    (void)argc; /* Evita warning */
    (void)argv; /* Evita warning */

    /* Configura handler de sinais */
    signal(SIGINT, handler_sinal);
    signal(SIGTERM, handler_sinal);

    /* Inicializa gerador aleatorio */
    srand(time(NULL));

    /* Inicializa ncurses */
    if (display_init() != 0) {
        fprintf(stderr, "Erro ao inicializar ncurses!\n");
        return 1;
    }

    /* Aloca estrutura do jogo */
    jogo = malloc(sizeof(EstadoJogoCompleto));
    if (!jogo) {
        display_finalizar();
        fprintf(stderr, "Erro ao alocar memoria!\n");
        return 1;
    }

    /* Configuracoes iniciais */
    ConfigJogo config = config_padrao();

    /* Inicializa o jogo */
    if (jogo_init(jogo, &config) != 0) {
        free(jogo);
        display_finalizar();
        fprintf(stderr, "Erro ao inicializar jogo!\n");
        return 1;
    }

    /* Loop principal do programa */
    bool continuar = true;
    while (continuar && !sinal_recebido) {
        int opcao = menu_principal(&config);

        switch (opcao) {
            case 0: /* Iniciar jogo */
                /* Atualiza configuracoes */
                pthread_mutex_lock(&jogo->mutex_estado);
                memcpy(&jogo->config, &config, sizeof(ConfigJogo));
                pthread_mutex_unlock(&jogo->mutex_estado);

                /* Inicia a partida */
                if (jogo_iniciar_partida(jogo) == 0) {
                    continuar = loop_partida();
                }
                break;

            case 1: /* Configuracoes */
                menu_configuracoes(&config);
                break;

            case 2: /* Ajuda */
                display_ajuda();
                nodelay(stdscr, FALSE);
                getch();
                nodelay(stdscr, TRUE);
                break;

            case 3: /* Sair */
                continuar = false;
                break;
        }
    }

    /* Finaliza o jogo */
    jogo->executando = false;
    jogo_finalizar(jogo);
    free(jogo);
    jogo = NULL;

    /* Finaliza ncurses */
    display_finalizar();

    printf("\n");
    printf("=================================================\n");
    printf(" Keep Solving and Nobody Explodes - Versao de Treino\n");
    printf(" Obrigado por jogar!\n");
    printf("=================================================\n");
    printf("\n");

    return 0;
}
