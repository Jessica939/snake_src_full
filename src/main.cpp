#include "game.h"
#include <ncurses.h>

int main(int argc, char** argv)
{
    // 初始化ncurses环境
    initscr();
    // 不显示用户输入字符
    noecho();
    // 启用功能键
    keypad(stdscr, TRUE);
    // 不等待输入
    nodelay(stdscr, TRUE);
    // 隐藏光标
    curs_set(0);
    
    // 新增：启用并初始化颜色
    if (has_colors()) {
        start_color();
        // 定义颜色对: 1=蛇1(青色), 2=蛇2(黄色), 3=食物(红色)
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
    }

    // 清除屏幕
    clear();
    // 刷新屏幕
    refresh();
    // 清空所有键盘输入缓冲
    flushinp();
    
    // 创建游戏实例
    Game game;
    
    // 启动游戏的主循环
    game.startGame();
    
    // 结束ncurses环境
    endwin();
    
    return 0;
}