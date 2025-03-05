CC = gcc
CFLAGS = -g -Iinclude
LDFLAGS = -lpthread -lm
SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TARGET = evc

all: $(TARGET)

$(TARGET): $(OBJS) $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)