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
    // 清除屏幕
    clear();
    // 刷新屏幕
    refresh();
    // 清空所有键盘输入缓冲
    flushinp();
    
    // 创建游戏实例
    Game game;
    
    // 进入游戏循环，使用户能够从classic模式退回到模式选择
    bool exitGame = false;
    while (!exitGame) {
        // 选择游戏模式
        bool continueGame = game.selectLevel();
        
        // 如果用户选择退出，则结束游戏
        if (!continueGame) {
            exitGame = true;
            continue;
        }
        
        // 启动游戏
        game.startGame();
        
        // 如果用户没有选择返回模式选择，则退出游戏
        if (!game.shouldReturnToModeSelect()) {
            exitGame = true;
        }
        
        // 刷新屏幕并清除所有输入缓冲
        clear();
        refresh();
        flushinp();
    }
    
    // 结束ncurses环境
    endwin();
    
    return 0;
}
