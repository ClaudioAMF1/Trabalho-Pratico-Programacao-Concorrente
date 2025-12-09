/**
 * @file jogo.h
 * @brief Declaracoes para controle geral do jogo
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#ifndef JOGO_H
#define JOGO_H

#include "tipos.h"

/**
 * @brief Inicializa o estado do jogo
 * @param estado Ponteiro para o estado
 * @param config Configuracoes iniciais
 * @return 0 se sucesso, -1 se erro
 */
int jogo_init(EstadoJogoCompleto* estado, ConfigJogo* config);

/**
 * @brief Finaliza o jogo (libera recursos)
 * @param estado Ponteiro para o estado
 */
void jogo_finalizar(EstadoJogoCompleto* estado);

/**
 * @brief Inicia uma nova partida
 * @param estado Ponteiro para o estado
 * @return 0 se sucesso, -1 se erro
 */
int jogo_iniciar_partida(EstadoJogoCompleto* estado);

/**
 * @brief Para a partida atual
 * @param estado Ponteiro para o estado
 */
void jogo_parar_partida(EstadoJogoCompleto* estado);

/**
 * @brief Pausa ou despausa a partida
 * @param estado Ponteiro para o estado
 */
void jogo_pausar(EstadoJogoCompleto* estado);

/**
 * @brief Verifica condicoes de fim de jogo
 * @param estado Ponteiro para o estado
 * @return true se o jogo terminou
 */
bool jogo_verificar_fim(EstadoJogoCompleto* estado);

/**
 * @brief Processa um comando do jogador
 * @param estado Ponteiro para o estado
 * @param comando String com o comando
 * @return true se comando valido
 */
bool jogo_processar_comando(EstadoJogoCompleto* estado, const char* comando);

/**
 * @brief Adiciona um caractere ao buffer de comando
 * @param estado Ponteiro para o estado
 * @param c Caractere a adicionar
 */
void jogo_adicionar_char_comando(EstadoJogoCompleto* estado, char c);

/**
 * @brief Remove ultimo caractere do buffer de comando
 * @param estado Ponteiro para o estado
 */
void jogo_remover_char_comando(EstadoJogoCompleto* estado);

/**
 * @brief Limpa o buffer de comando
 * @param estado Ponteiro para o estado
 */
void jogo_limpar_comando(EstadoJogoCompleto* estado);

/**
 * @brief Executa o comando no buffer
 * @param estado Ponteiro para o estado
 * @return true se executado com sucesso
 */
bool jogo_executar_comando(EstadoJogoCompleto* estado);

/**
 * @brief Thread do coordenador (input do jogador)
 * @param arg Ponteiro para EstadoJogoCompleto
 * @return NULL
 */
void* thread_coordenador(void* arg);

/**
 * @brief Thread do temporizador
 * @param arg Ponteiro para EstadoJogoCompleto
 * @return NULL
 */
void* thread_timer(void* arg);

/**
 * @brief Retorna configuracoes padrao
 * @return ConfigJogo com valores padrao
 */
ConfigJogo config_padrao(void);

/**
 * @brief Obtem o estado atual de forma segura
 * @param estado Ponteiro para o estado
 * @return Copia do estado atual
 */
EstadoJogo jogo_obter_estado(EstadoJogoCompleto* estado);

/**
 * @brief Define o estado do jogo de forma segura
 * @param estado Ponteiro para o estado
 * @param novo_estado Novo estado
 */
void jogo_definir_estado(EstadoJogoCompleto* estado, EstadoJogo novo_estado);

/**
 * @brief Adiciona uma mensagem de feedback
 * @param estado Ponteiro para o estado
 * @param formato Formato printf
 * @param ... Argumentos
 */
void jogo_feedback(EstadoJogoCompleto* estado, const char* formato, ...);

#endif /* JOGO_H */
