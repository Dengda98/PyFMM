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

OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

# 生成的库名称
TARGET = $(LIB_NAME)$(LIB_EXT)
STATIC_TARGET = $(LIB_NAME).a

.PHONY: all clean cleanbuild

all: $(TARGET) $(STATIC_TARGET)

$(BUILD_DIR):
	@mkdir -p $@

$(LIB_DIR):
	@mkdir -p $@

# ----------------------- Dependency generation -----------------------
-include $(DEPS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -MMD -MP -o $@ -c $< $(CFLAGS)

$(TARGET): $(OBJS) | $(LIB_DIR)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(STATIC_TARGET): $(OBJS) | $(LIB_DIR)
	ar rcs $@ $^

cleanbuild:
	rm -rf $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
