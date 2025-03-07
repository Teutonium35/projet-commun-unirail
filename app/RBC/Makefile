CC = gcc
CFLAGS = -g -Iinclude -I../utility/include
LDFLAGS = -lpthread -lm
SRC_DIR = src
UTILITY_SRC_DIR = ../utility/src
BIN_DIR = bin
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(UTILITY_SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c)) \
       $(patsubst $(UTILITY_SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(UTILITY_SRC_DIR)/*.c))

TARGET = rbc

all: $(TARGET)

$(TARGET): $(OBJS) $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(UTILITY_SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

bin/test_aig: test/test_aig.c
	$(CC) $(CFLAGS) $^ -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)