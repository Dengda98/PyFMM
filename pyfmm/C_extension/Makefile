
SRC_DIR = ./src
INC_DIR = ./include
BUILD_DIR = ./build
LIB_DIR = ./lib
LIB_NAME = $(LIB_DIR)/libfmm
LIB_EXT = .so

# 
CC = gcc
CFLAGS =  -O3 -g -ffast-math -march=native -mtune=native -fPIC -Wimplicit-function-declaration -I$(INC_DIR)  -lm  # -fsanitize=address -lasan
LDFLAGS = -shared -lm # -lasan
SRCS = $(wildcard $(SRC_DIR)/*.c)
INCS = $(wildcard $(INC_DIR)/*.h)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all clean

TARGET = $(LIB_NAME)$(LIB_EXT)

all: $(TARGET)

# 链接动态库
# 编译语句中顺序很关键，否则链接不上库
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	

# 编译目标文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCS)
	@mkdir -p $(BUILD_DIR)
	$(CC)  -o $@ -c $< $(CFLAGS)

# 清理
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)