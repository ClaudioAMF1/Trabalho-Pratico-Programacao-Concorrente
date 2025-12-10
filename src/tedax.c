/*
 * tedax.c - Thread do tecnico que desarma modulos nas bancadas
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#include "../include/tedax.h"
#include "../include/bancada.h"
#include "../include/modulos.h"
#include "../include/jogo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Referencia global ao estado do jogo */
extern EstadoJogoCompleto* jogo;

void tedax_init(Tedax* tedax, int id) {
    if (!tedax) return;

    tedax->id = id;
    tedax->estado = ESTADO_LIVRE;
    tedax->modulo_atual = NULL;
    tedax->bancada_atual = NULL;
    tedax->modulos_desarmados = 0;
    tedax->modulos_falhados = 0;
    tedax->ativo = false;
    tedax->bancada_designada = -1;
    tedax->tarefa_pendente = false;
    memset(tedax->instrucao_recebida, 0, MAX_INSTRUCAO);

    pthread_mutex_init(&tedax->mutex, NULL);
    pthread_cond_init(&tedax->cond_tarefa, NULL);
}

void tedax_destroy(Tedax* tedax) {
    if (!tedax) return;

    tedax_parar_thread(tedax);

    pthread_mutex_destroy(&tedax->mutex);
    pthread_cond_destroy(&tedax->cond_tarefa);
}

int tedax_iniciar_thread(Tedax* tedax) {
    if (!tedax) return -1;

    tedax->ativo = true;
    if (pthread_create(&tedax->thread, NULL, thread_tedax, tedax) != 0) {
        tedax->ativo = false;
        return -1;
    }

    return 0;
}

void tedax_parar_thread(Tedax* tedax) {
    if (!tedax || !tedax->ativo) return;

    pthread_mutex_lock(&tedax->mutex);
    tedax->ativo = false;
    pthread_cond_signal(&tedax->cond_tarefa);
    pthread_mutex_unlock(&tedax->mutex);

    pthread_join(tedax->thread, NULL);
}

bool tedax_disponivel(Tedax* tedax) {
    if (!tedax) return false;

    pthread_mutex_lock(&tedax->mutex);
    bool disp = (tedax->estado == ESTADO_LIVRE && !tedax->tarefa_pendente);
    pthread_mutex_unlock(&tedax->mutex);

    return disp;
}

bool tedax_designar_modulo(Tedax* tedax, Modulo* modulo, int bancada_id, const char* instrucao) {
    if (!tedax || !modulo || !instrucao) return false;

    pthread_mutex_lock(&tedax->mutex);

    /* Verifica se o tedax esta disponivel */
    if (tedax->estado != ESTADO_LIVRE || tedax->tarefa_pendente) {
        pthread_mutex_unlock(&tedax->mutex);
        return false;
    }

    /* Aloca memoria para o modulo e copia */
    tedax->modulo_atual = malloc(sizeof(Modulo));
    if (!tedax->modulo_atual) {
        pthread_mutex_unlock(&tedax->mutex);
        return false;
    }
    memcpy(tedax->modulo_atual, modulo, sizeof(Modulo));

    tedax->bancada_designada = bancada_id;
    strncpy(tedax->instrucao_recebida, instrucao, MAX_INSTRUCAO - 1);
    tedax->instrucao_recebida[MAX_INSTRUCAO - 1] = '\0';
    tedax->tarefa_pendente = true;

    /* Sinaliza que ha uma nova tarefa */
    pthread_cond_signal(&tedax->cond_tarefa);

    pthread_mutex_unlock(&tedax->mutex);
    return true;
}

const char* tedax_estado_str(Tedax* tedax) {
    if (!tedax) return "Invalido";

    pthread_mutex_lock(&tedax->mutex);
    Estado est = tedax->estado;
    pthread_mutex_unlock(&tedax->mutex);

    switch (est) {
        case ESTADO_LIVRE: return "Livre";
        case ESTADO_OCUPADO: return "Trabalhando";
        case ESTADO_AGUARDANDO_BANCADA: return "Aguardando";
        default: return "Desconhecido";
    }
}

bool verificar_instrucao(Modulo* modulo, const char* instrucao) {
    if (!modulo || !instrucao) return false;

    /* Compara a instrucao fornecida com a instrucao correta do modulo */
    return (strcmp(modulo->instrucao, instrucao) == 0);
}

bool tedax_resolver_modulo(Tedax* tedax, Modulo* modulo, const char* instrucao) {
    if (!tedax || !modulo || !instrucao) return false;

    /* Verifica se a instrucao esta correta */
    bool sucesso = verificar_instrucao(modulo, instrucao);

    modulo->tentativas++;
    modulo->resolvido = sucesso;

    return sucesso;
}

/* Thread principal do Tedax - aguarda tarefa, ocupa bancada, resolve modulo */
void* thread_tedax(void* arg) {
    Tedax* tedax = (Tedax*)arg;
    if (!tedax || !jogo) return NULL;

    while (tedax->ativo && jogo->executando) {
        /* ========== FASE 1: Aguardar tarefa ========== */
        pthread_mutex_lock(&tedax->mutex);

        while (!tedax->tarefa_pendente && tedax->ativo && jogo->executando) {
            /* Aguarda receber uma tarefa do coordenador */
            pthread_cond_wait(&tedax->cond_tarefa, &tedax->mutex);
        }

        if (!tedax->ativo || !jogo->executando) {
            pthread_mutex_unlock(&tedax->mutex);
            break;
        }

        /* Copia dados da tarefa */
        Modulo* modulo = tedax->modulo_atual;
        int bancada_id = tedax->bancada_designada;
        char instrucao[MAX_INSTRUCAO];
        strncpy(instrucao, tedax->instrucao_recebida, MAX_INSTRUCAO);

        tedax->tarefa_pendente = false;
        tedax->estado = ESTADO_AGUARDANDO_BANCADA;

        pthread_mutex_unlock(&tedax->mutex);

        if (!modulo || bancada_id < 0 || bancada_id >= jogo->config.num_bancadas) {
            /* Tarefa invalida, volta a esperar */
            pthread_mutex_lock(&tedax->mutex);
            tedax->estado = ESTADO_LIVRE;
            if (modulo) {
                free(modulo);
                tedax->modulo_atual = NULL;
            }
            pthread_mutex_unlock(&tedax->mutex);
            continue;
        }

        /* ========== FASE 2: Obter bancada ========== */
        Bancada* bancada = &jogo->bancadas[bancada_id];

        /* Tenta ocupar a bancada (pode haver fila - assimetria) */
        jogo_feedback(jogo, "Tedax %d aguardando bancada %d...", tedax->id + 1, bancada_id + 1);

        /* Loop para tentar ocupar a bancada */
        bool conseguiu_bancada = false;
        while (!conseguiu_bancada && tedax->ativo && jogo->executando) {
            if (bancada_ocupar(bancada, tedax->id, modulo)) {
                conseguiu_bancada = true;
            } else {
                /* Bancada ocupada, aguarda ficar livre */
                /* Implementa a fila de espera (assimetria tedax/bancadas) */
                bancada_aguardar_livre(bancada, 500); /* timeout de 500ms */
            }

            /* Verifica se o jogo ainda esta rodando */
            pthread_mutex_lock(&jogo->mutex_estado);
            EstadoJogo est = jogo->estado;
            pthread_mutex_unlock(&jogo->mutex_estado);

            if (est != JOGO_RODANDO) {
                break;
            }
        }

        if (!conseguiu_bancada) {
            /* Nao conseguiu a bancada, retorna modulo para fila */
            pthread_mutex_lock(&tedax->mutex);
            tedax->estado = ESTADO_LIVRE;
            pthread_mutex_unlock(&tedax->mutex);

            /* Retorna modulo para a fila */
            fila_modulos_adicionar(&jogo->fila_modulos, modulo);
            free(modulo);

            pthread_mutex_lock(&tedax->mutex);
            tedax->modulo_atual = NULL;
            pthread_mutex_unlock(&tedax->mutex);

            continue;
        }

        /* ========== FASE 3: Resolver modulo ========== */
        pthread_mutex_lock(&tedax->mutex);
        tedax->estado = ESTADO_OCUPADO;
        tedax->bancada_atual = bancada;
        pthread_mutex_unlock(&tedax->mutex);

        jogo_feedback(jogo, "Tedax %d desarmando %s na bancada %d...",
                     tedax->id + 1, modulo->nome, bancada_id + 1);

        /* Simula o tempo de resolucao */
        int tempo_resolucao = modulo->tempo_resolucao;
        for (int i = 0; i < tempo_resolucao * 10 && tedax->ativo && jogo->executando; i++) {
            usleep(100000); /* 100ms */

            pthread_mutex_lock(&jogo->mutex_estado);
            EstadoJogo est = jogo->estado;
            pthread_mutex_unlock(&jogo->mutex_estado);

            if (est != JOGO_RODANDO) break;
        }

        /* Verifica se a resolucao foi bem sucedida */
        bool sucesso = tedax_resolver_modulo(tedax, modulo, instrucao);

        /* ========== FASE 4: Finalizar e liberar recursos ========== */
        bancada_liberar(bancada, tedax->id);

        pthread_mutex_lock(&tedax->mutex);
        tedax->bancada_atual = NULL;
        pthread_mutex_unlock(&tedax->mutex);

        /* Atualiza estatisticas */
        pthread_mutex_lock(&jogo->mutex_estado);
        if (sucesso) {
            tedax->modulos_desarmados++;
            jogo->stats.modulos_desarmados++;
            jogo_feedback(jogo, "Tedax %d desarmou %s com sucesso!",
                         tedax->id + 1, modulo->nome);
        } else {
            tedax->modulos_falhados++;
            jogo->stats.modulos_falhados++;

            /* Retorna modulo para a fila */
            modulo->tentativas++;
            fila_modulos_adicionar(&jogo->fila_modulos, modulo);

            jogo_feedback(jogo, "Tedax %d FALHOU em %s! Instrucao errada.",
                         tedax->id + 1, modulo->nome);
        }
        jogo->stats.modulos_pendentes = fila_modulos_quantidade(&jogo->fila_modulos);
        pthread_mutex_unlock(&jogo->mutex_estado);

        /* Libera o modulo */
        free(modulo);

        pthread_mutex_lock(&tedax->mutex);
        tedax->modulo_atual = NULL;
        tedax->estado = ESTADO_LIVRE;
        pthread_mutex_unlock(&tedax->mutex);
    }

    return NULL;
}
