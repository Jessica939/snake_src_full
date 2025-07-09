# 编译器设置
CXX = g++
CXXFLAGS = -O2 -Wall -Iinclude

# 源文件和目标文件
SRC_DIR = src
INCLUDE_DIR = include
OBJ_FILES = main.o game.o snake.o map.o

# 最终目标
TARGET = snakegame

# 使用一个简单的判断来检测操作系统
ifeq ($(OS),Windows_NT)
  # Windows系统
  JOBS = 4
else
  # Linux/Unix系统
  JOBS = $(shell nproc 2>/dev/null || echo 2)
endif

# MAKEFLAGS设置，为默认make命令启用并行编译
MAKEFLAGS += -j$(JOBS)

# 默认目标
all: $(TARGET)

# 链接最终可执行文件
$(TARGET): $(OBJ_FILES)
	$(CXX) -o $@ $^ -lcurses

# 编译源文件为目标文件的规则
main.o: $(SRC_DIR)/main.cpp $(INCLUDE_DIR)/game.h
	$(CXX) $(CXXFLAGS) -c $<

game.o: $(SRC_DIR)/game.cpp $(INCLUDE_DIR)/game.h $(INCLUDE_DIR)/snake.h $(INCLUDE_DIR)/map.h
	$(CXX) $(CXXFLAGS) -c $<

snake.o: $(SRC_DIR)/snake.cpp $(INCLUDE_DIR)/snake.h $(INCLUDE_DIR)/map.h
	$(CXX) $(CXXFLAGS) -c $<

map.o: $(SRC_DIR)/map.cpp $(INCLUDE_DIR)/map.h
	$(CXX) $(CXXFLAGS) -c $<

# 清理编译产物
clean:
	rm -f *.o 
	rm -f $(TARGET)
	rm -f record.dat

# 增量编译（不重新生成已经最新的文件）
.PHONY: all clean

# 避免删除中间文件
.PRECIOUS: $(OBJ_FILES)
