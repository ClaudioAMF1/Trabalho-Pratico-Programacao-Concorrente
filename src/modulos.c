/**
 * @file modulos.c
 * @brief Implementacao do gerenciamento de modulos
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#include "../include/modulos.h"
#include "../include/jogo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/* Nomes dos tipos de modulos */
static const char* nomes_modulos[] = {
    "Fios",
    "Botao",
    "Sequencia",
    "Simon"
};

/* Caracteres identificadores */
static const char chars_modulos[] = {'f', 'b', 's', 'i'};

/* ==================== FUNCOES DA FILA ==================== */

void fila_modulos_init(FilaModulos* fila) {
    if (!fila) return;

    fila->inicio = 0;
    fila->fim = 0;
    fila->quantidade = 0;

    pthread_mutex_init(&fila->mutex, NULL);
    pthread_cond_init(&fila->cond_nao_vazia, NULL);
    pthread_cond_init(&fila->cond_nao_cheia, NULL);

    memset(fila->modulos, 0, sizeof(fila->modulos));
}

void fila_modulos_destroy(FilaModulos* fila) {
    if (!fila) return;

    pthread_mutex_destroy(&fila->mutex);
    pthread_cond_destroy(&fila->cond_nao_vazia);
    pthread_cond_destroy(&fila->cond_nao_cheia);
}

bool fila_modulos_adicionar(FilaModulos* fila, Modulo* modulo) {
    if (!fila || !modulo) return false;

    pthread_mutex_lock(&fila->mutex);

    if (fila->quantidade >= MAX_MODULOS_PENDENTES) {
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }

    /* Copia o modulo para a fila */
    memcpy(&fila->modulos[fila->fim], modulo, sizeof(Modulo));
    fila->fim = (fila->fim + 1) % MAX_MODULOS_PENDENTES;
    fila->quantidade++;

    /* Sinaliza que a fila nao esta mais vazia */
    pthread_cond_signal(&fila->cond_nao_vazia);

    pthread_mutex_unlock(&fila->mutex);
    return true;
}

bool fila_modulos_remover(FilaModulos* fila, Modulo* modulo) {
    if (!fila || !modulo) return false;

    pthread_mutex_lock(&fila->mutex);

    if (fila->quantidade == 0) {
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }

    /* Copia o modulo do inicio */
    memcpy(modulo, &fila->modulos[fila->inicio], sizeof(Modulo));
    fila->inicio = (fila->inicio + 1) % MAX_MODULOS_PENDENTES;
    fila->quantidade--;

    /* Sinaliza que a fila nao esta mais cheia */
    pthread_cond_signal(&fila->cond_nao_cheia);

    pthread_mutex_unlock(&fila->mutex);
    return true;
}

bool fila_modulos_remover_por_id(FilaModulos* fila, int id, Modulo* modulo) {
    if (!fila || !modulo) return false;

    pthread_mutex_lock(&fila->mutex);

    if (fila->quantidade == 0) {
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }

    /* Procura o modulo pelo ID */
    int encontrado = -1;
    for (int i = 0; i < fila->quantidade; i++) {
        int idx = (fila->inicio + i) % MAX_MODULOS_PENDENTES;
        if (fila->modulos[idx].id == id) {
            encontrado = i;
            memcpy(modulo, &fila->modulos[idx], sizeof(Modulo));
            break;
        }
    }

    if (encontrado == -1) {
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }

    /* Remove o modulo, movendo os posteriores */
    for (int i = encontrado; i < fila->quantidade - 1; i++) {
        int idx_atual = (fila->inicio + i) % MAX_MODULOS_PENDENTES;
        int idx_prox = (fila->inicio + i + 1) % MAX_MODULOS_PENDENTES;
        memcpy(&fila->modulos[idx_atual], &fila->modulos[idx_prox], sizeof(Modulo));
    }

    fila->fim = (fila->fim - 1 + MAX_MODULOS_PENDENTES) % MAX_MODULOS_PENDENTES;
    fila->quantidade--;

    pthread_cond_signal(&fila->cond_nao_cheia);

    pthread_mutex_unlock(&fila->mutex);
    return true;
}

bool fila_modulos_obter(FilaModulos* fila, int indice, Modulo* modulo) {
    if (!fila || !modulo || indice < 0) return false;

    pthread_mutex_lock(&fila->mutex);

    if (indice >= fila->quantidade) {
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }

    int idx = (fila->inicio + indice) % MAX_MODULOS_PENDENTES;
    memcpy(modulo, &fila->modulos[idx], sizeof(Modulo));

    pthread_mutex_unlock(&fila->mutex);
    return true;
}

bool fila_modulos_vazia(FilaModulos* fila) {
    if (!fila) return true;

    pthread_mutex_lock(&fila->mutex);
    bool vazia = (fila->quantidade == 0);
    pthread_mutex_unlock(&fila->mutex);

    return vazia;
}

bool fila_modulos_cheia(FilaModulos* fila) {
    if (!fila) return true;

    pthread_mutex_lock(&fila->mutex);
    bool cheia = (fila->quantidade >= MAX_MODULOS_PENDENTES);
    pthread_mutex_unlock(&fila->mutex);

    return cheia;
}

int fila_modulos_quantidade(FilaModulos* fila) {
    if (!fila) return 0;

    pthread_mutex_lock(&fila->mutex);
    int qtd = fila->quantidade;
    pthread_mutex_unlock(&fila->mutex);

    return qtd;
}

/* ==================== FUNCOES DE MODULO ==================== */

const char* nome_tipo_modulo(TipoModulo tipo) {
    if (tipo >= 0 && tipo < MODULO_TOTAL) {
        return nomes_modulos[tipo];
    }
    return "Desconhecido";
}

char char_tipo_modulo(TipoModulo tipo) {
    if (tipo >= 0 && tipo < MODULO_TOTAL) {
        return chars_modulos[tipo];
    }
    return '?';
}

TipoModulo tipo_modulo_por_char(char c) {
    for (int i = 0; i < MODULO_TOTAL; i++) {
        if (chars_modulos[i] == c) {
            return (TipoModulo)i;
        }
    }
    return -1;
}

Modulo gerar_modulo_aleatorio(int id, int dificuldade) {
    Modulo m;
    memset(&m, 0, sizeof(Modulo));

    m.id = id;
    m.tipo = rand() % MODULO_TOTAL;
    m.dificuldade = dificuldade;
    m.resolvido = false;
    m.tentativas = 0;
    m.criado_em = time(NULL);

    /* Configura baseado no tipo */
    switch (m.tipo) {
        case MODULO_FIOS:
            /* Cortar fios: parametro = quantidade de cortes (2-4) */
            m.parametro = 2 + (rand() % 3);
            m.tempo_resolucao = 3 + dificuldade;
            snprintf(m.nome, MAX_NOME_MODULO, "Fios #%d", id);
            /* Gera sequencia de cores (r=red, g=green, b=blue, y=yellow) */
            {
                char cores[] = {'r', 'g', 'b', 'y'};
                m.instrucao[0] = '\0';
                for (int i = 0; i < m.parametro; i++) {
                    char c[2] = {cores[rand() % 4], '\0'};
                    strcat(m.instrucao, c);
                }
            }
            break;

        case MODULO_BOTAO:
            /* Pressionar botao: parametro = quantidade de pressionamentos (2-5) */
            m.parametro = 2 + (rand() % 4);
            m.tempo_resolucao = 2 + dificuldade;
            snprintf(m.nome, MAX_NOME_MODULO, "Botao #%d", id);
            /* Instrucao: repetir 'p' N vezes */
            memset(m.instrucao, 'p', m.parametro);
            m.instrucao[m.parametro] = '\0';
            break;

        case MODULO_SEQUENCIA:
            /* Sequencia de teclas: parametro = tamanho (3-5) */
            m.parametro = 3 + (rand() % 3);
            m.tempo_resolucao = 4 + dificuldade;
            snprintf(m.nome, MAX_NOME_MODULO, "Seq #%d", id);
            /* Gera sequencia de numeros */
            for (int i = 0; i < m.parametro; i++) {
                m.instrucao[i] = '1' + (rand() % 4);
            }
            m.instrucao[m.parametro] = '\0';
            break;

        case MODULO_SIMON:
            /* Simon diz: parametro = tamanho da sequencia (3-4) */
            m.parametro = 3 + (rand() % 2);
            m.tempo_resolucao = 5 + dificuldade;
            snprintf(m.nome, MAX_NOME_MODULO, "Simon #%d", id);
            /* Gera sequencia UDLR (up, down, left, right) */
            {
                char dirs[] = {'u', 'd', 'l', 'r'};
                for (int i = 0; i < m.parametro; i++) {
                    m.instrucao[i] = dirs[rand() % 4];
                }
                m.instrucao[m.parametro] = '\0';
            }
            break;

        default:
            snprintf(m.nome, MAX_NOME_MODULO, "Modulo #%d", id);
            strcpy(m.instrucao, "x");
            m.tempo_resolucao = 3;
            break;
    }

    return m;
}

/* ==================== THREAD DO MURAL ==================== */

void* thread_mural_modulos(void* arg) {
    EstadoJogoCompleto* estado = (EstadoJogoCompleto*)arg;
    if (!estado) return NULL;

    while (estado->executando) {
        /* Verifica se o jogo esta rodando */
        pthread_mutex_lock(&estado->mutex_estado);
        EstadoJogo est = estado->estado;
        pthread_mutex_unlock(&estado->mutex_estado);

        if (est != JOGO_RODANDO) {
            usleep(100000); /* 100ms */
            continue;
        }

        /* Verifica se pode adicionar mais modulos */
        if (!fila_modulos_cheia(&estado->fila_modulos)) {
            /* Gera um novo modulo */
            pthread_mutex_lock(&estado->mutex_estado);
            int id = estado->proximo_id_modulo++;
            int dif = estado->config.dificuldade;
            pthread_mutex_unlock(&estado->mutex_estado);

            Modulo novo = gerar_modulo_aleatorio(id, dif);

            if (fila_modulos_adicionar(&estado->fila_modulos, &novo)) {
                pthread_mutex_lock(&estado->mutex_estado);
                estado->stats.modulos_gerados++;
                estado->stats.modulos_pendentes = fila_modulos_quantidade(&estado->fila_modulos);
                pthread_mutex_unlock(&estado->mutex_estado);

                jogo_feedback(estado, "Novo modulo: %s [%c] - Instrucao: %s",
                             novo.nome, char_tipo_modulo(novo.tipo), novo.instrucao);
            }
        }

        /* Intervalo aleatorio entre geracoes */
        int intervalo = INTERVALO_GERACAO_MIN +
                       (rand() % (INTERVALO_GERACAO_MAX - INTERVALO_GERACAO_MIN + 1));

        /* Ajusta intervalo pela dificuldade */
        pthread_mutex_lock(&estado->mutex_estado);
        int dif = estado->config.dificuldade;
        pthread_mutex_unlock(&estado->mutex_estado);

        intervalo = intervalo - dif + 1;
        if (intervalo < 2) intervalo = 2;

        /* Aguarda o intervalo, verificando se deve parar */
        for (int i = 0; i < intervalo * 10 && estado->executando; i++) {
            usleep(100000); /* 100ms */

            pthread_mutex_lock(&estado->mutex_estado);
            est = estado->estado;
            pthread_mutex_unlock(&estado->mutex_estado);

            if (est != JOGO_RODANDO) break;
        }
    }

    return NULL;
}
