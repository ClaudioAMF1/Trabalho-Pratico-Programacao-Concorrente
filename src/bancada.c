/*
 * bancada.c - Recurso compartilhado com mutex e cond var
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#include "../include/bancada.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

void bancada_init(Bancada* bancada, int id) {
    if (!bancada) return;

    bancada->id = id;
    bancada->estado = ESTADO_LIVRE;
    bancada->modulo_atual = NULL;
    bancada->tedax_id = -1;

    pthread_mutex_init(&bancada->mutex, NULL);
    pthread_cond_init(&bancada->cond_livre, NULL);
}

void bancada_destroy(Bancada* bancada) {
    if (!bancada) return;

    pthread_mutex_destroy(&bancada->mutex);
    pthread_cond_destroy(&bancada->cond_livre);
}

bool bancada_livre(Bancada* bancada) {
    if (!bancada) return false;

    pthread_mutex_lock(&bancada->mutex);
    bool livre = (bancada->estado == ESTADO_LIVRE);
    pthread_mutex_unlock(&bancada->mutex);

    return livre;
}

bool bancada_ocupar(Bancada* bancada, int tedax_id, Modulo* modulo) {
    if (!bancada) return false;

    pthread_mutex_lock(&bancada->mutex);

    if (bancada->estado != ESTADO_LIVRE) {
        pthread_mutex_unlock(&bancada->mutex);
        return false;
    }

    bancada->estado = ESTADO_OCUPADO;
    bancada->tedax_id = tedax_id;
    bancada->modulo_atual = modulo;

    pthread_mutex_unlock(&bancada->mutex);
    return true;
}

bool bancada_liberar(Bancada* bancada, int tedax_id) {
    if (!bancada) return false;

    pthread_mutex_lock(&bancada->mutex);

    /* Verifica se o tedax que esta liberando e o dono */
    if (bancada->tedax_id != tedax_id) {
        pthread_mutex_unlock(&bancada->mutex);
        return false;
    }

    bancada->estado = ESTADO_LIVRE;
    bancada->tedax_id = -1;
    bancada->modulo_atual = NULL;

    /* Sinaliza que a bancada esta livre */
    pthread_cond_broadcast(&bancada->cond_livre);

    pthread_mutex_unlock(&bancada->mutex);
    return true;
}

bool bancada_aguardar_livre(Bancada* bancada, int timeout_ms) {
    if (!bancada) return false;

    pthread_mutex_lock(&bancada->mutex);

    if (bancada->estado == ESTADO_LIVRE) {
        pthread_mutex_unlock(&bancada->mutex);
        return true;
    }

    if (timeout_ms <= 0) {
        /* Aguarda indefinidamente */
        while (bancada->estado != ESTADO_LIVRE) {
            pthread_cond_wait(&bancada->cond_livre, &bancada->mutex);
        }
        pthread_mutex_unlock(&bancada->mutex);
        return true;
    }

    /* Aguarda com timeout */
    struct timespec ts;
    struct timeval tv;
    gettimeofday(&tv, NULL);

    ts.tv_sec = tv.tv_sec + (timeout_ms / 1000);
    ts.tv_nsec = (tv.tv_usec * 1000) + ((timeout_ms % 1000) * 1000000);
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    int ret = 0;
    while (bancada->estado != ESTADO_LIVRE && ret != ETIMEDOUT) {
        ret = pthread_cond_timedwait(&bancada->cond_livre, &bancada->mutex, &ts);
    }

    bool livre = (bancada->estado == ESTADO_LIVRE);
    pthread_mutex_unlock(&bancada->mutex);

    return livre;
}

const char* bancada_estado_str(Bancada* bancada) {
    if (!bancada) return "Invalida";

    pthread_mutex_lock(&bancada->mutex);
    Estado est = bancada->estado;
    pthread_mutex_unlock(&bancada->mutex);

    switch (est) {
        case ESTADO_LIVRE: return "Livre";
        case ESTADO_OCUPADO: return "Ocupada";
        default: return "Desconhecido";
    }
}

int encontrar_bancada_livre(Bancada* bancadas, int num_bancadas) {
    if (!bancadas || num_bancadas <= 0) return -1;

    for (int i = 0; i < num_bancadas; i++) {
        if (bancada_livre(&bancadas[i])) {
            return i;
        }
    }

    return -1;
}

int contar_bancadas_ocupadas(Bancada* bancadas, int num_bancadas) {
    if (!bancadas || num_bancadas <= 0) return 0;

    int count = 0;
    for (int i = 0; i < num_bancadas; i++) {
        if (!bancada_livre(&bancadas[i])) {
            count++;
        }
    }

    return count;
}
