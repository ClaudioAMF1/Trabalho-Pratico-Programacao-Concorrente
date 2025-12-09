/**
 * @file bancada.h
 * @brief Declaracoes para gerenciamento de bancadas de desativacao
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#ifndef BANCADA_H
#define BANCADA_H

#include "tipos.h"

/**
 * @brief Inicializa uma bancada
 * @param bancada Ponteiro para a bancada
 * @param id ID da bancada
 */
void bancada_init(Bancada* bancada, int id);

/**
 * @brief Destroi uma bancada (libera recursos)
 * @param bancada Ponteiro para a bancada
 */
void bancada_destroy(Bancada* bancada);

/**
 * @brief Verifica se a bancada esta livre
 * @param bancada Ponteiro para a bancada
 * @return true se livre
 */
bool bancada_livre(Bancada* bancada);

/**
 * @brief Tenta ocupar uma bancada
 * @param bancada Ponteiro para a bancada
 * @param tedax_id ID do tedax que quer ocupar
 * @param modulo Modulo a ser trabalhado
 * @return true se conseguiu ocupar
 */
bool bancada_ocupar(Bancada* bancada, int tedax_id, Modulo* modulo);

/**
 * @brief Libera uma bancada
 * @param bancada Ponteiro para a bancada
 * @param tedax_id ID do tedax que esta liberando
 * @return true se liberou com sucesso
 */
bool bancada_liberar(Bancada* bancada, int tedax_id);

/**
 * @brief Aguarda a bancada ficar livre (bloqueante)
 * @param bancada Ponteiro para a bancada
 * @param timeout_ms Timeout em milissegundos (0 = sem timeout)
 * @return true se ficou livre, false se timeout
 */
bool bancada_aguardar_livre(Bancada* bancada, int timeout_ms);

/**
 * @brief Retorna o estado atual da bancada como string
 * @param bancada Ponteiro para a bancada
 * @return String descrevendo o estado
 */
const char* bancada_estado_str(Bancada* bancada);

/**
 * @brief Encontra uma bancada livre
 * @param bancadas Array de bancadas
 * @param num_bancadas Numero de bancadas
 * @return ID da bancada livre ou -1 se nenhuma
 */
int encontrar_bancada_livre(Bancada* bancadas, int num_bancadas);

/**
 * @brief Conta bancadas ocupadas
 * @param bancadas Array de bancadas
 * @param num_bancadas Numero de bancadas
 * @return Quantidade de bancadas ocupadas
 */
int contar_bancadas_ocupadas(Bancada* bancadas, int num_bancadas);

#endif /* BANCADA_H */
