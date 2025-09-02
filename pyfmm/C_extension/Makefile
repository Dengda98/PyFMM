# ifeq ($(OS),Windows_NT)
# define RMDIR_FUNC
#     if exist $(1) rmdir /S /Q $(1)
# endef
# else
# define RMDIR_FUNC
#     rm -rf $(1)
# endef
# endif

# ifeq ($(OS),Windows_NT)
# define MKDIR_FUNC
#     if not exist $(1) mkdir $(1)
# endef
# else
# define MKDIR_FUNC
#     mkdir -p $(1)
# endef
# endif

# ifeq ($(OS),Windows_NT)
# 	FILESEP = \\

# else 
# 	FILESEP = /
# endif

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BUILD_DIR_FLOAT = $(BUILD_DIR)/float
BUILD_DIR_DOUBLE = $(BUILD_DIR)/double
LIB_DIR = lib
LIB_NAME = $(LIB_DIR)/libfmm
LIB_EXT = .so

CC := gcc
FOMPFLAGS := -fopenmp 

# link static library on Windows
LINK_STATIC := 
LDFLAGS := $(FOMPFLAGS)

# expand stack memory for Windows
STACK_MEM := 

ifeq ($(OS),Windows_NT)  # link static oenpmp on windows
	STACK_MEM := -Wl,-stack,0x1000000
    LINK_STATIC := -static
	LDFLAGS := $(LINK_STATIC) $(FOMPFLAGS)
endif

# change architecture for macOS, from make command
ARCH = 

#  -ffast-math -march=native -mtune=native # 如果加上这些选项，数学库-lm需要在编译动态库时再次显式指定。总是gcc的编译参数的前后顺序很讲究
CFLAGS = $(LINK_STATIC) -lm -std=gnu99 -O3 -fPIC \
         -Wall $(STACK_MEM) -I$(shell realpath $(INC_DIR)) $(ARCH) $(FOMPFLAGS)   # -fsanitize=address -lasan

SRCS = $(wildcard $(SRC_DIR)/*.c)
INCS = $(wildcard $(INC_DIR)/*.h)

# 不同版本的目标文件目录
OBJS_FLOAT = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR_FLOAT)/%.o, $(SRCS))
OBJS_DOUBLE = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR_DOUBLE)/%.o, $(SRCS))
DEPS_FLOAT := $(OBJS_FLOAT:.o=.d)
DEPS_DOUBLE := $(OBJS_DOUBLE:.o=.d)

# 生成的库名称
TARGET_FLOAT = $(LIB_NAME)_float$(LIB_EXT)
TARGET_DOUBLE = $(LIB_NAME)_double$(LIB_EXT)
STATIC_TARGET_FLOAT = $(LIB_NAME)_float.a
STATIC_TARGET_DOUBLE = $(LIB_NAME)_double.a

.PHONY: all clean cleanbuild

all: $(BUILD_DIR) $(LIB_DIR) $(TARGET_FLOAT) $(TARGET_DOUBLE) $(STATIC_TARGET_FLOAT) $(STATIC_TARGET_DOUBLE)

$(BUILD_DIR):
	@mkdir -p $@

$(LIB_DIR):
	@mkdir -p $@

# ----------------------- Dependency generation -----------------------
-include $(DEPS_FLOAT)
-include $(DEPS_DOUBLE)

$(BUILD_DIR_FLOAT)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR_FLOAT)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(BUILD_DIR_DOUBLE)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR_DOUBLE)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# 编译 float 版本的目标文件
$(BUILD_DIR_FLOAT)/%.o: $(SRC_DIR)/%.c 
	$(CC) -o $@ -c $< $(CFLAGS) -DUSE_FLOAT

# 编译 double 版本的目标文件
$(BUILD_DIR_DOUBLE)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

# 链接动态库，生成 float 版本
$(TARGET_FLOAT): $(OBJS_FLOAT)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

# 链接动态库，生成 double 版本
$(TARGET_DOUBLE): $(OBJS_DOUBLE)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(STATIC_TARGET_FLOAT): $(OBJS_FLOAT)
	ar rcs $@ $^

$(STATIC_TARGET_DOUBLE): $(OBJS_DOUBLE)
	ar rcs $@ $^

cleanbuild:
	rm -rf $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
