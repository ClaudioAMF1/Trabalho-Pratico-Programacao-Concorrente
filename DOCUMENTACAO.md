# Documentacao Tecnica

## Keep Solving and Nobody Explodes - Versao de Treino

**Trabalho Pratico - Programacao Concorrente**
**Instituto IDP - 2025/2**

---

## 1. Introducao

Este documento descreve a arquitetura e implementacao do jogo "Keep Solving and Nobody Explodes - Versao de Treino", desenvolvido como trabalho pratico da disciplina de Programacao Concorrente.

O sistema foi implementado em linguagem C, utilizando threads POSIX (pthreads) para concorrencia e a biblioteca ncurses para interface grafica em terminal.

---

## 2. Arquitetura do Sistema

### 2.1 Visao Geral

O sistema e composto por multiplas threads que executam concorrentemente:

```
+------------------+     +------------------+     +------------------+
|   Thread Main    |     |   Thread Mural   |     |   Thread Timer   |
|  (Coordenador)   |     | (Gera Modulos)   |     | (Tempo Restante) |
+--------+---------+     +--------+---------+     +--------+---------+
         |                        |                        |
         |    +------------------+|+------------------+    |
         |    |                   v                   |    |
         +--->|    RECURSOS COMPARTILHADOS           |<---+
              |  - Fila de Modulos                   |
              |  - Estado do Jogo                    |
              |  - Bancadas                          |
              +------------------+--------------------+
                                 |
         +-----------+-----------+-----------+
         |           |           |           |
+--------v---+ +-----v------+ +--v-------+ +-v----------+
| Thread     | | Thread     | | Thread   | | Thread     |
| Tedax 1    | | Tedax 2    | | Tedax 3  | | Display    |
+------------+ +------------+ +----------+ +------------+
```

### 2.2 Threads do Sistema

| Thread | Funcao | Arquivo |
|--------|--------|---------|
| Main/Coordenador | Processa entrada do usuario e coordena o jogo | `main.c` |
| Mural de Modulos | Gera novos modulos periodicamente | `modulos.c` |
| Timer | Controla tempo da partida | `jogo.c` |
| Display | Atualiza interface grafica | `display.c` |
| Tedax (1-3) | Desarmam modulos nas bancadas | `tedax.c` |

---

## 3. Estruturas de Dados

### 3.1 Modulo

Representa um modulo da bomba a ser desarmado:

```c
typedef struct {
    int id;                     // Identificador unico
    TipoModulo tipo;            // FIOS, BOTAO, SEQUENCIA, SIMON
    char nome[32];              // Nome para exibicao
    int tempo_resolucao;        // Tempo em segundos
    char instrucao[64];         // Instrucao correta
    bool resolvido;             // Status de resolucao
} Modulo;
```

### 3.2 Bancada

Representa uma bancada de trabalho (recurso compartilhado):

```c
typedef struct {
    int id;
    Estado estado;              // LIVRE ou OCUPADO
    Modulo* modulo_atual;
    int tedax_id;               // Tedax usando (-1 se livre)
    pthread_mutex_t mutex;      // Protege acesso
    pthread_cond_t cond_livre;  // Sinaliza disponibilidade
} Bancada;
```

### 3.3 Tedax

Representa um tecnico de desativacao (thread):

```c
typedef struct {
    int id;
    Estado estado;              // LIVRE, OCUPADO, AGUARDANDO
    Modulo* modulo_atual;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond_tarefa; // Sinaliza nova tarefa
    bool tarefa_pendente;
} Tedax;
```

### 3.4 Fila de Modulos

Fila circular thread-safe para modulos pendentes:

```c
typedef struct {
    Modulo modulos[MAX_MODULOS];
    int inicio, fim, quantidade;
    pthread_mutex_t mutex;
    pthread_cond_t cond_nao_vazia;
    pthread_cond_t cond_nao_cheia;
} FilaModulos;
```

---

## 4. Secoes Criticas e Sincronizacao

### 4.1 Identificacao das Secoes Criticas

| Secao Critica | Recursos Protegidos | Mutex |
|---------------|---------------------|-------|
| Fila de Modulos | Adicao/remocao de modulos | `fila.mutex` |
| Estado do Jogo | Estatisticas, estado atual | `mutex_estado` |
| Bancadas | Ocupacao/liberacao | `bancada[i].mutex` |
| Estado Tedax | Atribuicao de tarefas | `tedax[i].mutex` |
| Display | Atualizacao de tela | `mutex_display` |
| Buffer Comando | Entrada do usuario | `mutex_comando` |

### 4.2 Protocolo de Sincronizacao das Bancadas

O acesso as bancadas segue o protocolo:

```c
// Thread Tedax tentando ocupar bancada
bool bancada_ocupar(Bancada* bancada, int tedax_id, Modulo* modulo) {
    pthread_mutex_lock(&bancada->mutex);

    if (bancada->estado != ESTADO_LIVRE) {
        pthread_mutex_unlock(&bancada->mutex);
        return false;  // Bancada ocupada
    }

    // Secao critica: modifica estado da bancada
    bancada->estado = ESTADO_OCUPADO;
    bancada->tedax_id = tedax_id;
    bancada->modulo_atual = modulo;

    pthread_mutex_unlock(&bancada->mutex);
    return true;
}

// Thread Tedax liberando bancada
bool bancada_liberar(Bancada* bancada, int tedax_id) {
    pthread_mutex_lock(&bancada->mutex);

    if (bancada->tedax_id != tedax_id) {
        pthread_mutex_unlock(&bancada->mutex);
        return false;  // Nao e o dono
    }

    bancada->estado = ESTADO_LIVRE;
    bancada->tedax_id = -1;
    bancada->modulo_atual = NULL;

    // Acorda threads aguardando
    pthread_cond_broadcast(&bancada->cond_livre);

    pthread_mutex_unlock(&bancada->mutex);
    return true;
}
```

### 4.3 Fila de Espera por Bancadas (Assimetria)

Quando um Tedax precisa de uma bancada ocupada:

```c
void* thread_tedax(void* arg) {
    // ...
    while (!conseguiu_bancada && tedax->ativo) {
        if (bancada_ocupar(bancada, tedax->id, modulo)) {
            conseguiu_bancada = true;
        } else {
            // Aguarda bancada ficar livre (com timeout)
            bancada_aguardar_livre(bancada, 500);
        }
    }
    // ...
}
```

### 4.4 Variaveis de Condicao

| Variavel | Uso | Sinalizacao |
|----------|-----|-------------|
| `bancada.cond_livre` | Tedax aguarda bancada | `pthread_cond_broadcast` ao liberar |
| `tedax.cond_tarefa` | Tedax aguarda tarefa | `pthread_cond_signal` ao designar |
| `cond_fim_jogo` | Threads aguardam fim | `pthread_cond_broadcast` ao terminar |

---

## 5. Fluxo de Execucao

### 5.1 Ciclo de Vida do Tedax

```
[INICIO] -> [AGUARDANDO TAREFA] -> [RECEBE TAREFA]
                    ^                     |
                    |                     v
            [LIBERA BANCADA] <- [OCUPA BANCADA]
                    ^                     |
                    |                     v
            [RESULTADO] <---- [RESOLVE MODULO]
```

### 5.2 Processamento de Comando

1. Usuario digita comando (ex: `1f1rgb`)
2. Main thread faz parse do comando
3. Valida Tedax, tipo de modulo e bancada
4. Busca modulo do tipo na fila
5. Remove modulo da fila
6. Designa modulo para o Tedax
7. Tedax recebe tarefa via `cond_tarefa`

### 5.3 Resolucao de Modulo

1. Tedax tenta ocupar bancada designada
2. Se ocupada, aguarda em `cond_livre`
3. Quando livre, ocupa bancada
4. Simula tempo de resolucao
5. Verifica se instrucao esta correta
6. Atualiza estatisticas
7. Libera bancada
8. Se falhou, retorna modulo para fila

---

## 6. Prevencao de Problemas

### 6.1 Deadlock

**Prevencao:**
- Ordem consistente de aquisicao de locks
- Uso de timeouts em esperas
- Locks de curta duracao

### 6.2 Race Conditions

**Prevencao:**
- Todo acesso a dados compartilhados protegido por mutex
- Variaveis de condicao para sinalizacao
- Copia de dados antes de liberar lock

### 6.3 Starvation

**Prevencao:**
- `pthread_cond_broadcast` para acordar todos
- Timeout em esperas por bancadas
- Fila FIFO para modulos

---

## 7. Guia de Instalacao e Execucao

### 7.1 Requisitos

- Sistema Operacional: Linux (Ubuntu 24.04 recomendado)
- Compilador: GCC com suporte a C99
- Bibliotecas: libncurses, pthreads

### 7.2 Instalacao de Dependencias

```bash
sudo apt-get update
sudo apt-get install build-essential libncurses5-dev
```

### 7.3 Compilacao

```bash
make clean
make
```

### 7.4 Execucao

```bash
./bomb_defuser
```

---

## 8. Conclusao

O sistema implementa com sucesso os conceitos de programacao concorrente:

- **Multiplas threads** executando em paralelo
- **Sincronizacao** via mutex e variaveis de condicao
- **Secoes criticas** devidamente protegidas
- **Comunicacao entre threads** via recursos compartilhados
- **Assimetria Tedax/Bancadas** com fila de espera

A arquitetura modular permite facil extensao e manutencao do codigo.
