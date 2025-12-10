# Keep Solving and Nobody Explodes

Simulador de treinamento para desativacao de bombas com programacao concorrente.

**Aluno:** Claudio da Aparecida Meireles Filho
**RA:** 2321070
**Disciplina:** Programacao Concorrente - IDP 2025/2

---

## Sobre o Projeto

Jogo onde voce coordena tecnicos (Tedax) para desarmar modulos de uma bomba. Cada Tedax e uma thread independente que compete por bancadas de trabalho.

---

## Compilacao

```bash
sudo apt install build-essential libncurses5-dev   # dependencias
make                                                # compilar
make run                                            # executar
```

---

## Como Jogar

### Entendendo a Tela

```
MODULOS PENDENTES
[1] f: rgb    [2] b: pppp    [3] s: 1234
     ^  ^^^
     |  instrucao (copie isso!)
     tipo do modulo
```

### Montando o Comando

Formato: `[Tedax][Tipo][Bancada][Instrucao]`

**Exemplo:** Modulo `[1] f: rgb` apareceu na tela
- Tedax 1 disponivel
- Bancada 1 livre
- Comando: `1f1rgb` + ENTER

### Tipos de Modulos

| Tipo | Letra | Instrucao |
|------|-------|-----------|
| Fios | f | cores: r, g, b, y |
| Botao | b | repetir p |
| Sequencia | s | numeros 1-4 |
| Simon | i | direcoes u, d, l, r |

### Teclas

- `ENTER` - enviar comando
- `BACKSPACE` - apagar
- `p` - pausar
- `h` - ajuda
- `q` - sair

---

## Estrutura

```
src/           codigo fonte
include/       headers
Makefile       compilacao
ARTIGO_SBC.md  documentacao tecnica
```

---

## Conceitos Aplicados

- Threads POSIX (pthreads)
- Mutex para protecao de recursos
- Variaveis de condicao para sincronizacao
- Recursos compartilhados (bancadas, fila de modulos)
