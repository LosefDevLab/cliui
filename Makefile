# 编译器和选项
CC = clang++
CFLAGS = -Wall -Wextra -I./src/ui -I./src/api
LDFLAGS = 
TARGET = cliui

# 源文件目录和构建目录配置
SRC_DIRS = src/ui src/api
BUILD_DIR = build

# 自动收集源文件并生成目标文件路径
SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $@ with Clang++..."
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp
	@echo "🔨 Compiling $<..."
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ -std=c++20 -O3

clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)

.PHONY: all clean