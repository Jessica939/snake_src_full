#include "game.h"
#include "gui/gui_manager.h"
#include <ncurses.h>
#include <QApplication>
#include <cstdlib>
#include <cstring>

// 检查是否支持GUI环境
bool isGUIAvailable()
{
    // 检查DISPLAY环境变量
    const char* display = std::getenv("DISPLAY");
    if (!display || strlen(display) == 0) {
        return false;
    }
    return true;
}

// Qt GUI模式启动函数
int startQtGUI(int argc, char** argv)
{
    // 首先检查是否有图形环境
    if (!isGUIAvailable()) {
        return -1; // 没有图形环境，直接回退
    }
    
    try {
        QApplication app(argc, argv);
        
        GUIManager guiManager;
        
        // 同步关卡进度（确保显示最新的解锁状态）
        guiManager.syncLevelProgress();
        
        guiManager.start();
        
        // 运行Qt事件循环，等待用户选择
        app.exec();
        
        // 检查用户选择
        if (guiManager.isExitRequested()) {
            return 0; // 用户选择退出
        }
        
        if (guiManager.isClassicModeSelected()) {
            return 1; // 用户选择经典模式
        }
        
        // 用户选择了剧情模式的某个关卡
        return guiManager.getSelectedLevel() + 1; // 返回关卡号+1（2-6代表关卡1-5）
    }
    catch (...) {
        // GUI启动失败，返回-1表示需要回退到ncurses模式
        return -1;
    }
}

int main(int argc, char** argv)
{
    // 首先尝试启动Qt GUI进行模式选择
    int guiResult = startQtGUI(argc, argv);
    
    // 如果GUI启动失败，回退到传统ncurses模式
    if (guiResult == -1) {
        // 初始化ncurses环境
        initscr();
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        curs_set(0);
        clear();
        refresh();
        flushinp();
        
        // 创建游戏实例
        Game game;
        
        // 进入传统的游戏循环
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
    
    // 根据GUI选择结果决定后续行为
    if (guiResult == 0) {
        // 用户选择退出
        return 0;
    }
    
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
    
    bool exitGame = false;
    
    if (guiResult == 1) {
        // 经典模式：进入游戏循环，允许用户选择模式并退回模式选择
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
    } else {
        // 剧情模式：直接启动指定关卡
        int selectedLevel = guiResult - 1; // 转换回关卡号（1-5）
        
        // 进入关卡模式循环，允许用户在通关后返回
        while (!exitGame) {
            // 直接启动指定关卡，跳过关卡选择界面
            game.startLevelDirectly(selectedLevel);
            
            // 检查关卡是否胜利，如果是level1胜利则显示漫画
            if (game.isLevelCompleted() && selectedLevel == 0) { // selectedLevel 0 = level1
                // 结束ncurses环境，切换到Qt环境显示胜利漫画
                endwin();
                
                // 创建Qt应用显示胜利漫画
                QApplication app(argc, argv);
                GUIManager guiManager;
                guiManager.showCartoonAfterLevelVictory(1); // level1胜利漫画
                app.exec();
                
                // 漫画播放完成后，结束ncurses并重新启动GUI进行关卡选择
                // 重新启动GUI进行关卡选择
                int newGuiResult = startQtGUI(argc, argv);
                
                if (newGuiResult == 0) {
                    // 用户选择退出
                    exitGame = true;
                    continue;
                } else if (newGuiResult == 1) {
                    // 用户选择经典模式，重新初始化ncurses
                    initscr();
                    noecho();
                    keypad(stdscr, TRUE);
                    nodelay(stdscr, TRUE);
                    curs_set(0);
                    
                    if (has_colors()) {
                        start_color();
                    }
                    
                    // 经典模式游戏循环
                    while (!exitGame) {
                        bool continueGame = game.selectLevel();
                        if (!continueGame) {
                            exitGame = true;
                            break;
                        }
                        game.startGame();
                        if (!game.shouldReturnToModeSelect()) {
                            exitGame = true;
                        }
                        clear();
                        refresh();
                        flushinp();
                    }
                    continue;
                } else {
                    // 用户选择了新的关卡，更新selectedLevel并重新初始化ncurses
                    selectedLevel = newGuiResult - 1;
                    initscr();
                    noecho();
                    keypad(stdscr, TRUE);
                    nodelay(stdscr, TRUE);
                    curs_set(0);
                    continue; // 继续关卡循环
                }
            }
            
            // 如果用户选择返回模式选择，重新显示GUI
            if (game.shouldReturnToModeSelect()) {
                // 结束ncurses环境
                endwin();
                
                // 重新启动GUI进行关卡选择
                int newGuiResult = startQtGUI(argc, argv);
                
                if (newGuiResult == 0) {
                    // 用户选择退出
                    exitGame = true;
                } else if (newGuiResult == 1) {
                    // 用户选择经典模式，重新初始化ncurses并进入经典模式循环
                    initscr();
                    noecho();
                    keypad(stdscr, TRUE);
                    nodelay(stdscr, TRUE);
                    curs_set(0);
                    
                    if (has_colors()) {
                        start_color();
                        init_pair(1, COLOR_CYAN, COLOR_BLACK);
                        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
                        init_pair(3, COLOR_RED, COLOR_BLACK);
                    }
                    
                    clear();
                    refresh();
                    flushinp();
                    
                    // 进入经典模式循环
                    while (!exitGame) {
                        bool continueGame = game.selectLevel();
                        if (!continueGame) {
                            exitGame = true;
                            continue;
                        }
                        game.startGame();
                        if (!game.shouldReturnToModeSelect()) {
                            exitGame = true;
                        }
                        clear();
                        refresh();
                        flushinp();
                    }
                } else {
                    // 用户选择了新的关卡
                    selectedLevel = newGuiResult - 1;
                    
                    // 重新初始化ncurses环境
                    initscr();
                    noecho();
                    keypad(stdscr, TRUE);
                    nodelay(stdscr, TRUE);
                    curs_set(0);
                    
                    if (has_colors()) {
                        start_color();
                        init_pair(1, COLOR_CYAN, COLOR_BLACK);
                        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
                        init_pair(3, COLOR_RED, COLOR_BLACK);
                    }
                    
                    clear();
                    refresh();
                    flushinp();
                    
                    // 继续新选择的关卡
                    continue;
                }
            } else {
                // 用户选择退出游戏
                exitGame = true;
            }
        }
    }
    
    // 结束ncurses环境
    endwin();
    
    return 0;
}