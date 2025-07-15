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
#include <set>
#include <map>

// 自定义模块
#include "snake.h"
#include "map.h"
// #include "ai.h" // 移除
#include "food_type.h"
class AI;

// ========== 枚举定义 ==========
enum class GameMode { Classic, Level, Timed, Battle ,Shop};
enum class LevelType { Normal, Speed, Maze, Custom1, Custom2 };
enum class LevelStatus { Locked, Unlocked, Completed };
enum class BossState { Red, Green };
enum class BattleType { PlayerVsPlayer, PlayerVsAI };

// 蛇皮肤枚举
enum class SnakeSkin {
    Default = 0,
    Red = 1,
    Blue = 2,
    Green = 3,
    Yellow = 4
};

// 道具枚举
enum class ItemType {
    Portal = 0,
    RandomBox = 1,
    Cheat = 2,
    Attack = 3,
    Shield = 4,
    Poison = 5  // 毒药类型
};

// ========== 游戏主类 ==========
class Game {
public:
    Game();
    ~Game();

    void startGame();

    // 公共函数
    bool selectLevel(); // 改为公有
    bool shouldReturnToModeSelect() const { return mReturnToModeSelect; }
    void initializeLevel(int level); // 移到public以供main.cpp调用
    
    // 新增：直接启动特定关卡的方法
    void startLevelDirectly(int level);
    void setGameMode(GameMode mode);
    bool isLevelCompleted();  // 检查关卡是否完成

    // 商店和皮肤相关
    void showShopMenu(); // 商店界面
    void showShopMenu_Skin(); // 皮肤商店界面
    void showShopMenu_Item(); // 道具商店界面
    void savePlayerProfile() const; // 持久化保存
    void loadPlayerProfile();       // 持久化读取
    void setSnakeSkin(SnakeSkin skin); // 切换皮肤
    SnakeSkin getSnakeSkin() const;    // 获取当前皮肤
    int getCoins() const;              // 获取金币
    void addCoins(int amount);         // 增加金币
    bool buySkin(SnakeSkin skin, int price); // 购买皮肤
    bool hasSkin(SnakeSkin skin) const;     // 是否已拥有皮肤

    // 道具相关
    void showItemShopMenu(); // 道具商店界面
    void saveItemInventory() const;
    void loadItemInventory();
    bool buyItem(ItemType item, int price);
    int getItemCount(ItemType item) const;
    void addItem(ItemType item, int count = 1);
    bool useItem(ItemType item); // 使用道具（后续实现）
    
    // 存档相关
    void saveGame() const; // 保存游戏状态
    bool loadGame(); // 加载游戏状态
    bool hasSaveFile() const; // 检查是否有存档文件
    void deleteSaveFile() const; // 删除存档文件

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
    SnakeBody mPoison;  // 新增毒药位置
    const char mSnakeSymbol = '@';
    const char mFoodSymbol = '#';
    const char mPoisonSymbol = 'P';  // 新增毒药符号
    const char mWallSymbol = '+';
    int mInitialSnakeLength = 3;
    bool mHasPoison = false;  // 新增是否有毒药的标志
    
    // 新增：食物和道具系统
    FoodType mCurrentFoodType = FoodType::Normal;
    SnakeBody mSpecialFood;
    bool mHasSpecialFood = false;
    const char mSpecialFoodSymbol = '&'; // 改为&符号，更容易识别
    std::chrono::time_point<std::chrono::steady_clock> mSpecialFoodSpawnTime;
    const float mSpecialFoodDuration = 5.0f;
    
    // 尸体食物系统
    std::vector<SnakeBody> mCorpseFoods; // 存储尸体食物的位置
    const char mCorpseFoodSymbol = 'C'; // 尸体食物符号
    
    // 随机道具系统
    SnakeBody mRandomItem;
    bool mHasRandomItem = false;
    ItemType mCurrentRandomItemType = ItemType::Portal;
    const char mRandomItemSymbol = '$';
    std::chrono::time_point<std::chrono::steady_clock> mRandomItemSpawnTime;
    const float mRandomItemDuration = 5.0f;
    
    // 毒药
    std::chrono::time_point<std::chrono::steady_clock> mPoisonSpawnTime;
    const float mPoisonDuration = 5.0f;

    // ===== 游戏设置与状态 =====
    int mPoints = 0;
    int mDifficulty = 0;
    int mDelay;
    const int mBaseDelay = 100;
    int mBattleBaseDelay = 150; // 对战模式基础延迟
    
    // 生命值系统
    int mPlayerLives = 3; // 玩家生命值
    int mPlayer2Lives = 3; // 玩家2/AI生命值

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
    
    // 存档文件路径
    const std::string mSaveFilePath = "game_save.dat";

    // 食物与控制
    void createRamdonFood();
    void createPoison();  // 新增生成毒药函数
    void createSpecialFood();  // 新增生成特殊食物
    void createCorpseFoods(const std::vector<SnakeBody>& snakeBody); // 新增生成尸体食物
    void createRandomItem();   // 新增生成随机道具
    void renderFood() const;
    void renderPoison() const;  // 新增渲染毒药函数
    void renderSpecialFood() const;  // 新增渲染特殊食物
    void renderCorpseFoods() const;  // 新增渲染尸体食物
    void renderRandomItem() const;   // 新增渲染随机道具
    void renderSnake() const;
    void renderMap() const;
    void controlSnake() const;
    void adjustDelay();
    void adjustBattleDelay(); // 对战模式专用延迟调整
    void initializeGame();
    void runGame();
    bool renderRestartMenu() const;
    bool renderRestartMenu(bool isBattleMode) const;
    int renderLevelFailureMenu() const;     // 新增：关卡失败菜单

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
    bool mIsLevelRetry = false; // 标记是否是重试关卡
    const std::string mLevelProgressFilePath = "level_progress.dat";
    std::vector<std::string> mLevelMapFiles = {
        "maps/level1.txt", "maps/level2.txt", "maps/level3.txt",
        "maps/level4.txt", "maps/level5.txt"
    };
    // ====== 关卡3模式选择 ======
    int mLevel3ModeChoice = 0; // 0: alone, 1: with ally

    void createDefaultLevelMaps();
    void loadNextLevel();
    void runLevel();
    bool selectLevelInLevelMode();
    void unlockLevel(int level);
    bool saveLevelProgress();
    bool loadLevelProgress();
    void renderLevel() const;
    void displayLevelIntroduction(int level); // 显示关卡开场介绍文字
    void displayLevelCompletion(int level);   // 显示关卡通关后的文字叙述
    void runLevel3Mode1();                    // 第三关模式一：镜像之舞
    void runLevel3Mode2();                    // 第三关模式二：协作模式

    // === 第四关特殊逻辑 ===
    void initializeLevel4();
    void runLevel4();
    void renderEndpoint() const;
    void controlSnakeLevel4() const;
    SnakeBody mEndpoint;
    const char mEndpointSymbol = 'X';
    const char mSingleKeyTurnSymbol = 'T';
    bool mHasEndpoint = false;

    // 视窗跟随相关变量
    int mViewOffsetX = 0; // 视窗X方向偏移
    int mViewOffsetY = 0; // 视窗Y方向偏移
    bool mUseViewport = false; // 是否启用视窗跟随
    void updateViewport(); // 更新视窗位置，让蛇居中

    // === 第三关模式一特殊逻辑 ===
    std::vector<SnakeBody> mLevel3Mode1Foods; // 第三关模式一的10个固定食物位置
    int mLevel3FoodIndex = 0; // 当前食物索引
    void initializeLevel3Mode1Foods(); // 初始化固定食物位置
    void setNextLevel3Mode1Food(); // 设置下一个固定食物

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
    const int mAccelDelay = 35; // 对战模式加速延迟，不要太快
    bool mAccelerateP1 = false;
    bool mAccelerateP2 = false;

    // 皮肤和金币相关
    int mCoins = 100; // 初始金币
    SnakeSkin mCurrentSkin = SnakeSkin::Default;
    std::set<SnakeSkin> mOwnedSkins = {SnakeSkin::Default};

    // 道具库存
    std::map<ItemType, int> mItemInventory; // item->count
    
    // 道具使用相关
    bool mCheatMode = false; // 作弊模式状态
    std::chrono::time_point<std::chrono::steady_clock> mCheatStartTime; // 作弊模式开始时间
    const float mCheatDuration = 10.0f; // 作弊模式持续时间（秒）

    // 护盾相关
    bool mShieldActive = false; // 是否有护盾保护
    void activateShield();      // 激活护盾
    void deactivateShield();    // 关闭护盾
    bool isShieldActive() const; // 检查护盾是否激活
    
    // 长按加速相关
    std::chrono::time_point<std::chrono::steady_clock> mLastKeyPressTime; // 上次按键时间
    Direction mLastKeyDirection = Direction::Right; // 上次按键方向
    bool mAccelerating = false; // 是否正在加速
    const int mAccelerateDelay = 25; // 其他模式加速时的延迟（毫秒）- 让加速更明显
    
    // 道具使用函数
    void activateCheatMode(); // 激活作弊模式
    void deactivateCheatMode(); // 停用作弊模式
    bool isCheatModeActive() const; // 检查作弊模式是否激活
    void usePortal(); // 使用传送门
    void useAttack(); // 使用攻击道具
    void handleItemUsage(int key); // 处理道具使用
    void updateCheatMode(); // 更新作弊模式状态
    void handleAcceleration(int key); // 处理长按加速
    bool isKeyPressed(int key); // 检查按键是否被按下
    
    // 新增：食物效果处理
    int getFoodEffect(FoodType foodType) const; // 获取食物效果
    void handleFoodEffect(FoodType foodType);   // 处理食物效果
    bool touchFood();
    bool touchPoison() const;  // 新增检测是否碰到毒药
    bool touchSpecialFood() const;  // 新增检测是否碰到特殊食物
    bool touchCorpseFood() const;   // 新增检测是否碰到尸体食物
    bool touchRandomItem() const;   // 新增检测是否碰到随机道具
};

#endif // GAME_H
