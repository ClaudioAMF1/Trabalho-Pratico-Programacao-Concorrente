/*
 * jogo.c - Controle geral do jogo, threads e processamento de comandos
 * Keep Solving and Nobody Explodes - Versao de Treino
 */

#include "../include/jogo.h"
#include "../include/modulos.h"
#include "../include/bancada.h"
#include "../include/tedax.h"
#include "../include/display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>

extern EstadoJogoCompleto* jogo;

ConfigJogo config_padrao(void) {
    ConfigJogo config;
    config.num_tedax = 2;
    config.num_bancadas = 2;
    config.tempo_partida = TEMPO_PARTIDA_PADRAO;
    config.dificuldade = 1;
    config.modulos_para_vencer = 10;
    config.modo_infinito = false;
    return config;
}

int jogo_init(EstadoJogoCompleto* estado, ConfigJogo* config) {
    if (!estado || !config) return -1;

    memset(estado, 0, sizeof(EstadoJogoCompleto));

    memcpy(&estado->config, config, sizeof(ConfigJogo));
    if (estado->config.num_tedax < 1) estado->config.num_tedax = 1;
    if (estado->config.num_tedax > MAX_TEDAX) estado->config.num_tedax = MAX_TEDAX;
    if (estado->config.num_bancadas < 1) estado->config.num_bancadas = 1;
    if (estado->config.num_bancadas > MAX_BANCADAS) estado->config.num_bancadas = MAX_BANCADAS;
    if (estado->config.dificuldade < 1) estado->config.dificuldade = 1;
    if (estado->config.dificuldade > 3) estado->config.dificuldade = 3;

    /* Inicializa estado */
    estado->estado = JOGO_MENU;
    estado->executando = true;
    estado->proximo_id_modulo = 1;

    /* Inicializa estatisticas */
    memset(&estado->stats, 0, sizeof(Estatisticas));
    estado->stats.tempo_restante = estado->config.tempo_partida;

    /* Inicializa mutexes */
    pthread_mutex_init(&estado->mutex_estado, NULL);
    pthread_mutex_init(&estado->mutex_display, NULL);
    pthread_mutex_init(&estado->mutex_comando, NULL);
    pthread_cond_init(&estado->cond_fim_jogo, NULL);

    /* Inicializa fila de modulos */
    fila_modulos_init(&estado->fila_modulos);

    /* Inicializa bancadas */
    for (int i = 0; i < MAX_BANCADAS; i++) {
        bancada_init(&estado->bancadas[i], i);
    }

    /* Inicializa tedax */
    for (int i = 0; i < MAX_TEDAX; i++) {
        tedax_init(&estado->tedax[i], i);
    }

    /* Buffer de comando */
    memset(estado->buffer_comando, 0, sizeof(estado->buffer_comando));
    estado->pos_buffer = 0;

    /* Mensagem de feedback */
    memset(estado->mensagem_feedback, 0, sizeof(estado->mensagem_feedback));
    estado->tempo_mensagem = 0;

    return 0;
}

void jogo_finalizar(EstadoJogoCompleto* estado) {
    if (!estado) return;

    /* Para a execucao */
    estado->executando = false;

    /* Para as threads */
    jogo_parar_partida(estado);

    /* Destroi bancadas */
    for (int i = 0; i < MAX_BANCADAS; i++) {
        bancada_destroy(&estado->bancadas[i]);
    }

    /* Destroi tedax */
    for (int i = 0; i < MAX_TEDAX; i++) {
        tedax_destroy(&estado->tedax[i]);
    }

    /* Destroi fila de modulos */
    fila_modulos_destroy(&estado->fila_modulos);

    /* Destroi mutexes */
    pthread_mutex_destroy(&estado->mutex_estado);
    pthread_mutex_destroy(&estado->mutex_display);
    pthread_mutex_destroy(&estado->mutex_comando);
    pthread_cond_destroy(&estado->cond_fim_jogo);
}

int jogo_iniciar_partida(EstadoJogoCompleto* estado) {
    if (!estado) return -1;

    /* Reinicializa estatisticas */
    pthread_mutex_lock(&estado->mutex_estado);
    memset(&estado->stats, 0, sizeof(Estatisticas));
    estado->stats.tempo_restante = estado->config.tempo_partida;
    estado->stats.inicio_partida = time(NULL);
    estado->proximo_id_modulo = 1;
    estado->estado = JOGO_RODANDO;
    pthread_mutex_unlock(&estado->mutex_estado);

    /* Limpa a fila de modulos */
    while (!fila_modulos_vazia(&estado->fila_modulos)) {
        Modulo m;
        fila_modulos_remover(&estado->fila_modulos, &m);
    }

    /* Reseta tedax */
    for (int i = 0; i < estado->config.num_tedax; i++) {
        pthread_mutex_lock(&estado->tedax[i].mutex);
        estado->tedax[i].estado = ESTADO_LIVRE;
        estado->tedax[i].modulos_desarmados = 0;
        estado->tedax[i].modulos_falhados = 0;
        estado->tedax[i].tarefa_pendente = false;
        pthread_mutex_unlock(&estado->tedax[i].mutex);
    }

    /* Reseta bancadas */
    for (int i = 0; i < estado->config.num_bancadas; i++) {
        pthread_mutex_lock(&estado->bancadas[i].mutex);
        estado->bancadas[i].estado = ESTADO_LIVRE;
        estado->bancadas[i].tedax_id = -1;
        estado->bancadas[i].modulo_atual = NULL;
        pthread_mutex_unlock(&estado->bancadas[i].mutex);
    }

    /* Inicia threads dos tedax */
    for (int i = 0; i < estado->config.num_tedax; i++) {
        tedax_iniciar_thread(&estado->tedax[i]);
    }

    /* Inicia thread do mural de modulos */
    pthread_create(&estado->thread_mural, NULL, thread_mural_modulos, estado);

    /* Inicia thread do timer */
    pthread_create(&estado->thread_timer, NULL, thread_timer, estado);

    /* Inicia thread de display */
    pthread_create(&estado->thread_display, NULL, thread_display, estado);

    jogo_feedback(estado, "Partida iniciada! Boa sorte!");

    return 0;
}

void jogo_parar_partida(EstadoJogoCompleto* estado) {
    if (!estado) return;

    /* Para as threads dos tedax */
    for (int i = 0; i < estado->config.num_tedax; i++) {
        tedax_parar_thread(&estado->tedax[i]);
    }

    /* Sinaliza fim do jogo */
    pthread_mutex_lock(&estado->mutex_estado);
    if (estado->estado == JOGO_RODANDO || estado->estado == JOGO_PAUSADO) {
        estado->estado = JOGO_SAINDO;
    }
    pthread_cond_broadcast(&estado->cond_fim_jogo);
    pthread_mutex_unlock(&estado->mutex_estado);

    /* Aguarda threads */
    pthread_join(estado->thread_mural, NULL);
    pthread_join(estado->thread_timer, NULL);
    pthread_join(estado->thread_display, NULL);
}

void jogo_pausar(EstadoJogoCompleto* estado) {
    if (!estado) return;

    pthread_mutex_lock(&estado->mutex_estado);
    if (estado->estado == JOGO_RODANDO) {
        estado->estado = JOGO_PAUSADO;
        jogo_feedback(estado, "Jogo pausado!");
    } else if (estado->estado == JOGO_PAUSADO) {
        estado->estado = JOGO_RODANDO;
        jogo_feedback(estado, "Jogo retomado!");
    }
    pthread_mutex_unlock(&estado->mutex_estado);
}

bool jogo_verificar_fim(EstadoJogoCompleto* estado) {
    if (!estado) return true;

    pthread_mutex_lock(&estado->mutex_estado);

    bool fim = false;
    EstadoJogo novo_estado = estado->estado;

    /* Verifica vitoria */
    if (!estado->config.modo_infinito &&
        estado->stats.modulos_desarmados >= estado->config.modulos_para_vencer) {
        novo_estado = JOGO_VITORIA;
        fim = true;
    }

    /* Verifica derrota por tempo */
    if (estado->stats.tempo_restante <= 0) {
        novo_estado = JOGO_DERROTA;
        fim = true;
    }

    /* Verifica derrota por acumulo de modulos */
    if (estado->fila_modulos.quantidade >= MAX_MODULOS_PENDENTES) {
        novo_estado = JOGO_DERROTA;
        fim = true;
    }

    if (fim) {
        estado->estado = novo_estado;
    }

    pthread_mutex_unlock(&estado->mutex_estado);

    return fim;
}

void jogo_adicionar_char_comando(EstadoJogoCompleto* estado, char c) {
    if (!estado) return;

    pthread_mutex_lock(&estado->mutex_comando);
    if (estado->pos_buffer < (int)sizeof(estado->buffer_comando) - 1) {
        estado->buffer_comando[estado->pos_buffer++] = c;
        estado->buffer_comando[estado->pos_buffer] = '\0';
    }
    pthread_mutex_unlock(&estado->mutex_comando);
}

void jogo_remover_char_comando(EstadoJogoCompleto* estado) {
    if (!estado) return;

    pthread_mutex_lock(&estado->mutex_comando);
    if (estado->pos_buffer > 0) {
        estado->buffer_comando[--estado->pos_buffer] = '\0';
    }
    pthread_mutex_unlock(&estado->mutex_comando);
}

void jogo_limpar_comando(EstadoJogoCompleto* estado) {
    if (!estado) return;

    pthread_mutex_lock(&estado->mutex_comando);
    memset(estado->buffer_comando, 0, sizeof(estado->buffer_comando));
    estado->pos_buffer = 0;
    pthread_mutex_unlock(&estado->mutex_comando);
}

/* Formato: [TEDAX][TIPO][BANCADA][INSTRUCAO] ex: 1f1rgb */
bool jogo_processar_comando(EstadoJogoCompleto* estado, const char* comando) {
    if (!estado || !comando || strlen(comando) < 4) {
        jogo_feedback(estado, "Comando muito curto! Formato: [TEDAX][TIPO][BANCADA][INSTRUCAO]");
        return false;
    }

    /* Parse do comando */
    int tedax_num = comando[0] - '0';
    char tipo_char = tolower(comando[1]);
    int bancada_num = comando[2] - '0';
    const char* instrucao = &comando[3];

    /* Valida tedax */
    if (tedax_num < 1 || tedax_num > estado->config.num_tedax) {
        jogo_feedback(estado, "Tedax invalido! Use 1-%d", estado->config.num_tedax);
        return false;
    }

    /* Valida tipo de modulo */
    TipoModulo tipo = tipo_modulo_por_char(tipo_char);
    if (tipo == (TipoModulo)-1) {
        jogo_feedback(estado, "Tipo de modulo invalido! Use: f/b/s/i");
        return false;
    }

    /* Valida bancada */
    if (bancada_num < 1 || bancada_num > estado->config.num_bancadas) {
        jogo_feedback(estado, "Bancada invalida! Use 1-%d", estado->config.num_bancadas);
        return false;
    }

    /* Verifica se o tedax esta disponivel */
    Tedax* tedax = &estado->tedax[tedax_num - 1];
    if (!tedax_disponivel(tedax)) {
        jogo_feedback(estado, "Tedax %d esta ocupado!", tedax_num);
        return false;
    }

    /* Procura um modulo do tipo especificado na fila */
    Modulo modulo_encontrado;
    bool encontrou = false;

    pthread_mutex_lock(&estado->fila_modulos.mutex);
    for (int i = 0; i < estado->fila_modulos.quantidade; i++) {
        int idx = (estado->fila_modulos.inicio + i) % MAX_MODULOS_PENDENTES;
        if (estado->fila_modulos.modulos[idx].tipo == tipo) {
            memcpy(&modulo_encontrado, &estado->fila_modulos.modulos[idx], sizeof(Modulo));
            encontrou = true;

            /* Remove o modulo da fila */
            for (int j = i; j < estado->fila_modulos.quantidade - 1; j++) {
                int idx_atual = (estado->fila_modulos.inicio + j) % MAX_MODULOS_PENDENTES;
                int idx_prox = (estado->fila_modulos.inicio + j + 1) % MAX_MODULOS_PENDENTES;
                memcpy(&estado->fila_modulos.modulos[idx_atual],
                       &estado->fila_modulos.modulos[idx_prox], sizeof(Modulo));
            }
            estado->fila_modulos.fim = (estado->fila_modulos.fim - 1 + MAX_MODULOS_PENDENTES) % MAX_MODULOS_PENDENTES;
            estado->fila_modulos.quantidade--;
            break;
        }
    }
    pthread_mutex_unlock(&estado->fila_modulos.mutex);

    if (!encontrou) {
        jogo_feedback(estado, "Nenhum modulo do tipo '%c' na fila!", tipo_char);
        return false;
    }

    /* Designa o modulo para o tedax */
    if (!tedax_designar_modulo(tedax, &modulo_encontrado, bancada_num - 1, instrucao)) {
        /* Retorna modulo para a fila */
        fila_modulos_adicionar(&estado->fila_modulos, &modulo_encontrado);
        jogo_feedback(estado, "Erro ao designar modulo para Tedax %d!", tedax_num);
        return false;
    }

    jogo_feedback(estado, "Tedax %d designado: %s [%s] -> Bancada %d",
                 tedax_num, modulo_encontrado.nome, instrucao, bancada_num);

    /* Atualiza estatisticas */
    pthread_mutex_lock(&estado->mutex_estado);
    estado->stats.modulos_pendentes = fila_modulos_quantidade(&estado->fila_modulos);
    pthread_mutex_unlock(&estado->mutex_estado);

    return true;
}

bool jogo_executar_comando(EstadoJogoCompleto* estado) {
    if (!estado) return false;

    pthread_mutex_lock(&estado->mutex_comando);
    char comando[32];
    strncpy(comando, estado->buffer_comando, sizeof(comando) - 1);
    comando[sizeof(comando) - 1] = '\0';
    pthread_mutex_unlock(&estado->mutex_comando);

    if (strlen(comando) == 0) {
        return false;
    }

    bool resultado = jogo_processar_comando(estado, comando);
    jogo_limpar_comando(estado);

    return resultado;
}

void jogo_feedback(EstadoJogoCompleto* estado, const char* formato, ...) {
    if (!estado || !formato) return;

    va_list args;
    va_start(args, formato);

    pthread_mutex_lock(&estado->mutex_estado);
    vsnprintf(estado->mensagem_feedback, sizeof(estado->mensagem_feedback), formato, args);
    estado->tempo_mensagem = time(NULL);
    pthread_mutex_unlock(&estado->mutex_estado);

    va_end(args);
}

EstadoJogo jogo_obter_estado(EstadoJogoCompleto* estado) {
    if (!estado) return JOGO_SAINDO;

    pthread_mutex_lock(&estado->mutex_estado);
    EstadoJogo est = estado->estado;
    pthread_mutex_unlock(&estado->mutex_estado);

    return est;
}

void jogo_definir_estado(EstadoJogoCompleto* estado, EstadoJogo novo_estado) {
    if (!estado) return;

    pthread_mutex_lock(&estado->mutex_estado);
    estado->estado = novo_estado;
    pthread_mutex_unlock(&estado->mutex_estado);
}

/* Thread do timer - decrementa tempo a cada segundo */
void* thread_timer(void* arg) {
    EstadoJogoCompleto* estado = (EstadoJogoCompleto*)arg;
    if (!estado) return NULL;

    while (estado->executando) {
        pthread_mutex_lock(&estado->mutex_estado);
        EstadoJogo est = estado->estado;
        pthread_mutex_unlock(&estado->mutex_estado);

        if (est == JOGO_RODANDO) {
            sleep(1);

            pthread_mutex_lock(&estado->mutex_estado);
            if (estado->stats.tempo_restante > 0) {
                estado->stats.tempo_restante--;
            }
            pthread_mutex_unlock(&estado->mutex_estado);

            /* Verifica fim de jogo */
            if (jogo_verificar_fim(estado)) {
                break;
            }
        } else if (est == JOGO_PAUSADO) {
            usleep(100000);
        } else if (est == JOGO_VITORIA || est == JOGO_DERROTA || est == JOGO_SAINDO) {
            break;
        } else {
            usleep(100000);
        }
    }

    return NULL;
}

void* thread_coordenador(void* arg) {
    EstadoJogoCompleto* estado = (EstadoJogoCompleto*)arg;
    if (!estado) return NULL;

    /* Esta thread foi substituida pelo main loop */
    while (estado->executando) {
        usleep(100000);
    }

    return NULL;
}
