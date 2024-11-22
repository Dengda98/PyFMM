ifeq ($(OS),Windows_NT)
define RMDIR_FUNC
    if exist $(1) rmdir /S /Q $(1)
endef
else
define RMDIR_FUNC
    rm -rf $(1)
endef
endif

ifeq ($(OS),Windows_NT)
define MKDIR_FUNC
    if not exist $(1) mkdir $(1)
endef
else
define MKDIR_FUNC
    mkdir -p $(1)
endef
endif

ifeq ($(OS),Windows_NT)
	FILESEP = \\

else 
	FILESEP = /
endif

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BUILD_DIR_FLOAT = $(BUILD_DIR)$(FILESEP)float
BUILD_DIR_DOUBLE = $(BUILD_DIR)$(FILESEP)double
LIB_DIR = lib
LIB_NAME = $(LIB_DIR)$(FILESEP)libfmm
LIB_EXT = .so

CC = gcc
FOMPFLAGS = -fopenmp 
CFLAGS = $(FOMPFLAGS) -O3 -g -ffast-math -march=native -mtune=native -fPIC -Wimplicit-function-declaration -I$(INC_DIR) -lm   # -fsanitize=address -lasan
LDFLAGS = $(FOMPFLAGS) -shared -lm # -lasan

SRCS = $(wildcard $(SRC_DIR)/*.c)
INCS = $(wildcard $(INC_DIR)/*.h)

# 不同版本的目标文件目录
OBJS_FLOAT = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR_FLOAT)/%.o, $(SRCS))
OBJS_DOUBLE = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR_DOUBLE)/%.o, $(SRCS))

# 生成的库名称
TARGET_FLOAT = $(LIB_NAME)_float$(LIB_EXT)
TARGET_DOUBLE = $(LIB_NAME)_double$(LIB_EXT)

.PHONY: all clean

all: $(TARGET_FLOAT) $(TARGET_DOUBLE)

# 链接动态库，生成 float 版本
$(TARGET_FLOAT): $(OBJS_FLOAT)
	@$(call MKDIR_FUNC, $(LIB_DIR))
	$(CC) -o $@ $^ $(LDFLAGS)

# 链接动态库，生成 double 版本
$(TARGET_DOUBLE): $(OBJS_DOUBLE)
	@$(call MKDIR_FUNC, $(LIB_DIR))
	$(CC) -o $@ $^ $(LDFLAGS)

# 编译 float 版本的目标文件
$(BUILD_DIR_FLOAT)/%.o: $(SRC_DIR)/%.c $(INCS)
	@$(call MKDIR_FUNC, $(BUILD_DIR_FLOAT))
	$(CC) -o $@ -c $< $(CFLAGS) -DUSE_FLOAT

# 编译 double 版本的目标文件
$(BUILD_DIR_DOUBLE)/%.o: $(SRC_DIR)/%.c $(INCS)
	@$(call MKDIR_FUNC, $(BUILD_DIR_DOUBLE))
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	$(call RMDIR_FUNC, $(BUILD_DIR))
	$(call RMDIR_FUNC, $(LIB_DIR))
