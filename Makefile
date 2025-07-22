# # Makefile para Endless Jumping Game (Linux)

# # Compilador
# CC = gcc

# # Flags
# CFLAGS = -O2 -Wall -Wextra -std=c99 -I. -Iinclude
# LDFLAGS = -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt

# # Diretórios
# SRC_DIR = src
# OBJ_DIR = obj
# BIN_DIR = bin

# # Arquivos fonte
# SRC = $(wildcard $(SRC_DIR)/*.c)
# OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))
# TARGET = $(BIN_DIR)/endless_jumping

# # Alvo principal
# all: create_dirs $(TARGET)

# create_dirs:
# 	@mkdir -p $(OBJ_DIR)
# 	@mkdir -p $(BIN_DIR)

# $(TARGET): $(OBJ)
# 	$(CC) $^ -o $@ $(LDFLAGS)

# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -rf $(OBJ_DIR) $(BIN_DIR)

# .PHONY: all clean create_dirs


# Makefile para Endless Jumping Game (Linux) - Compilação Estática

# Compilador

# Makefile para Endless Jumping Game (Linux) - Compilação Estática

# Compilador

# Makefile para Endless Jumping Game (Linux)

# Compilador
# Makefile para Endless Jumping Game (Linux)


# Makefile para Endless Jumping Game (Linux)

# Compilador
# sudo apt-get install libgl1-mesa-dev
CC = gcc

# Nome do executável
TARGET_NAME = endless_jumping

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Arquivos
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))
TARGET = $(BIN_DIR)/$(TARGET_NAME)

# Flags do compilador
CFLAGS = -O2 -Wall -Wextra -std=c99 -I./src

# Flags do linker (LINKAGEM MISTA)
# Sem a flag "-static", o linker usará as bibliotecas de sistema (.so)
# mas encontrará a sua "libraylib.a" estaticamente.
LDFLAGS = -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Alvo principal
all: $(TARGET)

# Regra de linkagem
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# Regra de compilação
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpa os arquivos gerados
clean:
	@echo "Limpando arquivos de build..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Alvos que não representam arquivos
.PHONY: all clean