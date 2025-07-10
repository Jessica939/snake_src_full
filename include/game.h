#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "snake.h"

enum class ItemType {
    NONE,        //无道具
    HELMET,      // 头盔
    GROW,        // 变大
    SHRINK,      // 变小
    SPEED_UP,    // 加速
    SLOW_DOWN,   // 减速
    PORTAL,      // 传送门
    RANDOM_BOX   // 随机盒子
};

struct ActiveItem {
    bool active = false;
    SnakeBody pos;
    ItemType type;
    int lifetime; // 存在时间
    char symbol;  // 在地图上显示的符号
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
    
		void createRamdonFood();
    void renderFood() const;
    void renderSnake() const;
    void controlSnake() ;
    
		void startGame();
    bool renderRestartMenu() const;
    void adjustDelay();

    void updateExtraFood();

    void renderLives() const;

    void showShop(); // 显示商店菜单
    void spawnRandomItem(); // 在地图上生成随机道具
    void renderSidePanel() const; // 渲染侧边面板，包括金钱、库存和道具
    void renderMoneyAndInventory(int& currentY) const; // 渲染金钱和库存

    void useRandomBox();

    void spawnObstacles(int count);
    void renderObstacles() const;
    bool checkObstacleCollision(const SnakeBody& pos) const;

    bool saveGame();
    bool loadGame();

    int renderMainMenu() const;
    

private:
    // We need to have two windows
    // One is for game introduction
    // One is for game mWindows
    int mScreenWidth;
    int mScreenHeight;
    int mGameBoardWidth;
    int mGameBoardHeight;
    int mEffectiveWidth; //有效边界的宽度，在边界缩小中有用
    int mEffectiveHeight;
    const int mInformationHeight = 6;
    const int mInstructionWidth = 18;
    std::vector<WINDOW *> mWindows;
    // Snake information
    const int mInitialSnakeLength = 2;
    const char mSnakeSymbol = '@';
    std::unique_ptr<Snake> mPtrSnake;
    // Food information
    SnakeBody mFood;
    ExtraFood mSpecialFood;    // 特殊食物（+2 或 +4）
    ExtraFood mPoisonFood;    //毒药
    bool mHasPoison = false; //记录当前生成的是毒药
    const char mFoodSymbol = '#';
    const char mSpecial2Symbol = '$';   // 特殊食物 +2
    const char mSpecial4Symbol = '*';   // 特殊食物 +4
    const char mPoisonSymbol = '^';     // 毒药。  
    
    int mPoints = 0;
    int mDifficulty = 0;
    int mBaseDelay = 100;
    int mDelay;
    const std::string mRecordBoardFilePath = "record.dat";
    const std::string mSaveFilePath = "game_save.dat";
    std::vector<int> mLeaderBoard;
    const int mNumLeaders = 3;
    
    //边界缩小参数
    bool mShrunkBoundary = false;
    int mShrinkDuration = 0; // 持续帧数 
    bool mBoundaryHasShrunkThisRound = false; //设置这个参数后每轮边界只会缩小一次

    int mMoney = 20; // 初始金钱
    std::map<ItemType, int> mInventory; // 玩家拥有的道具库存 <道具类型, 数量>
    ActiveItem mActiveItem; // 当前地图上活动的道具
    
    std::vector<SnakeBody> mObstacles;
    const char mObstacleSymbol = 'X'; // 障碍物在地图上显示的符号
    bool mIsPhasing = false;    // 一个标志，用于判断玩家本帧是否尝试穿墙

    bool mCheatMode = false;//作弊机制
};

#endif
