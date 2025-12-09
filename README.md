# Keep Solving and Nobody Explodes - Versao de Treino

## Trabalho Pratico - Programacao Concorrente
**Instituto IDP - 2025/2**

**Disciplina:** Programacao Concorrente
**Professor:** Jeremias Moreira Gomes

---

## Descricao

Este projeto implementa uma versao de treino do jogo "Keep Solving and Nobody Explodes", onde o jogador deve coordenar Tedax (Tecnicos Especialistas em Desativacao de Artefatos Explosivos) virtuais para desarmar modulos de uma bomba antes que o tempo acabe.

O jogo foi desenvolvido em C utilizando programacao concorrente (threads POSIX) e a biblioteca ncurses para interface grafica em terminal.

---

## Caracteristicas

- **Programacao Concorrente:** Multiplas threads trabalhando em paralelo
- **Sincronizacao:** Uso de mutex e variaveis de condicao
- **Interface Grafica:** Terminal com ncurses
- **Configuravel:** Numero de Tedax, bancadas, tempo e dificuldade
- **Assimetria Tedax/Bancadas:** Suporte a quantidades diferentes (bonus)

### Threads Implementadas

1. **Thread do Mural de Modulos** - Gera novos modulos durante a partida
2. **Thread de Display** - Atualiza a interface grafica
3. **Thread do Timer** - Controla o tempo da partida
4. **Threads dos Tedax** - Cada Tedax e uma thread independente (1-3 threads)

---

## Requisitos

### Sistema Operacional
- Linux (testado em Ubuntu 24.04)

### Dependencias
- GCC (compilador C)
- libncurses (biblioteca de interface)
- pthread (threads POSIX - geralmente incluido no GCC)

### Instalacao das Dependencias (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev
```

Ou utilize o comando do Makefile:

```bash
make install-deps
```

---

## Compilacao

### Compilacao Padrao

```bash
make
```

### Compilacao com Debug

```bash
make debug
```

### Compilacao Otimizada (Release)

```bash
make release
```

### Limpar Arquivos Compilados

```bash
make clean
```

### Verificar Dependencias

```bash
make check-deps
```

---

## Execucao

### Executar o Jogo

```bash
./bomb_defuser
```

Ou compilar e executar:

```bash
make run
```

---

## Como Jogar

### Objetivo

Desarme todos os modulos da bomba antes que o tempo acabe! Voce deve coordenar os Tedax (tecnicos) para resolver cada modulo.

### Interface do Jogo

```
+--------------------------------------------------+
|              MODULOS PENDENTES                   |
| [1] f: rgb    [2] b: pppp    [3] s: 1234         |
+--------------------------------------------------+
|                   BANCADAS                       |
| Bancada 1        Bancada 2        Bancada 3      |
| [LIVRE]          [OCUPADA]        [LIVRE]        |
+--------------------------------------------------+
|                    TEDAX                         |
| Tedax 1          Tedax 2          Tedax 3        |
| [DISPONIVEL]     [TRABALHANDO]    [AGUARDANDO]   |
+--------------------------------------------------+
|                    STATUS                        |
| TEMPO: 01:45    Desarmados: 5    Falhas: 2       |
+--------------------------------------------------+
| COMANDO: > 1f1rgb_                               |
+--------------------------------------------------+
```

### Formato dos Comandos

Os comandos seguem o formato: `[TEDAX][TIPO][BANCADA][INSTRUCAO]`

**Exemplo:** `1f1rgb`
- `1` - Tedax numero 1
- `f` - Modulo tipo "Fios"
- `1` - Usar Bancada 1
- `rgb` - Instrucao: cortar fios vermelho, verde, azul

### Tipos de Modulos

| Tipo | Letra | Descricao | Exemplo de Instrucao |
|------|-------|-----------|---------------------|
| Fios | `f` | Cortar fios na ordem correta | `rgb`, `ygb` (r=red, g=green, b=blue, y=yellow) |
| Botao | `b` | Pressionar botao N vezes | `pppp` (4 pressionamentos) |
| Sequencia | `s` | Digitar sequencia numerica | `1234`, `2143` |
| Simon | `i` | Repetir direcoes | `udlr` (u=up, d=down, l=left, r=right) |

### Teclas de Controle

| Tecla | Acao |
|-------|------|
| `ENTER` | Executar comando |
| `BACKSPACE` | Apagar caractere |
| `ESC` | Limpar comando |
| `p` | Pausar/Despausar |
| `h` | Ajuda |
| `q` | Sair do jogo |

### Menu Principal

- Use as **setas** ou **W/S** para navegar
- Pressione **ENTER** ou o **numero** da opcao para selecionar

### Configuracoes

| Opcao | Valores | Descricao |
|-------|---------|-----------|
| Tedax | 1-3 | Quantidade de tecnicos |
| Bancadas | 1-5 | Quantidade de bancadas de trabalho |
| Tempo | 30-300s | Duracao da partida |
| Dificuldade | 1-3 | Afeta velocidade de geracao e tempo de resolucao |
| Modulos para Vencer | 5-50 | Quantidade necessaria para vencer |
| Modo Infinito | Sim/Nao | Jogo sem limite de modulos |

---

## Estrutura do Projeto

```
Trabalho-Pratico-Programacao-Concorrente/
├── include/                # Arquivos de cabecalho
│   ├── tipos.h            # Definicoes de tipos e estruturas
│   ├── modulos.h          # Interface do sistema de modulos
│   ├── tedax.h            # Interface dos Tedax
│   ├── bancada.h          # Interface das bancadas
│   ├── display.h          # Interface grafica (ncurses)
│   └── jogo.h             # Controle geral do jogo
├── src/                    # Codigo fonte
│   ├── main.c             # Ponto de entrada e loop principal
│   ├── jogo.c             # Logica do jogo e threads principais
│   ├── modulos.c          # Implementacao dos modulos e fila
│   ├── tedax.c            # Implementacao dos Tedax (threads)
│   ├── bancada.c          # Implementacao das bancadas
│   └── display.c          # Interface com ncurses
├── obj/                    # Arquivos objeto (gerado)
├── Makefile               # Script de compilacao
├── README.md              # Este arquivo
├── DOCUMENTACAO.md        # Documentacao tecnica
└── bomb_defuser           # Executavel (gerado)
```

---

## Arquitetura de Threads

### Diagrama de Threads

```
                    +---------------+
                    |     main()    |
                    +-------+-------+
                            |
            +---------------+---------------+
            |               |               |
    +-------v-------+ +-----v-----+ +-------v-------+
    | Thread Mural  | | Thread    | | Thread Timer  |
    | (geracao)     | | Display   | | (contagem)    |
    +---------------+ +-----------+ +---------------+
            |
    +-------+-------+-------+
    |       |       |       |
+---v---+ +-v---+ +-v---+ +-v---+
| Tedax | |Tedax| |Tedax| | ... |
|   1   | |  2  | |  3  | |     |
+-------+ +-----+ +-----+ +-----+
```

### Recursos Compartilhados e Secoes Criticas

1. **Fila de Modulos** (`FilaModulos`)
   - Mutex: `fila.mutex`
   - Acesso: Thread Mural (escrita), Coordenador (leitura/escrita), Display (leitura)

2. **Bancadas** (`Bancada[]`)
   - Mutex: `bancada.mutex` (uma por bancada)
   - Condition: `bancada.cond_livre`
   - Acesso: Threads Tedax (leitura/escrita)

3. **Estado do Jogo** (`EstadoJogoCompleto`)
   - Mutex: `mutex_estado`
   - Acesso: Todas as threads

4. **Display**
   - Mutex: `mutex_display`
   - Acesso: Thread Display (escrita), outras (sinalizacao)

---

## Sincronizacao

### Mutex Utilizados

| Mutex | Protege | Threads que Acessam |
|-------|---------|---------------------|
| `mutex_estado` | Estado geral do jogo | Todas |
| `mutex_display` | Atualizacao de tela | Display, Main |
| `mutex_comando` | Buffer de comando | Main, Display |
| `fila.mutex` | Fila de modulos | Mural, Tedax, Main |
| `bancada[i].mutex` | Cada bancada | Tedax |
| `tedax[i].mutex` | Cada Tedax | Tedax, Main |

### Variaveis de Condicao

| Condition | Finalidade |
|-----------|------------|
| `cond_fim_jogo` | Sinaliza fim da partida |
| `bancada.cond_livre` | Sinaliza bancada disponivel |
| `tedax.cond_tarefa` | Sinaliza nova tarefa para Tedax |

---

## Bonus: Assimetria Tedax/Bancadas

O jogo implementa a funcionalidade de bonus onde o numero de Tedax e bancadas pode ser diferente:

- **Tedax > Bancadas:** Tedax aguardam em fila por bancadas livres
- **Tedax < Bancadas:** Bancadas podem ficar ociosas

A fila de espera por bancadas e implementada utilizando a variavel de condicao `bancada.cond_livre`, onde os Tedax aguardam ate que uma bancada fique disponivel.

---

## Dicas de Jogo

1. **Observe as instrucoes:** Cada modulo mostra a instrucao correta entre colchetes
2. **Gerencie os Tedax:** Nao deixe todos ocupados ao mesmo tempo
3. **Priorize modulos antigos:** Evite acumulo na fila
4. **Use a pausa:** Pressione `p` para pensar com calma
5. **Atencao ao tempo:** O jogo termina se a fila encher ou o tempo acabar

---

## Creditos

Desenvolvido como trabalho pratico para a disciplina de Programacao Concorrente.

**Instituto:** IDP - Instituto Brasileiro de Ensino, Desenvolvimento e Pesquisa
**Curso:** Ciencia da Computacao / Engenharia de Software
**Periodo:** 2025/2

---

## Licenca

Este projeto foi desenvolvido para fins educacionais.
