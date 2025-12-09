/**
 * @file tipos.h
 * @brief Definicoes de tipos e estruturas principais do jogo
 *
 * Keep Solving and Nobody Explodes - Versao de Treino
 * Trabalho Pratico - Programacao Concorrente
 * IDP 2025/2
 */

#ifndef TIPOS_H
#define TIPOS_H

#include <pthread.h>
#include <stdbool.h>
#include <time.h>

/* ==================== CONSTANTES ==================== */

#define MAX_TEDAX 3
#define MAX_BANCADAS 5
#define MAX_MODULOS_PENDENTES 10
#define MAX_NOME_MODULO 32
#define MAX_INSTRUCAO 64

#define TEMPO_PARTIDA_PADRAO 120    /* segundos */
#define INTERVALO_GERACAO_MIN 3     /* segundos entre geracao de modulos */
#define INTERVALO_GERACAO_MAX 8

/* Tipos de modulos */
typedef enum {
    MODULO_FIOS = 0,        /* 'f' - Cortar fios na sequencia correta */
    MODULO_BOTAO,           /* 'b' - Pressionar botao N vezes */
    MODULO_SEQUENCIA,       /* 's' - Digitar sequencia de teclas */
    MODULO_SIMON,           /* 'i' - Memorizar e repetir sequencia */
    MODULO_TOTAL
} TipoModulo;

/* Estados dos elementos do jogo */
typedef enum {
    ESTADO_LIVRE = 0,
    ESTADO_OCUPADO,
    ESTADO_AGUARDANDO_BANCADA,
    ESTADO_INATIVO
} Estado;

/* Estados do jogo */
typedef enum {
    JOGO_MENU = 0,
    JOGO_CONFIGURANDO,
    JOGO_RODANDO,
    JOGO_PAUSADO,
    JOGO_VITORIA,
    JOGO_DERROTA,
    JOGO_SAINDO
} EstadoJogo;

/* ==================== ESTRUTURAS ==================== */

/**
 * @struct Modulo
 * @brief Representa um modulo da bomba a ser desarmado
 */
typedef struct {
    int id;                         /* Identificador unico do modulo */
    TipoModulo tipo;                /* Tipo do modulo */
    char nome[MAX_NOME_MODULO];     /* Nome para exibicao */
    int dificuldade;                /* 1-3, afeta tempo de resolucao */
    int tempo_resolucao;            /* Tempo em segundos para resolver */
    int parametro;                  /* Parametro especifico do tipo (ex: qtd de cliques) */
    char instrucao[MAX_INSTRUCAO];  /* Instrucao para o jogador */
    bool resolvido;                 /* Se foi resolvido com sucesso */
    int tentativas;                 /* Numero de tentativas */
    time_t criado_em;               /* Quando foi criado */
} Modulo;

/**
 * @struct Bancada
 * @brief Representa uma bancada de desativacao
 */
typedef struct {
    int id;                         /* Identificador da bancada */
    Estado estado;                  /* Livre ou ocupada */
    Modulo* modulo_atual;           /* Modulo sendo desarmado */
    int tedax_id;                   /* ID do tedax usando a bancada (-1 se livre) */
    pthread_mutex_t mutex;          /* Mutex para acesso a bancada */
    pthread_cond_t cond_livre;      /* Condicao para bancada livre */
} Bancada;

/**
 * @struct Tedax
 * @brief Representa um Tecnico Especialista em Desativacao de Artefatos Explosivos
 */
typedef struct {
    int id;                         /* Identificador do tedax */
    Estado estado;                  /* Livre, ocupado ou aguardando */
    Modulo* modulo_atual;           /* Modulo sendo desarmado */
    Bancada* bancada_atual;         /* Bancada sendo utilizada */
    int modulos_desarmados;         /* Contador de sucessos */
    int modulos_falhados;           /* Contador de falhas */
    pthread_t thread;               /* Thread do tedax */
    pthread_mutex_t mutex;          /* Mutex para estado do tedax */
    pthread_cond_t cond_tarefa;     /* Condicao para nova tarefa */
    bool ativo;                     /* Se a thread esta ativa */

    /* Dados da tarefa atual */
    int bancada_designada;          /* ID da bancada designada (-1 se nenhuma) */
    char instrucao_recebida[MAX_INSTRUCAO]; /* Instrucao recebida do coordenador */
    bool tarefa_pendente;           /* Se ha tarefa pendente */
} Tedax;

/**
 * @struct FilaModulos
 * @brief Fila de modulos pendentes (circular)
 */
typedef struct {
    Modulo modulos[MAX_MODULOS_PENDENTES];
    int inicio;                     /* Indice do primeiro elemento */
    int fim;                        /* Indice apos o ultimo elemento */
    int quantidade;                 /* Quantidade atual de modulos */
    pthread_mutex_t mutex;          /* Mutex para acesso a fila */
    pthread_cond_t cond_nao_vazia;  /* Condicao para fila nao vazia */
    pthread_cond_t cond_nao_cheia;  /* Condicao para fila nao cheia */
} FilaModulos;

/**
 * @struct ConfigJogo
 * @brief Configuracoes da partida
 */
typedef struct {
    int num_tedax;                  /* Numero de tedax (1-3) */
    int num_bancadas;               /* Numero de bancadas (1-5) */
    int tempo_partida;              /* Tempo total da partida em segundos */
    int dificuldade;                /* Nivel de dificuldade (1-3) */
    int modulos_para_vencer;        /* Quantidade de modulos para vencer */
    bool modo_infinito;             /* Modo sem limite de modulos */
} ConfigJogo;

/**
 * @struct Estatisticas
 * @brief Estatisticas da partida atual
 */
typedef struct {
    int modulos_gerados;            /* Total de modulos gerados */
    int modulos_desarmados;         /* Total de modulos desarmados */
    int modulos_falhados;           /* Total de falhas */
    int modulos_pendentes;          /* Modulos na fila */
    time_t inicio_partida;          /* Quando a partida iniciou */
    int tempo_restante;             /* Tempo restante em segundos */
} Estatisticas;

/**
 * @struct EstadoJogoCompleto
 * @brief Estado completo do jogo (recurso compartilhado principal)
 */
typedef struct {
    /* Configuracoes */
    ConfigJogo config;

    /* Estado atual */
    EstadoJogo estado;
    Estatisticas stats;

    /* Elementos do jogo */
    Tedax tedax[MAX_TEDAX];
    Bancada bancadas[MAX_BANCADAS];
    FilaModulos fila_modulos;

    /* Controle de sincronizacao */
    pthread_mutex_t mutex_estado;    /* Mutex principal para estado do jogo */
    pthread_mutex_t mutex_display;   /* Mutex para atualizacao de tela */
    pthread_cond_t cond_fim_jogo;    /* Condicao para fim do jogo */

    /* Threads principais */
    pthread_t thread_mural;          /* Thread geradora de modulos */
    pthread_t thread_display;        /* Thread de exibicao */
    pthread_t thread_coordenador;    /* Thread de input */
    pthread_t thread_timer;          /* Thread do temporizador */

    /* Controle de execucao */
    bool executando;                 /* Flag de execucao */
    int proximo_id_modulo;           /* Contador de IDs de modulos */

    /* Buffer de comando do jogador */
    char buffer_comando[16];
    int pos_buffer;
    pthread_mutex_t mutex_comando;

    /* Mensagens de feedback */
    char mensagem_feedback[128];
    time_t tempo_mensagem;
} EstadoJogoCompleto;

/* ==================== VARIAVEIS GLOBAIS ==================== */

extern EstadoJogoCompleto* jogo;

#endif /* TIPOS_H */
