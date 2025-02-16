# 编译器和编译标志
CXX := g++
# 编译标志：启用所有警告，加入调试信息
CXXFLAGS := -Wall -Wextra -Wpedantic -g
# 链接器标志：可在编译时选择性添加，例如 -pthread 用于多线程支持
LDFLAGS := -lmysqlclient -lsqlite3 -ljson-c

# 目录
# 存放目标文件的目录
BUILD_DIR := .deps

# 文件
# 获取当前目录下的所有 .cpp 源文件，但排除 test.cpp
SRC := $(filter-out ./test.cpp, $(wildcard ./*.cpp))
# 将源文件路径转换为目标文件路径，存放在 BUILD_DIR 目录中
OBJ := $(patsubst ./%.cpp, $(BUILD_DIR)/%.o, $(SRC))
# 最终生成的可执行文件名称
TARGET := server

# 目标
.PHONY : all clean show run

# 默认目标：生成可执行文件
all: $(TARGET)

# 生成可执行文件的规则，依赖于所有的目标文件
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# 生成目标文件的规则，依赖于对应的源文件
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# 创建目标文件存放目录
$(BUILD_DIR):
	mkdir -p $@

# 清理构建文件
clean:
	$(RM) -r $(BUILD_DIR) $(TARGET)

# 显示源文件和目标文件信息，用于调试
show:
	@echo "Source files: $(SRC)"
	@echo "Object files: $(OBJ)"

# 运行程序并设置 core 文件
run: $(TARGET)
	@echo "设置 core 文件路径..."
	@echo "core.%e.%p" | sudo tee /proc/sys/kernel/core_pattern
	@echo "设置 core 文件大小为无限制..."
	ulimit -c unlimited
	@echo "运行程序..."
	./$(TARGET)
