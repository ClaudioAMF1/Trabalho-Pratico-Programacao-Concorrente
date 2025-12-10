# Guia Completo - Keep Solving and Nobody Explodes

## Como Jogar - Passo a Passo

---

## 1. Iniciando o Jogo

### Compilar e Executar

```bash
# Primeiro, compile o jogo
make

# Execute o jogo
./bomb_defuser
```

### Menu Principal

Ao iniciar, voce vera o menu principal:

```
  _  __                  ____        _       _
 | |/ /___  ___ _ __   / ___|  ___ | |_   _(_)_ __   __ _
 | ' // _ \/ _ \ '_ \  \___ \ / _ \| \ \ / / | '_ \ / _` |
 | . \  __/  __/ |_) |  ___) | (_) | |\ V /| | | | | (_| |
 |_|\_\___|\___| .__/  |____/ \___/|_| \_/ |_|_| |_|\__, |
              |_|                                   |___/
        *** AND NOBODY EXPLODES - Versao de Treino ***

                    1. Iniciar Jogo
                    2. Configuracoes
                    3. Ajuda
                    4. Sair
```

**Navegacao:**
- `Setas CIMA/BAIXO` ou `W/S` - Mover selecao
- `ENTER` - Confirmar opcao
- `1, 2, 3, 4` - Selecao direta

---

## 2. Configuracoes

Antes de jogar, voce pode ajustar as configuracoes:

| Opcao | Valores | O que faz |
|-------|---------|-----------|
| **Tedax** | 1-3 | Quantidade de tecnicos disponiveis |
| **Bancadas** | 1-5 | Mesas de trabalho para desarmar |
| **Tempo** | 30-300s | Duracao da partida |
| **Dificuldade** | 1-3 | Velocidade de geracao de modulos |
| **Modulos para Vencer** | 5-50 | Meta para ganhar |
| **Modo Infinito** | Sim/Nao | Jogo sem meta de modulos |

**Navegacao nas Configuracoes:**
- `Setas CIMA/BAIXO` - Mudar campo
- `Setas ESQUERDA/DIREITA` - Alterar valor
- `ENTER` em "Voltar" ou `ESC` - Voltar ao menu

---

## 3. Entendendo a Tela do Jogo

```
╔══════════════════════════════════════════════════════════════════════╗
║     KEEP SOLVING AND NOBODY EXPLODES - Versao de Treino             ║
╠══════════════════════════════════════════════════════════════════════╣
║  MODULOS PENDENTES                                          [3/10]  ║
║  [1] f: rgb      [2] b: pppp      [5] s: 1234                       ║
║                                                                      ║
╠══════════════════════════════════════════════════════════════════════╣
║  BANCADAS                                                            ║
║  Bancada 1          Bancada 2                                        ║
║  [LIVRE]            [OCUPADA]                                        ║
║                     Modulo: Fios #3                                  ║
║                     Tedax: 1                                         ║
╠══════════════════════════════════════════════════════════════════════╣
║  TEDAX                                                               ║
║  Tedax 1            Tedax 2                                          ║
║  [TRABALHANDO]      [DISPONIVEL]                                     ║
║  OK: 2  Falha: 0    OK: 1  Falha: 1                                  ║
╠══════════════════════════════════════════════════════════════════════╣
║  STATUS                                                              ║
║  TEMPO: 01:45      Desarmados: 3      Falhas: 1                      ║
║  >> Tedax 1 desarmando Fios #3 na bancada 2...                       ║
╠══════════════════════════════════════════════════════════════════════╣
║  COMANDO                                                             ║
║  > _                                                                 ║
║  [TEDAX][TIPO][BANCADA][INSTRUCAO]  Ex: 1f1rgb | 'q'=sair | 'h'=ajuda║
╚══════════════════════════════════════════════════════════════════════╝
```

### Areas da Tela:

1. **MODULOS PENDENTES** - Modulos aguardando serem desarmados
   - `[1]` = ID do modulo
   - `f:` = Tipo (f=fios, b=botao, s=sequencia, i=simon)
   - `rgb` = Instrucao que voce deve usar

2. **BANCADAS** - Onde os Tedax trabalham
   - [LIVRE] = Disponivel para uso
   - [OCUPADA] = Tedax trabalhando

3. **TEDAX** - Seus tecnicos de desativacao
   - [DISPONIVEL] = Pronto para receber tarefa
   - [TRABALHANDO] = Desarmando um modulo
   - [AGUARDANDO] = Esperando bancada livre

4. **STATUS** - Tempo e estatisticas

5. **COMANDO** - Onde voce digita os comandos

---

## 4. Como Dar Comandos

### Formato do Comando

```
[TEDAX][TIPO][BANCADA][INSTRUCAO]
```

### Exemplo Pratico

Se voce ve na tela:
```
[1] f: rgb
```

Isso significa:
- Modulo ID: 1
- Tipo: `f` (fios)
- Instrucao correta: `rgb`

Para desarmar, digite:
```
1f1rgb
```

Onde:
- `1` = Usar Tedax 1
- `f` = Tipo do modulo (fios)
- `1` = Usar Bancada 1
- `rgb` = A instrucao (copie da tela!)

Pressione `ENTER` para enviar o comando.

---

## 5. Tipos de Modulos e Instrucoes

### Fios (f)
- **Descricao:** Cortar fios coloridos na ordem certa
- **Instrucao:** Sequencia de cores
  - `r` = vermelho (red)
  - `g` = verde (green)
  - `b` = azul (blue)
  - `y` = amarelo (yellow)
- **Exemplo:** Se mostrar `f: ygb`, digite `ygb` como instrucao

### Botao (b)
- **Descricao:** Pressionar botao varias vezes
- **Instrucao:** Repetir a letra `p`
- **Exemplo:** Se mostrar `b: pppp`, digite `pppp` (4 vezes)

### Sequencia (s)
- **Descricao:** Digitar sequencia numerica
- **Instrucao:** Numeros de 1 a 4
- **Exemplo:** Se mostrar `s: 1324`, digite `1324`

### Simon (i)
- **Descricao:** Repetir sequencia de direcoes
- **Instrucao:** Direcoes
  - `u` = cima (up)
  - `d` = baixo (down)
  - `l` = esquerda (left)
  - `r` = direita (right)
- **Exemplo:** Se mostrar `i: udlr`, digite `udlr`

---

## 6. Teclas de Controle

| Tecla | Funcao |
|-------|--------|
| `ENTER` | Enviar comando |
| `BACKSPACE` | Apagar ultimo caractere |
| `ESC` | Limpar todo o comando |
| `p` | Pausar/Despausar o jogo |
| `h` | Mostrar tela de ajuda |
| `q` | Sair do jogo |

---

## 7. Estrategias de Jogo

### Dicas para Iniciantes

1. **Sempre copie a instrucao exatamente como aparece na tela**
   - Se o modulo mostra `f: rgb`, a instrucao e `rgb`
   - Errar a instrucao = modulo volta para fila!

2. **Use todos os Tedax disponiveis**
   - Nao deixe Tedax parados enquanto ha modulos pendentes

3. **Fique de olho na fila**
   - Se chegar a 10 modulos pendentes, voce perde!

4. **Use a pausa (p) quando precisar pensar**
   - O jogo para, mas voce pode ver os modulos

### Exemplo de Partida

```
Situacao na tela:
  MODULOS PENDENTES
  [1] f: rgb    [2] b: ppp    [3] s: 1234

  Tedax 1: [DISPONIVEL]    Tedax 2: [DISPONIVEL]
  Bancada 1: [LIVRE]       Bancada 2: [LIVRE]

Seus comandos:
  > 1f1rgb    [ENTER]    <- Tedax 1 pega modulo de fios na bancada 1
  > 2b2ppp    [ENTER]    <- Tedax 2 pega modulo de botao na bancada 2

Agora ambos estao trabalhando!
Espere eles terminarem e repita com novos modulos.
```

---

## 8. Condicoes de Vitoria e Derrota

### Voce VENCE quando:
- Desarmar a quantidade de modulos definida nas configuracoes
- (Padrao: 10 modulos)

### Voce PERDE quando:
- O tempo acabar
- A fila de modulos pendentes encher (10 modulos)

---

## 9. Fluxo Completo de Jogo

```
1. Execute: ./bomb_defuser
2. Selecione "1. Iniciar Jogo" (ou configure primeiro)
3. Observe os modulos pendentes que aparecem
4. Para cada modulo:
   a. Veja o ID, tipo e instrucao
   b. Escolha um Tedax disponivel (1, 2 ou 3)
   c. Escolha uma bancada livre (1, 2, etc)
   d. Digite: [tedax][tipo][bancada][instrucao]
   e. Pressione ENTER
5. Repita ate vencer ou perder
6. Pressione 'q' para sair
```

---

## 10. Resolucao de Problemas

### "Tedax X esta ocupado!"
- Espere ele terminar ou use outro Tedax

### "Tipo de modulo invalido!"
- Use apenas: f, b, s, i

### "Nenhum modulo do tipo X na fila!"
- Nao ha modulos desse tipo pendentes
- Olhe a tela e escolha um tipo que existe

### "Comando muito curto!"
- O comando precisa ter pelo menos 4 caracteres
- Formato: [tedax][tipo][bancada][instrucao]

---

## Resumo Rapido

```
FORMATO DO COMANDO: [TEDAX][TIPO][BANCADA][INSTRUCAO]

TIPOS:
  f = Fios      -> instrucao: cores (r/g/b/y)
  b = Botao     -> instrucao: pressionar (pppp)
  s = Sequencia -> instrucao: numeros (1234)
  i = Simon     -> instrucao: direcoes (udlr)

TECLAS:
  ENTER = enviar    ESC = limpar    p = pausar    q = sair

EXEMPLO:
  Modulo na tela: [1] f: rgb
  Seu comando:    1f1rgb + ENTER
```

Boa sorte desarmando as bombas!
