# Keep Solving and Nobody Explodes

Simulador de treinamento para desativacao de bombas com programacao concorrente.

**Aluno:** Claudio da Aparecida Meireles Filho

**RA:** 2321070

**Disciplina:** Programacao Concorrente - IDP 2025/2

**Professor:** Jeremias Moreira Gomes

---

## Sobre o Projeto

Jogo de terminal onde voce assume o papel de coordenador de uma equipe de tecnicos especialistas em desativacao de bombas (Tedax). Seu objetivo e designar os tecnicos para desarmar modulos antes que o tempo acabe ou que muitos modulos se acumulem.

O jogo implementa conceitos de programacao concorrente:
- Cada Tedax opera como uma thread independente
- As bancadas de trabalho sao recursos compartilhados
- Sincronizacao via mutex e variaveis de condicao
- Fila de modulos thread-safe com produtor/consumidor
- Threads de temporizador e mural operando em paralelo com a thread principal

---

## Requisitos

- Sistema Linux
- GCC (compilador C)
- Biblioteca ncurses
- Make

---

## Compilacao e Execucao

```bash
# Instalar dependencias (Ubuntu/Debian)
sudo apt install build-essential libncurses5-dev

# Compilar o projeto
make

# Executar o jogo
make run

# Ou executar diretamente
./bomb_defuser

# Limpar arquivos compilados
make clean
```

> Observacao: se estiver em um ambiente sem internet (como o avaliador automatico), as dependencias de compilacao ja estao
> presentes. Basta executar `make` e `make run` diretamente.

---

## Entrega (conforme enunciado)

- Gere um arquivo `.zip` contendo este repositório (sem a pasta `.git/`) e a documentação `ARTIGO_SBC.md`.
- Calcule o `md5sum` do `.zip` e renomeie o arquivo incluindo as matrículas e o hash, por exemplo: `2321070-<hash>.zip`.
- A submissão deve ser feita no Ambiente Virtual da disciplina dentro do prazo informado pelo professor.

---

## Como Jogar

### Objetivo

Desarme a quantidade necessaria de modulos antes que:
- O tempo acabe (derrota)
- A fila de modulos pendentes fique cheia (derrota)

### Entendendo a Interface

```
MODULOS PENDENTES - Copie a INSTRUCAO para seu comando!
[1] f: rgb    [2] b: pppp    [3] s: 1234
     ^  ^^^
     |  instrucao (copie exatamente isso!)
     tipo do modulo (f=fios, b=botao, s=sequencia, i=simon)

BANCADAS
Bancada 1      Bancada 2
[LIVRE]        [OCUPADA]

TEDAX
Tedax 1        Tedax 2
[DISPONIVEL]   [TRABALHANDO]
```

### Montando um Comando

O formato do comando e: `[Tedax][Tipo][Bancada][Instrucao]`

**Exemplo passo a passo:**
1. Aparece na tela: `[1] f: rgb`
2. Isso significa: modulo tipo Fios com instrucao `rgb`
3. Verifique um Tedax disponivel (ex: Tedax 1)
4. Verifique uma bancada livre (ex: Bancada 1)
5. Monte o comando: `1f1rgb`
6. Pressione ENTER

O Tedax 1 ira para a Bancada 1 e tentara desarmar o modulo de fios cortando os fios vermelho, verde e azul (rgb).

### Tipos de Modulos

| Tipo | Letra | Descricao | Exemplo de Instrucao |
|------|-------|-----------|---------------------|
| Fios | f | Cortar fios coloridos | `rgb` (vermelho, verde, azul) |
| Botao | b | Pressionar botao N vezes | `pppp` (4 pressionamentos) |
| Sequencia | s | Digitar sequencia numerica | `1234` |
| Simon | i | Repetir direcoes | `udlr` (cima, baixo, esq, dir) |

### Teclas de Controle

| Tecla | Funcao |
|-------|--------|
| Letras/numeros | Digitar comando |
| ENTER | Enviar comando |
| BACKSPACE | Apagar ultimo caractere |
| ESC | Limpar comando |
| p | Pausar/continuar jogo |
| h | Mostrar tela de ajuda |
| q | Sair do jogo |
| Ctrl+C | Forca a saida, mostra motivo final |

### Dicas

1. **Observe a instrucao correta**: Cada modulo mostra sua instrucao na tela. Copie exatamente!
2. **Gerencie os Tedax**: Nao envie um Tedax ocupado para outra tarefa
3. **Bancadas sao compartilhadas**: Se uma bancada esta ocupada, o Tedax aguarda na fila
4. **Fique atento ao tempo**: O temporizador nao para, seja rapido!
5. **Nao deixe acumular**: Se a fila encher, voce perde

---

## Configuracoes

No menu de configuracoes voce pode ajustar:

| Opcao | Valores | Padrao |
|-------|---------|--------|
| Numero de Tedax | 1-3 | 2 |
| Numero de Bancadas | 1-5 | 2 |
| Tempo da Partida | segundos | 120 |
| Dificuldade | 1-3 | 1 |
| Modulos para Vencer | quantidade | 10 |
| Modo Infinito | Sim/Nao | Nao |

Cada alteracao e validada na interface e aplicada imediatamente ao iniciar uma nova partida. O modo infinito ignora a meta de
modulos para vencer, mantendo o jogo ate ocorrer uma derrota por tempo ou fila cheia.

---

## Arquitetura do Sistema

### Threads

| Thread | Funcao |
|--------|--------|
| Main | Loop de entrada, processa comandos do usuario |
| Mural | Gera modulos aleatorios periodicamente |
| Timer | Decrementa tempo restante |
| Display | Atualiza interface a cada 100ms |
| Tedax (1-3) | Cada tecnico e uma thread que processa modulos |

### Sincronizacao

- **mutex_estado**: Protege variaveis do estado do jogo
- **mutex_display**: Garante atualizacao atomica da tela
- **mutex_comando**: Protege buffer de entrada
- **mutex (fila)**: Protege fila circular de modulos
- **mutex (bancada)**: Protege cada bancada individualmente
- **mutex (tedax)**: Protege estado de cada tecnico
- **cond_livre**: Sinaliza quando bancada fica disponivel
- **cond_tarefa**: Sinaliza nova tarefa para tedax

### Estrutura de Arquivos

```
Trabalho-Pratico-Programacao-Concorrente/
├── include/
│   ├── tipos.h       # Estruturas de dados
│   ├── modulos.h     # Interface da fila de modulos
│   ├── tedax.h       # Interface dos tecnicos
│   ├── bancada.h     # Interface das bancadas
│   ├── display.h     # Interface grafica
│   └── jogo.h        # Controle do jogo
├── src/
│   ├── main.c        # Ponto de entrada e loop principal
│   ├── jogo.c        # Logica do jogo e threads
│   ├── modulos.c     # Fila thread-safe de modulos
│   ├── tedax.c       # Implementacao dos tecnicos
│   ├── bancada.c     # Gerenciamento de bancadas
│   └── display.c     # Interface ncurses
├── Makefile          # Sistema de compilacao
├── README.md         # Este arquivo
└── ARTIGO_SBC.md     # Documentacao tecnica detalhada
```

---

## Bonus Implementado

**Assimetria Tedax/Bancadas**: O sistema suporta configuracoes onde o numero de Tedax difere do numero de bancadas. Quando ha mais Tedax que bancadas, os tecnicos aguardam em fila pela liberacao de uma bancada, implementando um sistema de espera com variaveis de condicao.

---

## Checklist de Requisitos do Trabalho

- **Uso de threads POSIX:** main, mural, timer, display e ate 3 threads de Tedax trabalham concorrentemente.
- **Fila e bancadas protegidas:** todas as estruturas compartilhadas utilizam mutex e variaveis de condicao para evitar race conditions e busy waiting.
- **Tratamento de encerramento limpo:** flags de controle e `pthread_join` garantem que as threads terminem antes de liberar a interface ncurses.
- **Documentacao completa:** este README resume execucao e gameplay; o arquivo `ARTIGO_SBC.md` apresenta a documentacao tecnica solicitada com nome e RA do aluno.
- **Configuracoes obrigatorias:** numero de Tedax, bancadas, tempo, meta de modulos e dificuldade podem ser ajustados no menu de configuracao do jogo, conforme solicitado no enunciado.

---

## Referencias

- Tanenbaum, A. S. - Sistemas Operacionais Modernos
- Silberschatz, A. - Fundamentos de Sistemas Operacionais
- POSIX Threads Programming - Lawrence Livermore National Laboratory
- ncurses Programming HOWTO

---

## Licenca

Trabalho academico desenvolvido para a disciplina de Programacao Concorrente do IDP.
