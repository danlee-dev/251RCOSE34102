CC = gcc
CFLAGS = -Wall -I./include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 소스 파일 목록
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = scheduler

# 디렉토리 생성
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# 빌드 타겟
all: $(BIN_DIR)/$(TARGET)

# 실행 파일 생성
$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 오브젝트 파일 생성
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 정리
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/$(TARGET)

.PHONY: all clean
