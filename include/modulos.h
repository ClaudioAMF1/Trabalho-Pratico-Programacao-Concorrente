/**
 * @file modulos.h
 * @brief Declaracoes para gerenciamento de modulos
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#ifndef MODULOS_H
#define MODULOS_H

#include "tipos.h"

/**
 * @brief Inicializa a fila de modulos
 * @param fila Ponteiro para a fila
 */
void fila_modulos_init(FilaModulos* fila);

/**
 * @brief Destroi a fila de modulos (libera recursos)
 * @param fila Ponteiro para a fila
 */
void fila_modulos_destroy(FilaModulos* fila);

/**
 * @brief Adiciona um modulo a fila
 * @param fila Ponteiro para a fila
 * @param modulo Modulo a ser adicionado
 * @return true se adicionado com sucesso, false se fila cheia
 */
bool fila_modulos_adicionar(FilaModulos* fila, Modulo* modulo);

/**
 * @brief Remove um modulo da fila
 * @param fila Ponteiro para a fila
 * @param modulo Ponteiro para armazenar o modulo removido
 * @return true se removido com sucesso, false se fila vazia
 */
bool fila_modulos_remover(FilaModulos* fila, Modulo* modulo);

/**
 * @brief Remove um modulo especifico da fila pelo ID
 * @param fila Ponteiro para a fila
 * @param id ID do modulo a remover
 * @param modulo Ponteiro para armazenar o modulo removido
 * @return true se encontrado e removido, false caso contrario
 */
bool fila_modulos_remover_por_id(FilaModulos* fila, int id, Modulo* modulo);

/**
 * @brief Retorna um modulo especifico da fila pelo indice (0-based)
 * @param fila Ponteiro para a fila
 * @param indice Indice do modulo na fila
 * @param modulo Ponteiro para armazenar o modulo
 * @return true se encontrado, false caso contrario
 */
bool fila_modulos_obter(FilaModulos* fila, int indice, Modulo* modulo);

/**
 * @brief Verifica se a fila esta vazia
 * @param fila Ponteiro para a fila
 * @return true se vazia
 */
bool fila_modulos_vazia(FilaModulos* fila);

/**
 * @brief Verifica se a fila esta cheia
 * @param fila Ponteiro para a fila
 * @return true se cheia
 */
bool fila_modulos_cheia(FilaModulos* fila);

/**
 * @brief Retorna a quantidade de modulos na fila
 * @param fila Ponteiro para a fila
 * @return Quantidade de modulos
 */
int fila_modulos_quantidade(FilaModulos* fila);

/**
 * @brief Gera um novo modulo aleatorio
 * @param id ID para o novo modulo
 * @param dificuldade Nivel de dificuldade (1-3)
 * @return Modulo gerado
 */
Modulo gerar_modulo_aleatorio(int id, int dificuldade);

/**
 * @brief Retorna o nome do tipo de modulo
 * @param tipo Tipo do modulo
 * @return String com o nome
 */
const char* nome_tipo_modulo(TipoModulo tipo);

/**
 * @brief Retorna o caractere identificador do tipo
 * @param tipo Tipo do modulo
 * @return Caractere identificador
 */
char char_tipo_modulo(TipoModulo tipo);

/**
 * @brief Retorna o tipo pelo caractere identificador
 * @param c Caractere
 * @return Tipo do modulo ou -1 se invalido
 */
TipoModulo tipo_modulo_por_char(char c);

/**
 * @brief Thread do mural de modulos pendentes
 * @param arg Ponteiro para EstadoJogoCompleto
 * @return NULL
 */
void* thread_mural_modulos(void* arg);

#endif /* MODULOS_H */
