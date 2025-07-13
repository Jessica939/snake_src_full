# 编译器设置
CXX = g++
CXXFLAGS = -O2 -Wall -Iinclude -std=c++17 -fPIC

# Qt设置
QT_INCLUDES = $(shell pkg-config --cflags Qt5Core Qt5Widgets Qt5Gui Qt5Multimedia)
QT_LIBS = $(shell pkg-config --libs Qt5Core Qt5Widgets Qt5Gui Qt5Multimedia)
MOC = moc

# 源文件和目标文件
SRC_DIR = src
GUI_DIR = gui
INCLUDE_DIR = include
OBJ_FILES = main.o game.o snake.o map.o mode_select_window.o story_level_window.o gui_manager.o gui_manager_moc.o mode_select_window_moc.o story_level_window_moc.o

# 可执行文件的名称
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
	$(CXX) -o $@ $^ -lcurses $(QT_LIBS)

# 编译源文件为目标文件的规则
main.o: $(SRC_DIR)/main.cpp $(INCLUDE_DIR)/game.h $(INCLUDE_DIR)/gui/gui_manager.h
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $<

game.o: $(SRC_DIR)/game.cpp $(INCLUDE_DIR)/game.h $(INCLUDE_DIR)/snake.h $(INCLUDE_DIR)/map.h
	$(CXX) $(CXXFLAGS) -c $<

snake.o: $(SRC_DIR)/snake.cpp $(INCLUDE_DIR)/snake.h $(INCLUDE_DIR)/map.h
	$(CXX) $(CXXFLAGS) -c $<

map.o: $(SRC_DIR)/map.cpp $(INCLUDE_DIR)/map.h
	$(CXX) $(CXXFLAGS) -c $<

# GUI相关编译规则
mode_select_window.o: $(GUI_DIR)/mode_select_window.cpp $(INCLUDE_DIR)/gui/mode_select_window.h
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $<

story_level_window.o: $(GUI_DIR)/story_level_window.cpp $(INCLUDE_DIR)/gui/story_level_window.h
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $<

gui_manager.o: $(GUI_DIR)/gui_manager.cpp $(INCLUDE_DIR)/gui/gui_manager.h
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $<

# MOC文件生成和编译规则
gui_manager_moc.cpp: $(INCLUDE_DIR)/gui/gui_manager.h
	$(MOC) $(INCLUDE_DIR)/gui/gui_manager.h -o gui_manager_moc.cpp

mode_select_window_moc.cpp: $(INCLUDE_DIR)/gui/mode_select_window.h
	$(MOC) $(INCLUDE_DIR)/gui/mode_select_window.h -o mode_select_window_moc.cpp

story_level_window_moc.cpp: $(INCLUDE_DIR)/gui/story_level_window.h
	$(MOC) $(INCLUDE_DIR)/gui/story_level_window.h -o story_level_window_moc.cpp

gui_manager_moc.o: gui_manager_moc.cpp
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $<

mode_select_window_moc.o: mode_select_window_moc.cpp
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $<

story_level_window_moc.o: story_level_window_moc.cpp
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $<

# 清理编译产物
clean:
	rm -f *.o 
	rm -f $(TARGET)
	rm -f record.dat
	rm -f *_moc.cpp

# 增量编译（不重新生成已经最新的文件）
.PHONY: all clean

# 避免删除中间文件
.PRECIOUS: $(OBJ_FILES)
