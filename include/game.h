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
#include <chrono> 

#include "snake.h"
#include "map.h"

// 游戏模式枚举
enum class GameMode
{
    Classic = 0, // 经典模式
    Level = 1,   // 关卡模式
    Timed = 2    // 限时模式
};

// 关卡类型枚举
enum class LevelType
{
    Normal, // 普通关卡
    Speed,  // 速度挑战
    Maze,   // 迷宫闯关
    Custom1, // 自定义关卡1 - 单键转向
    Custom2  // 自定义关卡2 - Boss战
};

// 关卡状态枚举
enum class LevelStatus
{
    Locked,    // 未解锁
    Unlocked,  // 已解锁
    Completed  // 已完成
};

// Boss状态枚举
enum class BossState
{
    Red,    // 红色状态 - 两道激光
    Green,  // 绿色状态 - 三道激光，可被攻击
    Yellow  // 黄色状态 - 四道激光
};

class Game
{
public:
    Game();
    ~Game();
    void startGame();
    
    // 把selectLevel()方法改为公有
    bool selectLevel();
    
    // 添加公有的shouldReturnToModeSelect()方法
    bool shouldReturnToModeSelect() const { return mReturnToModeSelect; }
    
private:
    // 界面相关
    std::vector<WINDOW *> mWindows;
    int mScreenHeight, mScreenWidth;
    int mInformationHeight = 5;
    int mInstructionWidth = 20;
    int mGameBoardHeight;
    int mGameBoardWidth;
    void createInformationBoard();
    void renderInformationBoard() const;
    void createGameBoard();
    void renderGameBoard() const;
    void createInstructionBoard();
    void renderInstructionBoard() const;
    
    // 蛇和地图
    std::unique_ptr<Snake> mPtrSnake;
    std::unique_ptr<Map> mPtrMap;
    SnakeBody mFood;
    const char mSnakeSymbol = '*';
    const char mFoodSymbol = '@';
    const char mWallSymbol = '#';
    const char mEndpointSymbol = 'X'; // 终点标记符号
    const char mSingleKeyTurnSymbol = 'T'; // 单键转向按键
    
    // 默认关卡地图文件
    const std::vector<std::string> mLevelMapFiles = {
        "maps/level1.txt",
        "maps/level2.txt",
        "maps/level3.txt",
        "maps/level4.txt", 
        "maps/level5.txt"
    };
    
    // 自定义地图文件
    const std::string mDefaultMapName = "[Default Map]";
    const std::vector<std::string> mMapFiles = {
        "maps/map1.txt",
        "maps/map2.txt",
        "maps/map3.txt"
    };
    
    // 游戏设置
    int mInitialSnakeLength = 3;
    
    // 游戏状态
    int mPoints = 0;
    int mDifficulty = 0;
    int mDelay = mBaseDelay;
    const int mBaseDelay = 100;
    
    // 排行榜系统
    int mNumLeaders = 5;
    std::vector<int> mLeaderBoard;
    const char* mRecordBoardFilePath = "record.dat";
    bool readLeaderBoard();
    bool updateLeaderBoard();
    bool writeLeaderBoard();
    void renderLeaderBoard() const;
    
    // 游戏相关函数
    void renderBoards() const;
    void initializeGame();
    void createRamdonFood();
    void renderFood() const;
    void renderSnake() const;
    void renderMap() const;
    void controlSnake() const;
    void adjustDelay();
    void runGame();
    bool renderRestartMenu() const;
    bool selectMap();
    void renderPoints() const;
    void renderDifficulty() const;
    
    // 关卡系统
    int mCurrentLevel = 1;
    int mMaxLevel = 5;
    GameMode mCurrentMode = GameMode::Classic;
    LevelType mCurrentLevelType = LevelType::Normal;
    std::vector<LevelStatus> mLevelStatus;
    int mLevelTargetPoints = 0;
    const char* mLevelProgressFilePath = "level_progress.dat";
    bool mReturnToModeSelect = false;
    
    // 创建默认的关卡地图
    void createDefaultLevelMaps();
    
    // 关卡系统函数
    void initializeLevel(int level);
    void loadNextLevel();
    bool isLevelCompleted();
    void runLevel();
    bool selectLevelInLevelMode();
    void unlockLevel(int level);
    bool saveLevelProgress();
    bool loadLevelProgress();
    void renderLevel() const;
    
    // 第四关特殊设置
    void initializeLevel4();
    void runLevel4();
    bool mHasEndpoint = false;
    SnakeBody mEndpoint;
    void renderEndpoint() const;
    void controlSnakeLevel4() const;
    
    // 第五关Boss战设置
    void initializeLevel5();
    void runLevel5();
    int mBossHP = 5;
    int mBossSize = 5;
    std::pair<int, int> mBossPosition; // Boss左上角的位置
    BossState mBossState = BossState::Red;
    int mBossStateTimer = 0;
    int mBossStateChangeDuration = 50;
    double mLaserAngle = 0.0;
    double mLaserRotationSpeed = 2.0;
    int mLaserLength = 0;
    
    // Boss战相关函数
    void updateBossState();
    void renderBoss();
    void updateAndRenderLasers();
    void renderLaser(int x1, int y1, int x2, int y2, char symbol);
    bool checkLaserCollision();
    bool checkSnakeLaserCollision(const std::vector<SnakeBody>& snake, int x1, int y1, int x2, int y2);
    double pointToLineDistance(int x0, int y0, int x1, int y1, int x2, int y2);
    bool checkBossAttack();
    
    // 限时模式
    void initializeTimeAttack();
    void runTimeAttack();
    void renderTimer() const;
    int mTimeAttackDurationSeconds = 120;
    int mTimeRemaining = 0;
    std::chrono::time_point<std::chrono::steady_clock> mTimeAttackStartTime;
};

#endif // GAME_H
