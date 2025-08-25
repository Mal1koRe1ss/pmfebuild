CC = gcc
CFLAGS = -Wall -Wextra -I./src
SRC = src/extract.c src/setup.c
BUILD_DIR = build
OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))
TARGET = $(BUILD_DIR)/program

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

