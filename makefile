CC = mpicc
CFLAGS = -Wall -g

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/utils.c
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/utils.o
TARGET = $(BIN_DIR)/gameoflife

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/utils.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)
