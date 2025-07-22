# Makefile para Endless Jumping Game (Linux)

# Compilador
CC = gcc

# Flags
CFLAGS = -O2 -Wall -Wextra -std=c99 -I. -Iinclude
LDFLAGS = -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Arquivos fonte
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))
TARGET = $(BIN_DIR)/endless_jumping

# Alvo principal
all: create_dirs $(TARGET)

create_dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean create_dirs