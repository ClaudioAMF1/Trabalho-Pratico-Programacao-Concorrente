/**
 * @file display.h
 * @brief Declaracoes para exibicao de informacoes (ncurses)
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "tipos.h"
#include <ncurses.h>

/* Pares de cores */
#define COR_PADRAO      1
#define COR_TITULO      2
#define COR_SUCESSO     3
#define COR_ERRO        4
#define COR_ALERTA      5
#define COR_INFO        6
#define COR_DESTAQUE    7
#define COR_TEDAX_LIVRE 8
#define COR_TEDAX_OCUP  9
#define COR_BANCADA     10
#define COR_MODULO      11
#define COR_TEMPO       12

/* Dimensoes das areas */
#define ALTURA_TITULO    3
#define ALTURA_MODULOS   8
#define ALTURA_BANCADAS  6
#define ALTURA_TEDAX     6
#define ALTURA_STATUS    4
#define ALTURA_COMANDO   3

/**
 * @brief Inicializa o sistema de display (ncurses)
 * @return 0 se sucesso, -1 se erro
 */
int display_init(void);

/**
 * @brief Finaliza o sistema de display
 */
void display_finalizar(void);

/**
 * @brief Inicializa os pares de cores
 */
void display_init_cores(void);

/**
 * @brief Limpa a tela completamente
 */
void display_limpar(void);

/**
 * @brief Atualiza toda a interface do jogo
 * @param estado Ponteiro para o estado do jogo
 */
void display_atualizar(EstadoJogoCompleto* estado);

/**
 * @brief Desenha o titulo do jogo
 * @param linha Linha inicial
 */
void display_titulo(int linha);

/**
 * @brief Desenha a area de modulos pendentes
 * @param estado Ponteiro para o estado do jogo
 * @param linha Linha inicial
 */
void display_modulos_pendentes(EstadoJogoCompleto* estado, int linha);

/**
 * @brief Desenha a area das bancadas
 * @param estado Ponteiro para o estado do jogo
 * @param linha Linha inicial
 */
void display_bancadas(EstadoJogoCompleto* estado, int linha);

/**
 * @brief Desenha a area dos tedax
 * @param estado Ponteiro para o estado do jogo
 * @param linha Linha inicial
 */
void display_tedax(EstadoJogoCompleto* estado, int linha);

/**
 * @brief Desenha a barra de status
 * @param estado Ponteiro para o estado do jogo
 * @param linha Linha inicial
 */
void display_status(EstadoJogoCompleto* estado, int linha);

/**
 * @brief Desenha a area de entrada de comandos
 * @param estado Ponteiro para o estado do jogo
 * @param linha Linha inicial
 */
void display_comando(EstadoJogoCompleto* estado, int linha);

/**
 * @brief Exibe uma mensagem de feedback temporaria
 * @param estado Ponteiro para o estado do jogo
 * @param mensagem Mensagem a exibir
 * @param tipo 0=info, 1=sucesso, 2=erro, 3=alerta
 */
void display_mensagem(EstadoJogoCompleto* estado, const char* mensagem, int tipo);

/**
 * @brief Exibe a tela do menu inicial
 * @param opcao_selecionada Opcao atualmente selecionada
 */
void display_menu(int opcao_selecionada);

/**
 * @brief Exibe a tela de configuracao
 * @param config Ponteiro para configuracoes atuais
 * @param campo_selecionado Campo atualmente selecionado
 */
void display_configuracao(ConfigJogo* config, int campo_selecionado);

/**
 * @brief Exibe a tela de fim de jogo
 * @param estado Ponteiro para o estado do jogo
 */
void display_fim_jogo(EstadoJogoCompleto* estado);

/**
 * @brief Exibe a tela de ajuda
 */
void display_ajuda(void);

/**
 * @brief Thread principal de display
 * @param arg Ponteiro para EstadoJogoCompleto
 * @return NULL
 */
void* thread_display(void* arg);

/**
 * @brief Desenha uma caixa com borda
 * @param y Linha inicial
 * @param x Coluna inicial
 * @param altura Altura da caixa
 * @param largura Largura da caixa
 * @param titulo Titulo da caixa (pode ser NULL)
 */
void desenhar_caixa(int y, int x, int altura, int largura, const char* titulo);

/**
 * @brief Formata tempo em mm:ss
 * @param segundos Tempo em segundos
 * @param buffer Buffer para o resultado
 * @param tamanho Tamanho do buffer
 */
void formatar_tempo(int segundos, char* buffer, int tamanho);

#endif /* DISPLAY_H */
