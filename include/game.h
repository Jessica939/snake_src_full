#ifndef GAME_H
#define GAME_H

#ifdef _WIN32
#include <curses.h>
#else
#include <ncurses.h>
#endif

#include <string>
#include <vector>
#include <memory>

#include "snake.h"
#include "map.h"

// 关卡类型枚举
enum class LevelType
{
    Normal = 0,     // 普通关卡
    Speed = 1,      // 速度挑战关卡
    Maze = 2,       // 迷宫关卡
    Custom1 = 3,    // 自定义关卡1（第四关）
    Custom2 = 4     // 自定义关卡2（第五关）
};

// 关卡状态枚举
enum class LevelStatus
{
    Locked = 0,     // 未解锁
    Unlocked = 1,   // 已解锁
    Completed = 2   // 已完成
};

class Game
{
public:
    Game();
    ~Game();
    
    void createInformationBoard();
    void renderInformationBoard() const;

    void createGameBoard();
    void renderGameBoard() const;
    
    void createInstructionBoard();
    void renderInstructionBoard() const;
    
    void loadLeadBoard();
    void updateLeadBoard();
    bool readLeaderBoard();
    bool updateLeaderBoard();
    bool writeLeaderBoard();
    void renderLeaderBoard() const;
    
    void renderBoards() const;
    
    void initializeGame();
    void runGame();
    void renderPoints() const;
    void renderDifficulty() const;
    void renderLevel() const; // 新增：显示当前关卡
    
    void createRamdonFood();
    void renderFood() const;
    void renderSnake() const;
    void renderMap() const;
    void controlSnake() const;
    
    void startGame();
    bool renderRestartMenu() const;
    void adjustDelay();
    bool selectMap();
    bool selectLevel(); // 返回值：true表示继续游戏，false表示退出
    bool shouldReturnToModeSelect() const { return mReturnToModeSelect; } // 获取是否应该返回模式选择
    bool selectLevelInLevelMode(); // 新增：在关卡模式中选择已解锁的关卡
    
    // 关卡相关方法
    void createDefaultLevelMaps(); // 创建默认关卡地图文件
    void initializeLevel(int level); // 初始化指定关卡
    void loadNextLevel(); // 加载下一关卡
    bool isLevelCompleted(); // 检查当前关卡是否完成
    void runLevel(); // 运行当前关卡
    void unlockLevel(int level); // 解锁指定关卡
    void updateLevelStatus(); // 更新关卡状态
    bool saveLevelProgress(); // 保存关卡进度
    bool loadLevelProgress(); // 加载关卡进度
    

private:
    // We need to have two windows
    // One is for game introduction
    // One is for game mWindows
    int mScreenWidth;
    int mScreenHeight;
    int mGameBoardWidth;
    int mGameBoardHeight;
    const int mInformationHeight = 6;
    const int mInstructionWidth = 18;
    std::vector<WINDOW *> mWindows;
    // Snake information
    const int mInitialSnakeLength = 2;
    const char mSnakeSymbol = '@';
    std::unique_ptr<Snake> mPtrSnake;
    // Map information
    std::unique_ptr<Map> mPtrMap;
    const char mWallSymbol = '+';
    // Food information
    SnakeBody mFood;
    const char mFoodSymbol = '#';
    int mPoints = 0;
    int mDifficulty = 0;
    int mBaseDelay = 100;
    int mDelay;
    const std::string mRecordBoardFilePath = "record.dat";
    std::vector<int> mLeaderBoard;
    const int mNumLeaders = 3;
    // Map file paths
    const std::string mDefaultMapName = "default";
    std::vector<std::string> mMapFiles = {"maps/map1.txt", "maps/map2.txt", "maps/map3.txt"};
    
    // 关卡相关变量
    int mCurrentLevel = 1;  // 当前关卡
    const int mMaxLevel = 5; // 最大关卡数
    LevelType mCurrentLevelType = LevelType::Normal; // 当前关卡类型
    int mLevelTargetPoints = 5; // 当前关卡通过所需的点数
    bool mIsLevelMode = false; // 是否处于关卡模式
    bool mReturnToModeSelect = false; // 是否返回到模式选择界面
    std::vector<LevelStatus> mLevelStatus; // 关卡状态列表
    const std::string mLevelProgressFilePath = "level_progress.dat"; // 关卡进度文件路径
    std::vector<std::string> mLevelMapFiles = {"maps/level1.txt", "maps/level2.txt", "maps/level3.txt", "maps/level4.txt", "maps/level5.txt"}; // 关卡地图文件
};

#endif
