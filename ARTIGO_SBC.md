# Keep Solving and Nobody Explodes - Versao de Treino

## Documentacao de Implementacao - Programacao Concorrente

**Aluno:** Claudio da Aparecida Meireles Filho | **RA:** 2321070  
**Instituto IDP - Ciencia da Computacao / Engenharia de Software**  
**Disciplina:** Programacao Concorrente | **Professor:** Jeremias Moreira Gomes | **Periodo:** 2025/2

---

## Resumo

Simulador em C que utiliza threads POSIX para coordenar Tedax virtuais na desativacao de modulos de uma bomba. O projeto destaca sincronizacao por mutex e variaveis de condicao, evitando race conditions e busy waiting, alem de explorar configuracoes assimetricas entre Tedax e bancadas.

---

## 1. Arquitetura de Threads

Executam-se entre 5 e 8 threads simultaneamente:

| Thread | Quantidade | Funcao |
|--------|------------|--------|
| Main (Coordenador) | 1 | Processa entrada e monta comandos |
| Mural de Modulos | 1 | Gera modulos aleatorios periodicamente |
| Timer | 1 | Decrementa tempo e sinaliza fim |
| Display | 1 | Atualiza interface ncurses a cada 100ms |
| Tedax | 1-3 | Desarmam modulos nas bancadas |

As threads compartilham fila de modulos, bancadas, estado do jogo e buffer de entrada.

---

## 2. Secoes Criticas e Sincronizacao

| Recurso Compartilhado | Mutex | Condicao | Threads |
|-----------------------|-------|----------|---------|
| Fila de Modulos | `fila.mutex` | `cond_nao_vazia` | Mural, Tedax, Main |
| Bancadas | `bancada[i].mutex` | `cond_livre` | Tedax |
| Estado do Jogo | `mutex_estado` | `cond_fim_jogo` | Todas |
| Buffer de Comando | `mutex_comando` | - | Main, Display |

**Protocolo das Bancadas**

1. `pthread_mutex_lock` protege o teste de disponibilidade.  
2. Ao ocupar, registra `estado` e `tedax_id`.  
3. Ao liberar, `pthread_cond_broadcast(&cond_livre)` desperta Tedax aguardando.

**Assimetria (Bonus)**

Se houver mais Tedax que bancadas, cada Tedax aguarda em `cond_livre` com timeout controlado, evitando busy waiting e garantindo fila justa.

---

## 3. Fluxo de Execucao

**Ciclo do Tedax**

1. Aguarda tarefa em `cond_tarefa`.  
2. Recebe modulo e bancada designada.  
3. Tenta ocupar bancada; se ocupada, espera sinalizacao.  
4. Simula resolucao, verifica instrucao e atualiza estatisticas protegidas por `mutex_estado`.  
5. Libera bancada e volta ao passo 1.

**Processamento de Comandos (Main)**

1. Le buffer com `mutex_comando` e valida formato `[TEDAX][TIPO][BANCADA][INSTRUCAO]`.  
2. Checa Tedax disponivel e busca modulo do tipo.  
3. Remove modulo da fila, atribui ao Tedax e sinaliza `cond_tarefa`.  
4. Mantem flags `executando` e `resultado_final` sob `mutex_estado` para encerramento consistente.

---

## 4. Prevencao de Problemas

| Risco | Mitigacao |
|-------|-----------|
| Deadlock | Ordem previsivel de locks; liberacao garantida antes de esperar condicoes |
| Race Condition | Toda variavel compartilhada protegida por mutex dedicado |
| Starvation | `broadcast` ao liberar bancadas e fila FIFO de modulos |
| Busy Waiting | Espera bloqueante em condicoes para mural, Tedax e bancadas |

Flags de controle sao atualizadas sob `mutex_estado`, e `pthread_join` sincroniza todas as threads antes de restaurar a tela ncurses.

---

## 5. Guia de Utilizacao

**Compilar e Executar (Ubuntu 24.04)**

```bash
sudo apt install build-essential libncurses5-dev
make
make run   # ou ./bomb_defuser
```

**Como Jogar**

1. Observe os modulos pendentes: `[ID] TIPO: INSTRUCAO`.  
2. Monte o comando `[Tedax][Tipo][Bancada][Instrucao]` (ex.: `1f1rgb`).  
3. Envie com ENTER. Use `p` para pausar, `h` para ajuda, `q` para sair e `Ctrl+C` para forcar encerramento exibindo o motivo final.

**Tipos de Modulos**

| Tipo | Letra | Instrucao |
|------|-------|-----------|
| Fios | `f` | Cores: r, g, b, y (ex.: `rgb`) |
| Botao | `b` | Pressionamentos: `p` repetido (ex.: `pppp`) |
| Sequencia | `s` | Numeros 1-4 (ex.: `1234`) |
| Simon | `i` | Direcoes u, d, l, r (ex.: `udlr`) |

**Configuracoes Disponiveis**

- Tedax: 1 a 3  
- Bancadas: 1 a 5  
- Tempo da partida (s)  
- Meta de modulos para vencer (pode ser ignorada no modo infinito)  
- Dificuldade (1-3) ajusta geracao de modulos

---

## 6. Validacao e Conformidade

- **Threads obrigatorias:** main, mural, timer, display e ate 3 Tedax em paralelo (>=5 threads).  
- **Sincronizacao:** fila de modulos, bancadas, estado global e buffer de comando protegidos por mutex e condicoes, sem busy waiting.  
- **Bonus de assimetria:** bloqueio com condicao quando ha mais Tedax que bancadas, garantindo acesso justo.  
- **Encerramento limpo:** flags sob mutex, sinalizacao de fim e `pthread_join` antes de liberar ncurses.  
- **Documentacao:** README traz guia de instalacao/execucao e este artigo resume arquitetura e decisoes com identificacao do autor (Claudio da Aparecida Meireles Filho, RA 2321070).  
- **Testes:** `make` compila o projeto com as mesmas flags do avaliador.

---

## 7. Validacao e Conformidade com o Enunciado

- **Threads requeridas:** main, mural, timer, display e ate 3 Tedax executam concorrentemente, cobrindo o minimo de cinco
  threads exigido.
- **Recursos compartilhados:** fila de modulos, bancadas, estado do jogo e buffer de entrada sao protegidos por mutex e variaveis
  de condicao, evitando race conditions.
- **Fila de espera (bonus):** quando ha mais Tedax que bancadas, ha bloqueio e sinalizacao por condicao para eliminar busy
  waiting, conforme opcional de assimetria.
- **Encerramento consistente:** flags de controle sao atualizadas com mutex, e as threads sao sincronizadas via `pthread_join`
  antes de restaurar a tela, evitando deadlocks na finalizacao.
- **Documentacao completa:** README detalha compilacao, gameplay e configuracoes; este artigo apresenta arquitetura e decisoes
  tecnicas com nome e RA do autor (Claudio da Aparecida Meireles Filho, RA 2321070).
- **Testes executados:** `make` valida a construcao de todo o projeto e depende das mesmas flags utilizadas pelo avaliador.

---

## 8. Conclusao

O projeto demonstra aplicacao pratica de programacao concorrente em C: uso de multiplas threads coordenadas, secoes criticas protegidas, sincronizacao por mutex/condicoes e exploracao de configuracoes assimetricas. A modularizacao em arquivos separados facilita manutencao e extensao para novos tipos de modulos ou politicas de agendamento.

---

## Referencias

1. TANENBAUM, Andrew S. Sistemas Operacionais Modernos. 3a ed. 2010.  
2. POSIX Threads Programming - Lawrence Livermore National Laboratory.  
3. Ncurses Programming HOWTO - Linux Documentation Project.
