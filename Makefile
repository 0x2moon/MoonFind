CC = gcc
TARGET_NAME = endless_jumping

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
LIB_DIR = lib

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))
TARGET = $(BIN_DIR)/$(TARGET_NAME)

RAYLIB_PATH = $(LIB_DIR)/libraylib.a

CFLAGS = -O2 -Wall -Wextra -std=c99 -I.

LDFLAGS = -L$(LIB_DIR) -l:libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11 \
          -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run:
	./$(TARGET)

clean:
	@echo "Limpando arquivos de build..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)


# sudo apt update
# sudo apt install build-essential libgl1-mesa-dev libx11-dev libxrandr-dev \
# libxinerama-dev libxcursor-dev libxi-dev libxext-dev libxfixes-dev \
# libxmu-dev libxxf86vm-dev libpthread-stubs0-dev libdl-dev librt-dev
