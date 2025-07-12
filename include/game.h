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

// 自定义模块
#include "snake.h"
#include "map.h"
#include "ai.h"

// ========== 枚举定义 ==========
enum class GameMode { Classic, Level, Timed, Battle };
enum class LevelType { Normal, Speed, Maze, Custom1, Custom2 };
enum class LevelStatus { Locked, Unlocked, Completed };
enum class BossState { Red, Green };
enum class BattleType { PlayerVsPlayer, PlayerVsAI };

// ========== 游戏主类 ==========
class Game {
public:
    Game();
    ~Game();

    void startGame();

    // 公共函数
    bool selectLevel(); // 改为公有
    bool shouldReturnToModeSelect() const { return mReturnToModeSelect; }

private:
    // ===== 界面控制相关 =====
    std::vector<WINDOW *> mWindows;
    int mScreenHeight, mScreenWidth;
    int mGameBoardHeight, mGameBoardWidth;
    const int mInformationHeight = 6;
    const int mInstructionWidth = 18;

    void createInformationBoard();
    void renderInformationBoard() const;
    void createGameBoard();
    void renderGameBoard() const;
    void createInstructionBoard();
    void renderInstructionBoard() const;
    void renderBoards() const;

    // ===== 蛇、地图、食物基础信息 =====
    std::unique_ptr<Snake> mPtrSnake;
    std::unique_ptr<Map> mPtrMap;
    SnakeBody mFood;
    const char mSnakeSymbol = '@';
    const char mFoodSymbol = '#';
    const char mWallSymbol = '+';
    int mInitialSnakeLength = 3;

    // ===== 游戏设置与状态 =====
    int mPoints = 0;
    int mDifficulty = 0;
    int mDelay;
    const int mBaseDelay = 100;

    // 排行榜
    const std::string mRecordBoardFilePath = "record.dat";
    std::vector<int> mLeaderBoard;
    const int mNumLeaders = 5;
    bool readLeaderBoard();
    bool updateLeaderBoard();
    bool writeLeaderBoard();
    void renderLeaderBoard() const;
    void renderPoints() const;
    void renderDifficulty() const;

    // 食物与控制
    void createRamdonFood();
    void renderFood() const;
    void renderSnake() const;
    void renderMap() const;
    void controlSnake() const;
    void adjustDelay();
    void initializeGame();
    void runGame();
    bool renderRestartMenu() const;
    bool renderRestartMenu(bool isBattleMode) const;

    // 地图文件管理
    const std::string mDefaultMapName = "default";
    std::vector<std::string> mMapFiles = {"maps/map1.txt", "maps/map2.txt", "maps/map3.txt"};
    bool selectMap();

    // ========== 关卡模式 ==========
    int mCurrentLevel = 1;
    const int mMaxLevel = 5;
    GameMode mCurrentMode;
    LevelType mCurrentLevelType = LevelType::Normal;
    std::vector<LevelStatus> mLevelStatus;
    int mLevelTargetPoints = 5;
    bool mReturnToModeSelect = false;
    bool mIsLevelMode = false;
    const std::string mLevelProgressFilePath = "level_progress.dat";
    std::vector<std::string> mLevelMapFiles = {
        "maps/level1.txt", "maps/level2.txt", "maps/level3.txt",
        "maps/level4.txt", "maps/level5.txt"
    };

    void createDefaultLevelMaps();
    void initializeLevel(int level);
    void loadNextLevel();
    bool isLevelCompleted();
    void runLevel();
    bool selectLevelInLevelMode();
    void unlockLevel(int level);
    bool saveLevelProgress();
    bool loadLevelProgress();
    void renderLevel() const;

    // === 第四关特殊逻辑 ===
    void initializeLevel4();
    void runLevel4();
    void renderEndpoint() const;
    void controlSnakeLevel4() const;
    SnakeBody mEndpoint;
    const char mEndpointSymbol = 'X';
    const char mSingleKeyTurnSymbol = 'T';
    bool mHasEndpoint = false;

    // === 第五关 Boss 战 ===
    void initializeLevel5();
    void runLevel5();
    int mBossHP = 5;
    int mBossSize = 5;
    std::pair<int, int> mBossPosition;
    BossState mBossState = BossState::Red;

    float mBossStateDuration = 0.0f;
    const float mRedStateDuration = 6.0f;
    const float mGreenStateDuration = 3.0f;
    std::chrono::time_point<std::chrono::steady_clock> mBossStateStartTime;

    bool mSnakeInvincible = false;
    std::chrono::time_point<std::chrono::steady_clock> mInvincibleStartTime;
    const float mInvincibleDuration = 2.0f;

    SnakeBody mBossAttackPoint;
    int mBossStateTimer = 0;
    int mBossStateChangeDuration = 50;
    double mLaserAngle = 0.0;
    double mLaserRotationSpeed = 2.0;
    int mLaserLength = 0;

    void updateBossState();
    void renderBoss();
    void updateBossAttackPoint();
    void updateAndRenderLasers();
    void renderLaser(int x1, int y1, int x2, int y2, char symbol);
    bool checkLaserCollision();
    bool checkSnakeLaserCollision(const std::vector<SnakeBody>& snake, int x1, int y1, int x2, int y2);
    bool checkBossAttack();
    double pointToLineDistance(int x0, int y0, int x1, int y1, int x2, int y2);


    // ========== 限时模式 ==========
    void initializeTimeAttack();
    void runTimeAttack();
    void renderTimer() const;

    int mTimeAttackDurationSeconds = 120;
    int mTimeRemaining = 0;
    std::chrono::time_point<std::chrono::steady_clock> mTimeAttackStartTime;

    // ========== 对战模式 ==========
    BattleType mCurrentBattleType;
    std::unique_ptr<Snake> mPtrSnake2;
    std::unique_ptr<AI> mPtrAI;
    int mPoints2 = 0;
    const char mSnakeSymbol2 = '&';
    bool selectBattleType();
    void initializeBattle(BattleType type);
    void runBattle();
    void controlSnakes(int key);
    std::string checkBattleCollisions();
    void renderSnakes() const;
    void renderBattleStatus() const;
    void renderWinnerText(const std::string& winner) const;

    // 加速功能（对战模式用）
    const int mAccelDelay = 40;
    bool mAccelerateP1 = false;
    bool mAccelerateP2 = false;
};

#endif // GAME_H