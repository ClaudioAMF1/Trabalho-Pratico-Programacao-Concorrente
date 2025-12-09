# =============================================================================
# Makefile - Keep Solving and Nobody Explodes (Versao de Treino)
# Trabalho Pratico - Programacao Concorrente
# IDP 2025/2
# =============================================================================

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
LDFLAGS = -lncurses -lpthread

# Diretorios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = .

# Nome do executavel
TARGET = $(BIN_DIR)/bomb_defuser

# Arquivos fonte
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/jogo.c \
          $(SRC_DIR)/modulos.c \
          $(SRC_DIR)/tedax.c \
          $(SRC_DIR)/bancada.c \
          $(SRC_DIR)/display.c

# Arquivos objeto
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Headers
HEADERS = $(INC_DIR)/tipos.h \
          $(INC_DIR)/jogo.h \
          $(INC_DIR)/modulos.h \
          $(INC_DIR)/tedax.h \
          $(INC_DIR)/bancada.h \
          $(INC_DIR)/display.h

# =============================================================================
# Regras principais
# =============================================================================

.PHONY: all clean run debug help

# Regra padrao: compila o projeto
all: $(OBJ_DIR) $(TARGET)
	@echo ""
	@echo "=============================================="
	@echo " Compilacao concluida com sucesso!"
	@echo " Execute: ./bomb_defuser"
	@echo "=============================================="
	@echo ""

# Cria diretorio de objetos
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Linka o executavel
$(TARGET): $(OBJECTS)
	@echo "[LINK] Gerando executavel..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compila arquivos fonte
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@echo "[CC] Compilando $<..."
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# =============================================================================
# Regras auxiliares
# =============================================================================

# Limpa arquivos compilados
clean:
	@echo "[CLEAN] Removendo arquivos compilados..."
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)
	@echo "[CLEAN] Concluido!"

# Executa o jogo
run: all
	@echo ""
	@echo "Iniciando o jogo..."
	@echo ""
	./$(TARGET)

# Compila com simbolos de debug
debug: CFLAGS += -DDEBUG -O0
debug: clean all
	@echo "[DEBUG] Compilado com simbolos de debug"

# Compila com otimizacoes
release: CFLAGS += -O2 -DNDEBUG
release: clean all
	@echo "[RELEASE] Compilado com otimizacoes"

# Verifica dependencias
check-deps:
	@echo "Verificando dependencias..."
	@which gcc > /dev/null || (echo "ERRO: gcc nao encontrado!" && exit 1)
	@echo "  [OK] gcc"
	@ldconfig -p | grep ncurses > /dev/null || (echo "ERRO: libncurses nao encontrada!" && exit 1)
	@echo "  [OK] libncurses"
	@echo "Todas as dependencias estao instaladas!"

# Instala dependencias (Ubuntu/Debian)
install-deps:
	@echo "Instalando dependencias..."
	sudo apt-get update
	sudo apt-get install -y build-essential libncurses5-dev libncursesw5-dev
	@echo "Dependencias instaladas!"

# Ajuda
help:
	@echo ""
	@echo "=== Keep Solving and Nobody Explodes - Makefile ==="
	@echo ""
	@echo "Comandos disponiveis:"
	@echo "  make          - Compila o projeto"
	@echo "  make all      - Compila o projeto"
	@echo "  make clean    - Remove arquivos compilados"
	@echo "  make run      - Compila e executa o jogo"
	@echo "  make debug    - Compila com simbolos de debug"
	@echo "  make release  - Compila com otimizacoes"
	@echo "  make check-deps   - Verifica dependencias"
	@echo "  make install-deps - Instala dependencias (apt)"
	@echo "  make help     - Exibe esta ajuda"
	@echo ""
	@echo "Requisitos:"
	@echo "  - GCC (compilador C)"
	@echo "  - libncurses (biblioteca de interface)"
	@echo "  - pthread (threads POSIX)"
	@echo ""

# =============================================================================
# Informacoes de debug
# =============================================================================

info:
	@echo "=== Informacoes do projeto ==="
	@echo "Fontes: $(SOURCES)"
	@echo "Objetos: $(OBJECTS)"
	@echo "Headers: $(HEADERS)"
	@echo "Executavel: $(TARGET)"
	@echo "Flags: $(CFLAGS)"
	@echo "Libs: $(LDFLAGS)"
