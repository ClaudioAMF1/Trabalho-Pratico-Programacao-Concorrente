/**
 * @file tedax.h
 * @brief Declaracoes para gerenciamento de Tedax
 *
 * Tedax = Tecnico Especialista em Desativacao de Artefatos Explosivos
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#ifndef TEDAX_H
#define TEDAX_H

#include "tipos.h"

/**
 * @brief Inicializa um tedax
 * @param tedax Ponteiro para o tedax
 * @param id ID do tedax
 */
void tedax_init(Tedax* tedax, int id);

/**
 * @brief Destroi um tedax (libera recursos)
 * @param tedax Ponteiro para o tedax
 */
void tedax_destroy(Tedax* tedax);

/**
 * @brief Inicia a thread do tedax
 * @param tedax Ponteiro para o tedax
 * @return 0 se sucesso, -1 se erro
 */
int tedax_iniciar_thread(Tedax* tedax);

/**
 * @brief Para a thread do tedax
 * @param tedax Ponteiro para o tedax
 */
void tedax_parar_thread(Tedax* tedax);

/**
 * @brief Verifica se o tedax esta disponivel
 * @param tedax Ponteiro para o tedax
 * @return true se disponivel
 */
bool tedax_disponivel(Tedax* tedax);

/**
 * @brief Designa um modulo para o tedax
 * @param tedax Ponteiro para o tedax
 * @param modulo Ponteiro para o modulo
 * @param bancada_id ID da bancada a usar
 * @param instrucao Instrucao do coordenador
 * @return true se designado com sucesso
 */
bool tedax_designar_modulo(Tedax* tedax, Modulo* modulo, int bancada_id, const char* instrucao);

/**
 * @brief Retorna o estado atual do tedax como string
 * @param tedax Ponteiro para o tedax
 * @return String descrevendo o estado
 */
const char* tedax_estado_str(Tedax* tedax);

/**
 * @brief Thread principal do tedax
 * @param arg Ponteiro para o Tedax
 * @return NULL
 */
void* thread_tedax(void* arg);

/**
 * @brief Processa a resolucao de um modulo
 * @param tedax Ponteiro para o tedax
 * @param modulo Ponteiro para o modulo
 * @param instrucao Instrucao fornecida
 * @return true se resolvido com sucesso
 */
bool tedax_resolver_modulo(Tedax* tedax, Modulo* modulo, const char* instrucao);

/**
 * @brief Verifica se a instrucao esta correta para o modulo
 * @param modulo Ponteiro para o modulo
 * @param instrucao Instrucao fornecida
 * @return true se correta
 */
bool verificar_instrucao(Modulo* modulo, const char* instrucao);

#endif /* TEDAX_H */
