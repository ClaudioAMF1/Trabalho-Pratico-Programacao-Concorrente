/**
 * @file display.c
 * @brief Implementacao da interface grafica com ncurses
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 *
 * SECAO CRITICA: A thread de display acessa recursos compartilhados
 * (estado do jogo, fila de modulos, tedax, bancadas) apenas para leitura.
 * Utiliza mutex_display para garantir consistencia na exibicao.
 */

#include "../include/display.h"
#include "../include/modulos.h"
#include "../include/bancada.h"
#include "../include/tedax.h"
#include "../include/jogo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

/* Variavel global do jogo */
extern EstadoJogoCompleto* jogo;

/* ==================== INICIALIZACAO ==================== */

int display_init(void) {
    setlocale(LC_ALL, "");

    initscr();
    if (!stdscr) return -1;

    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);  /* getch nao-bloqueante */
    curs_set(0);            /* esconde cursor */

    display_init_cores();

    return 0;
}

void display_finalizar(void) {
    nodelay(stdscr, FALSE);
    keypad(stdscr, FALSE);
    noraw();
    echo();
    curs_set(1);
    endwin();
}

void display_init_cores(void) {
    /* Usar cores mais visiveis */
    init_pair(COR_PADRAO,      COLOR_WHITE,  COLOR_BLACK);
    init_pair(COR_TITULO,      COLOR_CYAN,   COLOR_BLACK);
    init_pair(COR_SUCESSO,     COLOR_GREEN,  COLOR_BLACK);
    init_pair(COR_ERRO,        COLOR_RED,    COLOR_BLACK);
    init_pair(COR_ALERTA,      COLOR_YELLOW, COLOR_BLACK);
    init_pair(COR_INFO,        COLOR_CYAN,   COLOR_BLACK);
    init_pair(COR_DESTAQUE,    COLOR_BLACK,  COLOR_CYAN);
    init_pair(COR_TEDAX_LIVRE, COLOR_GREEN,  COLOR_BLACK);
    init_pair(COR_TEDAX_OCUP,  COLOR_RED,    COLOR_BLACK);
    init_pair(COR_BANCADA,     COLOR_MAGENTA,COLOR_BLACK);
    init_pair(COR_MODULO,      COLOR_WHITE,  COLOR_BLACK);
    init_pair(COR_TEMPO,       COLOR_YELLOW, COLOR_BLACK);
}

void display_limpar(void) {
    clear();
}

/* ==================== FUNCOES AUXILIARES ==================== */

void formatar_tempo(int segundos, char* buffer, int tamanho) {
    if (!buffer || tamanho < 6) return;
    int min = segundos / 60;
    int seg = segundos % 60;
    snprintf(buffer, tamanho, "%02d:%02d", min, seg);
}

void desenhar_caixa(int y, int x, int altura, int largura, const char* titulo) {
    /* Cantos */
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + largura - 1, ACS_URCORNER);
    mvaddch(y + altura - 1, x, ACS_LLCORNER);
    mvaddch(y + altura - 1, x + largura - 1, ACS_LRCORNER);

    /* Linhas horizontais */
    for (int i = 1; i < largura - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + altura - 1, x + i, ACS_HLINE);
    }

    /* Linhas verticais */
    for (int i = 1; i < altura - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + largura - 1, ACS_VLINE);
    }

    /* Titulo */
    if (titulo && strlen(titulo) > 0) {
        int pos = x + (largura - strlen(titulo)) / 2;
        mvprintw(y, pos, " %s ", titulo);
    }
}

/* ==================== DESENHO DAS AREAS ==================== */

void display_titulo(int linha) {
    attron(COLOR_PAIR(COR_TITULO) | A_BOLD);

    mvprintw(linha, 2, "  _  __                  ____        _       _             ");
    mvprintw(linha + 1, 2, " | |/ /___  ___ _ __   / ___|  ___ | |_   _(_)_ __   __ _ ");
    mvprintw(linha + 2, 2, " | ' // _ \\/ _ \\ '_ \\  \\___ \\ / _ \\| \\ \\ / / | '_ \\ / _` |");
    mvprintw(linha + 3, 2, " | . \\  __/  __/ |_) |  ___) | (_) | |\\ V /| | | | | (_| |");
    mvprintw(linha + 4, 2, " |_|\\_\\___|\\___|  __/  |____/ \\___/|_| \\_/ |_|_| |_|\\__, |");
    mvprintw(linha + 5, 2, "              |_|                                   |___/ ");

    attroff(COLOR_PAIR(COR_TITULO) | A_BOLD);

    attron(COLOR_PAIR(COR_ALERTA));
    mvprintw(linha + 6, 2, "        *** AND NOBODY EXPLODES - Versao de Treino ***");
    attroff(COLOR_PAIR(COR_ALERTA));
}

void display_modulos_pendentes(EstadoJogoCompleto* estado, int linha) {
    if (!estado) return;

    int largura = COLS - 4;
    desenhar_caixa(linha, 2, ALTURA_MODULOS, largura, "MODULOS PENDENTES - Copie a INSTRUCAO para seu comando!");

    pthread_mutex_lock(&estado->fila_modulos.mutex);
    int qtd = estado->fila_modulos.quantidade;

    if (qtd == 0) {
        attron(COLOR_PAIR(COR_SUCESSO) | A_BOLD);
        mvprintw(linha + 2, 4, "Nenhum modulo pendente! Aguarde novos modulos...");
        attroff(COLOR_PAIR(COR_SUCESSO) | A_BOLD);
    } else {
        /* Cabecalho explicativo */
        attron(COLOR_PAIR(COR_INFO));
        mvprintw(linha + 1, 4, "ID   TIPO  INSTRUCAO (copie!)");
        attroff(COLOR_PAIR(COR_INFO));

        int lin = linha + 2;
        int col = 4;
        int mostrados = 0;

        for (int i = 0; i < qtd && mostrados < 6; i++) {
            int idx = (estado->fila_modulos.inicio + i) % MAX_MODULOS_PENDENTES;
            Modulo* m = &estado->fila_modulos.modulos[idx];

            /* ID em destaque */
            attron(COLOR_PAIR(COR_ALERTA) | A_BOLD);
            mvprintw(lin, col, "[%d]", m->id);
            attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD);

            /* Tipo */
            attron(COLOR_PAIR(COR_SUCESSO));
            mvprintw(lin, col + 5, "%c:", char_tipo_modulo(m->tipo));
            attroff(COLOR_PAIR(COR_SUCESSO));

            /* Instrucao em destaque */
            attron(COLOR_PAIR(COR_ERRO) | A_BOLD);
            mvprintw(lin, col + 8, "%s", m->instrucao);
            attroff(COLOR_PAIR(COR_ERRO) | A_BOLD);

            col += 22;
            mostrados++;
            if (mostrados % 4 == 0) {
                col = 4;
                lin++;
            }
        }

        if (qtd > 6) {
            attron(COLOR_PAIR(COR_ALERTA));
            mvprintw(linha + ALTURA_MODULOS - 2, largura - 20, "... e mais %d", qtd - 6);
            attroff(COLOR_PAIR(COR_ALERTA));
        }
    }

    pthread_mutex_unlock(&estado->fila_modulos.mutex);

    /* Mostra quantidade */
    attron(COLOR_PAIR(qtd >= MAX_MODULOS_PENDENTES - 2 ? COR_ERRO : COR_INFO) | A_BOLD);
    mvprintw(linha, largura - 10, " [%d/%d] ", qtd, MAX_MODULOS_PENDENTES);
    attroff(COLOR_PAIR(qtd >= MAX_MODULOS_PENDENTES - 2 ? COR_ERRO : COR_INFO) | A_BOLD);
}

void display_bancadas(EstadoJogoCompleto* estado, int linha) {
    if (!estado) return;

    int largura = COLS - 4;
    desenhar_caixa(linha, 2, ALTURA_BANCADAS, largura, "BANCADAS");

    int num_bancadas = estado->config.num_bancadas;
    int largura_bancada = (largura - 6) / num_bancadas;

    for (int i = 0; i < num_bancadas; i++) {
        Bancada* b = &estado->bancadas[i];
        int x = 4 + i * largura_bancada;

        pthread_mutex_lock(&b->mutex);

        /* Desenha caixa da bancada */
        attron(COLOR_PAIR(COR_BANCADA));
        mvprintw(linha + 1, x, "Bancada %d", i + 1);
        attroff(COLOR_PAIR(COR_BANCADA));

        if (b->estado == ESTADO_LIVRE) {
            attron(COLOR_PAIR(COR_SUCESSO));
            mvprintw(linha + 2, x, "[LIVRE]");
            attroff(COLOR_PAIR(COR_SUCESSO));
        } else {
            attron(COLOR_PAIR(COR_ERRO));
            mvprintw(linha + 2, x, "[OCUPADA]");
            attroff(COLOR_PAIR(COR_ERRO));

            if (b->modulo_atual) {
                mvprintw(linha + 3, x, "Modulo: %s", b->modulo_atual->nome);
            }
            mvprintw(linha + 4, x, "Tedax: %d", b->tedax_id + 1);
        }

        pthread_mutex_unlock(&b->mutex);
    }
}

void display_tedax(EstadoJogoCompleto* estado, int linha) {
    if (!estado) return;

    int largura = COLS - 4;
    desenhar_caixa(linha, 2, ALTURA_TEDAX, largura, "TEDAX");

    int num_tedax = estado->config.num_tedax;
    int largura_tedax = (largura - 6) / num_tedax;

    for (int i = 0; i < num_tedax; i++) {
        Tedax* t = &estado->tedax[i];
        int x = 4 + i * largura_tedax;

        pthread_mutex_lock(&t->mutex);

        attron(COLOR_PAIR(COR_INFO) | A_BOLD);
        mvprintw(linha + 1, x, "Tedax %d", i + 1);
        attroff(COLOR_PAIR(COR_INFO) | A_BOLD);

        if (t->estado == ESTADO_LIVRE) {
            attron(COLOR_PAIR(COR_TEDAX_LIVRE));
            mvprintw(linha + 2, x, "[DISPONIVEL]");
            attroff(COLOR_PAIR(COR_TEDAX_LIVRE));
        } else if (t->estado == ESTADO_AGUARDANDO_BANCADA) {
            attron(COLOR_PAIR(COR_ALERTA));
            mvprintw(linha + 2, x, "[AGUARDANDO]");
            attroff(COLOR_PAIR(COR_ALERTA));
        } else {
            attron(COLOR_PAIR(COR_TEDAX_OCUP));
            mvprintw(linha + 2, x, "[TRABALHANDO]");
            attroff(COLOR_PAIR(COR_TEDAX_OCUP));
        }

        mvprintw(linha + 3, x, "OK: %d  Falha: %d",
                t->modulos_desarmados, t->modulos_falhados);

        pthread_mutex_unlock(&t->mutex);
    }
}

void display_status(EstadoJogoCompleto* estado, int linha) {
    if (!estado) return;

    int largura = COLS - 4;
    desenhar_caixa(linha, 2, ALTURA_STATUS, largura, "STATUS");

    pthread_mutex_lock(&estado->mutex_estado);

    /* Tempo restante */
    char tempo_str[16];
    formatar_tempo(estado->stats.tempo_restante, tempo_str, sizeof(tempo_str));

    int cor_tempo = COR_SUCESSO;
    if (estado->stats.tempo_restante < 30) cor_tempo = COR_ALERTA;
    if (estado->stats.tempo_restante < 10) cor_tempo = COR_ERRO;

    attron(COLOR_PAIR(cor_tempo) | A_BOLD);
    mvprintw(linha + 1, 4, "TEMPO: %s", tempo_str);
    attroff(COLOR_PAIR(cor_tempo) | A_BOLD);

    /* Estatisticas */
    mvprintw(linha + 1, 30, "Gerados: %d", estado->stats.modulos_gerados);
    mvprintw(linha + 1, 50, "Desarmados: %d", estado->stats.modulos_desarmados);
    mvprintw(linha + 1, 75, "Falhas: %d", estado->stats.modulos_falhados);

    /* Objetivo */
    if (!estado->config.modo_infinito) {
        attron(COLOR_PAIR(COR_INFO));
        mvprintw(linha + 2, 4, "Objetivo: Desarmar %d modulos",
                estado->config.modulos_para_vencer);
        attroff(COLOR_PAIR(COR_INFO));
    }

    /* Mensagem de feedback */
    if (strlen(estado->mensagem_feedback) > 0) {
        time_t agora = time(NULL);
        if (agora - estado->tempo_mensagem < 5) {
            attron(COLOR_PAIR(COR_ALERTA));
            mvprintw(linha + 2, 35, ">> %s", estado->mensagem_feedback);
            attroff(COLOR_PAIR(COR_ALERTA));
        }
    }

    pthread_mutex_unlock(&estado->mutex_estado);
}

void display_comando(EstadoJogoCompleto* estado, int linha) {
    if (!estado) return;

    int largura = COLS - 4;
    desenhar_caixa(linha, 2, ALTURA_COMANDO + 2, largura, "DIGITE SEU COMANDO");

    pthread_mutex_lock(&estado->mutex_comando);
    char buffer[32];
    strncpy(buffer, estado->buffer_comando, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    pthread_mutex_unlock(&estado->mutex_comando);

    /* Instrucoes de como jogar */
    attron(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha + 1, 4, "FORMATO: [Tedax 1-3][Tipo f/b/s/i][Bancada 1-3][Instrucao da tela]");
    attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD);

    attron(COLOR_PAIR(COR_INFO));
    mvprintw(linha + 2, 4, "EXEMPLO: Se aparece [1] f: rgb -> digite: 1f1rgb e pressione ENTER");
    attroff(COLOR_PAIR(COR_INFO));

    /* Campo de entrada */
    attron(COLOR_PAIR(COR_DESTAQUE) | A_BOLD);
    mvprintw(linha + 3, 4, ">>> %s_", buffer);
    attroff(COLOR_PAIR(COR_DESTAQUE) | A_BOLD);

    /* Teclas de atalho */
    attron(COLOR_PAIR(COR_PADRAO));
    mvprintw(linha + 4, 4, "TECLAS: ENTER=enviar | BACKSPACE=apagar | p=pausar | h=ajuda | q=sair");
    attroff(COLOR_PAIR(COR_PADRAO));
}

/* ==================== TELAS ESPECIAIS ==================== */

void display_menu(int opcao_selecionada) {
    clear();

    display_titulo(2);

    int linha_menu = 12;
    const char* opcoes[] = {
        "1. Iniciar Jogo",
        "2. Configuracoes",
        "3. Ajuda",
        "4. Sair"
    };

    for (int i = 0; i < 4; i++) {
        if (i == opcao_selecionada) {
            attron(COLOR_PAIR(COR_DESTAQUE) | A_BOLD);
        } else {
            attron(COLOR_PAIR(COR_PADRAO));
        }
        mvprintw(linha_menu + i * 2, (COLS - 20) / 2, "%s", opcoes[i]);
        attroff(COLOR_PAIR(COR_DESTAQUE) | A_BOLD | COLOR_PAIR(COR_PADRAO));
    }

    attron(COLOR_PAIR(COR_INFO));
    mvprintw(LINES - 2, 2, "Use SETAS ou NUMEROS para selecionar, ENTER para confirmar");
    attroff(COLOR_PAIR(COR_INFO));

    refresh();
}

void display_configuracao(ConfigJogo* config, int campo_selecionado) {
    if (!config) return;

    clear();

    attron(COLOR_PAIR(COR_TITULO) | A_BOLD);
    mvprintw(2, (COLS - 20) / 2, "=== CONFIGURACOES ===");
    attroff(COLOR_PAIR(COR_TITULO) | A_BOLD);

    int linha = 6;
    const char* campos[] = {
        "Numero de Tedax",
        "Numero de Bancadas",
        "Tempo da Partida (seg)",
        "Dificuldade",
        "Modulos para Vencer",
        "Modo Infinito"
    };
    int valores[] = {
        config->num_tedax,
        config->num_bancadas,
        config->tempo_partida,
        config->dificuldade,
        config->modulos_para_vencer,
        config->modo_infinito
    };

    for (int i = 0; i < 6; i++) {
        if (i == campo_selecionado) {
            attron(COLOR_PAIR(COR_DESTAQUE) | A_BOLD);
        }

        if (i == 5) {
            mvprintw(linha + i * 2, 10, "%-25s: %s",
                    campos[i], valores[i] ? "SIM" : "NAO");
        } else {
            mvprintw(linha + i * 2, 10, "%-25s: %d", campos[i], valores[i]);
        }

        if (i == campo_selecionado) {
            printw("  <- / ->");
            attroff(COLOR_PAIR(COR_DESTAQUE) | A_BOLD);
        }
    }

    /* Botao Voltar */
    if (campo_selecionado == 6) {
        attron(COLOR_PAIR(COR_DESTAQUE) | A_BOLD);
    }
    mvprintw(linha + 14, (COLS - 10) / 2, "[ VOLTAR ]");
    attroff(COLOR_PAIR(COR_DESTAQUE) | A_BOLD);

    attron(COLOR_PAIR(COR_INFO));
    mvprintw(LINES - 2, 2, "SETAS: navegar | ESQUERDA/DIREITA: alterar | ENTER: confirmar | ESC: voltar");
    attroff(COLOR_PAIR(COR_INFO));

    refresh();
}

void display_fim_jogo(EstadoJogoCompleto* estado) {
    if (!estado) return;

    clear();

    pthread_mutex_lock(&estado->mutex_estado);
    EstadoJogo est = estado->estado;

    int linha = 8;

    if (est == JOGO_VITORIA) {
        attron(COLOR_PAIR(COR_SUCESSO) | A_BOLD);
        mvprintw(linha, (COLS - 30) / 2, "*** PARABENS! VOCE VENCEU! ***");
        attroff(COLOR_PAIR(COR_SUCESSO) | A_BOLD);
    } else {
        attron(COLOR_PAIR(COR_ERRO) | A_BOLD);
        mvprintw(linha, (COLS - 30) / 2, "*** BOOM! A BOMBA EXPLODIU! ***");
        attroff(COLOR_PAIR(COR_ERRO) | A_BOLD);
    }

    linha += 4;
    mvprintw(linha++, 20, "=== ESTATISTICAS ===");
    mvprintw(linha++, 20, "Modulos gerados:    %d", estado->stats.modulos_gerados);
    mvprintw(linha++, 20, "Modulos desarmados: %d", estado->stats.modulos_desarmados);
    mvprintw(linha++, 20, "Falhas:             %d", estado->stats.modulos_falhados);

    /* Estatisticas por tedax */
    linha += 2;
    mvprintw(linha++, 20, "=== DESEMPENHO DOS TEDAX ===");
    for (int i = 0; i < estado->config.num_tedax; i++) {
        mvprintw(linha++, 20, "Tedax %d: %d desarmados, %d falhas",
                i + 1,
                estado->tedax[i].modulos_desarmados,
                estado->tedax[i].modulos_falhados);
    }

    pthread_mutex_unlock(&estado->mutex_estado);

    attron(COLOR_PAIR(COR_INFO));
    mvprintw(LINES - 2, (COLS - 30) / 2, "Pressione qualquer tecla...");
    attroff(COLOR_PAIR(COR_INFO));

    refresh();
}

void display_ajuda(void) {
    clear();

    attron(COLOR_PAIR(COR_TITULO) | A_BOLD);
    mvprintw(2, (COLS - 10) / 2, "=== AJUDA ===");
    attroff(COLOR_PAIR(COR_TITULO) | A_BOLD);

    int linha = 5;

    attron(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "OBJETIVO:");
    attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "Desarme todos os modulos da bomba antes do tempo acabar!");
    linha++;

    attron(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "COMO JOGAR:");
    attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "Digite comandos no formato: [TEDAX][TIPO][BANCADA][INSTRUCAO]");
    mvprintw(linha++, 4, "Exemplo: 1f1rgb = Tedax 1, modulo Fios, Bancada 1, instrucao 'rgb'");
    linha++;

    attron(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "TIPOS DE MODULOS:");
    attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "  f = Fios    - Corte os fios na ordem correta (r/g/b/y)");
    mvprintw(linha++, 4, "  b = Botao   - Pressione o botao N vezes (pppp...)");
    mvprintw(linha++, 4, "  s = Sequencia - Digite a sequencia de numeros (1234...)");
    mvprintw(linha++, 4, "  i = Simon   - Repita a sequencia de direcoes (u/d/l/r)");
    linha++;

    attron(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "TECLAS:");
    attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD);
    mvprintw(linha++, 4, "  ENTER = Executar comando");
    mvprintw(linha++, 4, "  BACKSPACE = Apagar caractere");
    mvprintw(linha++, 4, "  ESC = Limpar comando");
    mvprintw(linha++, 4, "  p = Pausar/Despausar");
    mvprintw(linha++, 4, "  q = Sair do jogo");
    mvprintw(linha++, 4, "  h = Esta ajuda");

    attron(COLOR_PAIR(COR_INFO));
    mvprintw(LINES - 2, (COLS - 30) / 2, "Pressione qualquer tecla...");
    attroff(COLOR_PAIR(COR_INFO));

    refresh();
}

/* ==================== ATUALIZACAO PRINCIPAL ==================== */

void display_atualizar(EstadoJogoCompleto* estado) {
    if (!estado) return;

    pthread_mutex_lock(&estado->mutex_display);

    clear();

    int linha = 0;

    /* Titulo compacto */
    attron(COLOR_PAIR(COR_TITULO) | A_BOLD);
    mvprintw(linha, (COLS - 50) / 2, "KEEP SOLVING AND NOBODY EXPLODES - Versao de Treino");
    attroff(COLOR_PAIR(COR_TITULO) | A_BOLD);
    linha += 2;

    /* Areas do jogo */
    display_modulos_pendentes(estado, linha);
    linha += ALTURA_MODULOS + 1;

    display_bancadas(estado, linha);
    linha += ALTURA_BANCADAS + 1;

    display_tedax(estado, linha);
    linha += ALTURA_TEDAX + 1;

    display_status(estado, linha);
    linha += ALTURA_STATUS + 1;

    display_comando(estado, linha);

    refresh();

    pthread_mutex_unlock(&estado->mutex_display);
}

void display_mensagem(EstadoJogoCompleto* estado, const char* mensagem, int tipo) {
    (void)tipo; /* Evita warning - pode ser usado para cores no futuro */
    if (!estado || !mensagem) return;

    pthread_mutex_lock(&estado->mutex_estado);
    strncpy(estado->mensagem_feedback, mensagem, sizeof(estado->mensagem_feedback) - 1);
    estado->tempo_mensagem = time(NULL);
    pthread_mutex_unlock(&estado->mutex_estado);
}

/* ==================== THREAD DE DISPLAY ==================== */

void* thread_display(void* arg) {
    EstadoJogoCompleto* estado = (EstadoJogoCompleto*)arg;
    if (!estado) return NULL;

    while (estado->executando) {
        pthread_mutex_lock(&estado->mutex_estado);
        EstadoJogo est = estado->estado;
        pthread_mutex_unlock(&estado->mutex_estado);

        if (est == JOGO_RODANDO || est == JOGO_PAUSADO) {
            display_atualizar(estado);

            if (est == JOGO_PAUSADO) {
                pthread_mutex_lock(&estado->mutex_display);
                attron(COLOR_PAIR(COR_ALERTA) | A_BOLD | A_BLINK);
                mvprintw(LINES / 2, (COLS - 20) / 2, "*** JOGO PAUSADO ***");
                attroff(COLOR_PAIR(COR_ALERTA) | A_BOLD | A_BLINK);
                refresh();
                pthread_mutex_unlock(&estado->mutex_display);
            }
        }

        usleep(100000); /* Atualiza a cada 100ms */
    }

    return NULL;
}
