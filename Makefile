# 编译器设置
CXX = g++

# 编译选项
# -std=c++17: 启用C++17标准
# -Wall: 显示所有警告信息
# -g: 生成调试信息 (发布时可替换为 -O2)
CXXFLAGS = -std=c++17 -Wall -g -Iinclude

# 可执行文件的名称
TARGET = snakegame

# 源文件列表
# 如果未来添加了新的 .cpp 文件，请在此处添加
SOURCES = src/main.cpp src/game.cpp src/snake.cpp src/map.cpp src/ai.cpp

# 目标文件列表
# 根据源文件列表自动生成对应的 .o 文件名
OBJECTS = $(SOURCES:.cpp=.o)



# 默认链接选项和清理命令
LDFLAGS = -lncurses
RM = rm -f

# 检测操作系统并覆盖设置
# Windows NT (通常用于 MinGW/MSYS2 环境)
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe  # Windows下可执行文件通常有.exe后缀
    LDFLAGS = -lpdcurses     # Windows下使用PDCurses
    RM = del /Q /F           # Windows下使用del命令
endif


# --- 规则定义 ---

# 默认目标：all
.PHONY: all
all: $(TARGET)

# 链接规则
# 从所有的目标文件(.o)生成最终的可执行文件
$(TARGET): $(OBJECTS)
	@echo "==> Linking all object files..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "==> Build finished successfully!"
	@echo "==> Run the game with: ./$(TARGET)"

# 编译规则 (模式规则)
# 将每个 .cpp 文件编译成对应的 .o 文件
%.o: %.cpp
	@echo "==> Compiling $<..."
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# 清理规则
.PHONY: clean
clean:
	@echo "==> Cleaning up build files and saved data..."
	-$(RM) $(TARGET) $(OBJECTS) leaderboard.bin level_progress.bin
	@echo "==> Cleanup complete."