# Keep Solving and Nobody Explodes - Versao de Treino

## Documentacao de Implementacao - Programacao Concorrente

**Instituto IDP - Ciencia da Computacao / Engenharia de Software**
**Disciplina:** Programacao Concorrente | **Professor:** Jeremias Moreira Gomes | **Periodo:** 2025/2

---

## Resumo

Este documento descreve a implementacao do jogo "Keep Solving and Nobody Explodes - Versao de Treino", um simulador desenvolvido em C que utiliza programacao concorrente para treinar habilidades de coordenacao. O sistema emprega threads POSIX para gerenciar multiplos Tedax (tecnicos de desativacao) que trabalham em paralelo para desarmar modulos de uma bomba, utilizando bancadas como recursos compartilhados sincronizados via mutex e variaveis de condicao.

---

## 1. Introducao

O jogo implementa uma versao de treino onde um jogador coordena Tedax virtuais para desarmar modulos. Diferente do jogo original cooperativo, esta versao permite treino individual com configuracoes ajustaveis de dificuldade.

**Elementos principais:**
- **Mural de Modulos:** Thread que gera novos modulos periodicamente
- **Tedax (1-3):** Threads que desarmam modulos nas bancadas
- **Bancadas (1-5):** Recursos compartilhados para trabalho
- **Display:** Thread que atualiza a interface em tempo real
- **Coordenador:** Thread principal que processa comandos do jogador

---

## 2. Arquitetura de Threads

O sistema utiliza entre 5 e 8 threads executando concorrentemente:

| Thread | Quantidade | Funcao |
|--------|------------|--------|
| Main (Coordenador) | 1 | Processa entrada do usuario |
| Mural de Modulos | 1 | Gera modulos periodicamente |
| Timer | 1 | Controla tempo da partida |
| Display | 1 | Atualiza interface ncurses |
| Tedax | 1-3 | Desarmam modulos |

As threads comunicam-se atraves de recursos compartilhados protegidos por mecanismos de sincronizacao.

---

## 3. Secoes Criticas e Sincronizacao

### 3.1 Recursos Compartilhados

| Recurso | Mutex | Condicao | Threads que Acessam |
|---------|-------|----------|---------------------|
| Fila de Modulos | `fila.mutex` | `cond_nao_vazia` | Mural, Tedax, Main |
| Bancadas | `bancada[i].mutex` | `cond_livre` | Tedax |
| Estado do Jogo | `mutex_estado` | `cond_fim_jogo` | Todas |
| Buffer de Comando | `mutex_comando` | - | Main, Display |

### 3.2 Protocolo de Acesso as Bancadas

As bancadas sao o principal recurso compartilhado. O acesso segue:

```c
// Ocupar bancada (secao critica)
pthread_mutex_lock(&bancada->mutex);
if (bancada->estado == ESTADO_LIVRE) {
    bancada->estado = ESTADO_OCUPADO;
    bancada->tedax_id = tedax_id;
    // sucesso
}
pthread_mutex_unlock(&bancada->mutex);

// Liberar bancada
pthread_mutex_lock(&bancada->mutex);
bancada->estado = ESTADO_LIVRE;
pthread_cond_broadcast(&bancada->cond_livre); // acorda threads esperando
pthread_mutex_unlock(&bancada->mutex);
```

### 3.3 Assimetria Tedax/Bancadas (Bonus)

Quando ha mais Tedax que bancadas, os Tedax aguardam em fila:

```c
while (!conseguiu_bancada && tedax->ativo) {
    if (bancada_ocupar(bancada, tedax->id, modulo)) {
        conseguiu_bancada = true;
    } else {
        bancada_aguardar_livre(bancada, 500); // aguarda com timeout
    }
}
```

---

## 4. Fluxo de Execucao

### 4.1 Ciclo do Tedax

1. Aguarda tarefa (`pthread_cond_wait` em `cond_tarefa`)
2. Recebe modulo e bancada designada
3. Tenta ocupar bancada (pode aguardar se ocupada)
4. Executa resolucao (simula tempo)
5. Verifica instrucao e atualiza estatisticas
6. Libera bancada (`pthread_cond_broadcast`)
7. Retorna ao passo 1

### 4.2 Processamento de Comando

Formato: `[TEDAX][TIPO][BANCADA][INSTRUCAO]` (ex: `1f1rgb`)

1. Parse do comando no buffer
2. Valida Tedax disponivel
3. Busca modulo do tipo na fila
4. Remove modulo e designa para Tedax
5. Sinaliza Tedax via `pthread_cond_signal`

---

## 5. Prevencao de Problemas

| Problema | Estrategia de Prevencao |
|----------|------------------------|
| **Deadlock** | Ordem consistente de locks; timeouts em esperas |
| **Race Condition** | Todo acesso compartilhado protegido por mutex |
| **Starvation** | `broadcast` ao liberar; fila FIFO |
| **Busy Waiting** | Uso de `pthread_cond_wait` em vez de polling |

---

## 6. Guia de Utilizacao

### Compilacao e Execucao

```bash
# Instalar dependencias (Ubuntu)
sudo apt-get install build-essential libncurses5-dev

# Compilar
make

# Executar
make run
```

### Como Jogar

1. **Observe os modulos pendentes** - cada um mostra: `[ID] TIPO: INSTRUCAO`
2. **Monte o comando:** `[Tedax][Tipo][Bancada][Instrucao]`
3. **Exemplo:** Se aparece `[1] f: rgb`, digite `1f1rgb` e pressione ENTER
4. **Teclas:** `p`=pausar, `h`=ajuda, `q`=sair

### Tipos de Modulos

| Tipo | Letra | Instrucao |
|------|-------|-----------|
| Fios | `f` | Cores: r(ed), g(reen), b(lue), y(ellow) |
| Botao | `b` | Pressionar: p repetido (ex: `pppp`) |
| Sequencia | `s` | Numeros: 1-4 (ex: `1234`) |
| Simon | `i` | Direcoes: u(p), d(own), l(eft), r(ight) |

---

## 7. Conclusao

O sistema implementa com sucesso os conceitos de programacao concorrente:

- **Multiplas threads** trabalhando em paralelo (5-8 threads)
- **Sincronizacao** via mutex e variaveis de condicao POSIX
- **Secoes criticas** protegidas para acesso a recursos compartilhados
- **Assimetria Tedax/Bancadas** com fila de espera implementada
- **Interface ncurses** atualizada por thread dedicada

A arquitetura modular em 6 arquivos fonte facilita manutencao e extensao.

---

## Referencias

1. TANENBAUM, Andrew S. Sistemas Operacionais Modernos. 3a ed. 2010.
2. POSIX Threads Programming - Lawrence Livermore National Laboratory
3. Ncurses Programming HOWTO - Linux Documentation Project
