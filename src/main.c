/*
 * main.c - Ponto de entrada e loop principal do jogo
 * Keep Solving and Nobody Explodes - Versao de Treino
 * Trabalho Pratico - Programacao Concorrente IDP 2025/2
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

EstadoJogoCompleto* jogo = NULL;
static volatile sig_atomic_t sinal_recebido = 0;

void handler_sinal(int sig) {
    (void)sig;
    sinal_recebido = 1;
    if (jogo) {
        jogo->executando = false;
    }
}

int menu_principal(ConfigJogo* config) {
    (void)config;
    int opcao = 0;
    int tecla;

    /* Modo bloqueante para o menu */
    nodelay(stdscr, FALSE);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    flushinp();

    while (1) {
        display_menu(opcao);
        tecla = getch();

        switch (tecla) {
            case KEY_UP: case 'w': case 'W': opcao = (opcao - 1 + 4) % 4; break;
            case KEY_DOWN: case 's': case 'S': opcao = (opcao + 1) % 4; break;
            case '1': return 0;
            case '2': return 1;
            case '3': return 2;
            case '4': case 'q': case 'Q': return 3;
            case '\n': case KEY_ENTER: return opcao;
        }
        if (sinal_recebido) return 3;
    }
}

void menu_configuracoes(ConfigJogo* config) {
    if (!config) return;
    int campo = 0;
    int num_campos = 7;
    int tecla;

    nodelay(stdscr, FALSE);
    cbreak();
    noecho();
    flushinp();

    while (1) {
        display_configuracao(config, campo);
        tecla = getch();

        switch (tecla) {
            case KEY_UP: case 'w': case 'W': campo = (campo - 1 + num_campos) % num_campos; break;
            case KEY_DOWN: case 's': case 'S': campo = (campo + 1) % num_campos; break;
            case KEY_LEFT: case 'a': case 'A':
                switch (campo) {
                    case 0: if (config->num_tedax > 1) config->num_tedax--; break;
                    case 1: if (config->num_bancadas > 1) config->num_bancadas--; break;
                    case 2: if (config->tempo_partida > 30) config->tempo_partida -= 30; break;
                    case 3: if (config->dificuldade > 1) config->dificuldade--; break;
                    case 4: if (config->modulos_para_vencer > 5) config->modulos_para_vencer -= 5; break;
                    case 5: config->modo_infinito = !config->modo_infinito; break;
                } break;
            case KEY_RIGHT: case 'd': case 'D':
                switch (campo) {
                    case 0: if (config->num_tedax < MAX_TEDAX) config->num_tedax++; break;
                    case 1: if (config->num_bancadas < MAX_BANCADAS) config->num_bancadas++; break;
                    case 2: if (config->tempo_partida < 300) config->tempo_partida += 30; break;
                    case 3: if (config->dificuldade < 3) config->dificuldade++; break;
                    case 4: if (config->modulos_para_vencer < 50) config->modulos_para_vencer += 5; break;
                    case 5: config->modo_infinito = !config->modo_infinito; break;
                } break;
            case '\n': case KEY_ENTER: if (campo == 6) return; break;
            case 27: case 'q': case 'Q': return;
        }
        if (sinal_recebido) return;
    }
}

bool loop_partida(void) {
    int tecla;

    /* Configura modo NAO BLOQUEANTE */
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE); /* getch retorna ERR se nao houver tecla */
    
    /* IMPORTANTE: flushinp aqui APENAS no inicio, nunca dentro do loop */
    flushinp(); 

    while (jogo->executando) {
        EstadoJogo estado = jogo_obter_estado(jogo);

        /* Verifica Fim de Jogo */
        if (estado == JOGO_VITORIA || estado == JOGO_DERROTA) {
            usleep(500000);
            jogo_parar_partida(jogo);
            
            nodelay(stdscr, FALSE); /* Volta para modo bloqueante */
            display_fim_jogo(jogo);
            flushinp();
            getch();
            return true;
        }

        /* --- DESENHA TELA --- */
        erase();
        int linha = 0;
        attron(COLOR_PAIR(COR_TITULO) | A_BOLD);
        mvprintw(linha, (COLS - 50) / 2, "KEEP SOLVING AND NOBODY EXPLODES - Versao de Treino");
        attroff(COLOR_PAIR(COR_TITULO) | A_BOLD);
        linha += 2;

        display_modulos_pendentes(jogo, linha);
        linha += ALTURA_MODULOS + 1;
        display_bancadas(jogo, linha);
        linha += ALTURA_BANCADAS + 1;
        display_tedax(jogo, linha);
        linha += ALTURA_TEDAX + 1;
        display_status(jogo, linha);
        linha += ALTURA_STATUS + 1;
        display_comando(jogo, linha);

        if (estado == JOGO_PAUSADO) {
            attron(COLOR_PAIR(COR_ALERTA) | A_BOLD | A_BLINK);
            mvprintw(LINES / 2, (COLS - 20) / 2, "*** JOGO PAUSADO ***");
            attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD | A_BLINK);
        }
        refresh();
        /* -------------------- */

        /* Leitura de Input - Loop para ler buffer rapido */
        while ((tecla = getch()) != ERR) {
            switch (tecla) {
                case 'q':
                case 'Q':
                    jogo->executando = false; 
                    jogo_feedback(jogo, "Saindo do jogo...");
                    jogo_parar_partida(jogo);
                    nodelay(stdscr, FALSE);
                    return true;

                case 'p':
                case 'P':
                    if (jogo->pos_buffer > 0) {
                        jogo_adicionar_char_comando(jogo, (char)tecla);
                    } else {
                        jogo_pausar(jogo);
                    }
                    break;

                case 'h':
                case 'H':
                    if (jogo_obter_estado(jogo) == JOGO_RODANDO) jogo_pausar(jogo);
                    nodelay(stdscr, FALSE);
                    display_ajuda();
                    flushinp();
                    getch();
                    nodelay(stdscr, TRUE);
                    flushinp();
                    if (jogo_obter_estado(jogo) == JOGO_PAUSADO) jogo_pausar(jogo);
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
                    if (jogo_obter_estado(jogo) == JOGO_RODANDO) jogo_executar_comando(jogo);
                    break;

                default:
                    if (tecla >= 32 && tecla < 127) jogo_adicionar_char_comando(jogo, (char)tecla);
                    break;
            }
        }
        
        /* Delay de 50ms (20 FPS) para nao consumir 100% CPU */
        usleep(50000);

        if (sinal_recebido) {
            jogo_parar_partida(jogo);
            return false;
        }
    }

    nodelay(stdscr, FALSE);
    return false;
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    signal(SIGINT, handler_sinal);
    signal(SIGTERM, handler_sinal);
    srand(time(NULL));

    if (display_init() != 0) {
        fprintf(stderr, "Erro ncurses!\n");
        return 1;
    }

    jogo = malloc(sizeof(EstadoJogoCompleto));
    if (!jogo) {
        display_finalizar();
        return 1;
    }

    ConfigJogo config = config_padrao();
    if (jogo_init(jogo, &config) != 0) {
        free(jogo);
        display_finalizar();
        return 1;
    }

    bool continuar = true;
    while (continuar && !sinal_recebido) {
        int opcao = menu_principal(&config);
        switch (opcao) {
            case 0: 
                pthread_mutex_lock(&jogo->mutex_estado);
                memcpy(&jogo->config, &config, sizeof(ConfigJogo));
                pthread_mutex_unlock(&jogo->mutex_estado);
                jogo->executando = true; 
                if (jogo_iniciar_partida(jogo) == 0) continuar = loop_partida();
                break;
            case 1: menu_configuracoes(&config); break;
            case 2: 
                nodelay(stdscr, FALSE);
                display_ajuda(); 
                getch(); 
                break;
            case 3: continuar = false; break;
        }
    }

    jogo->executando = false;
    jogo_finalizar(jogo);
    free(jogo);
    display_finalizar();
    
    printf("Obrigado por jogar!\n");
    return 0;
}