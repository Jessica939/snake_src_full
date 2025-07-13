#ifndef SNAKE_H
#define SNAKE_H

#include <vector>

// 前向声明
class Map;

// 蛇初始方向枚举
enum class InitialDirection;


// 方向枚举
enum class Direction
{
    Left,
    Right,
    Up,
    Down
};

// 转向模式枚举
enum class TurnMode
{
    FourDirection, // 普通的四向控制
    SingleKey,     // 单键转弯模式（第四关）
    AutoTurn       // 自动转向模式（新增）
};

// 蛇身体部分
class SnakeBody
{
public:
    SnakeBody();
    SnakeBody(int x, int y);
    int getX() const;
    int getY() const;
    bool operator == (const SnakeBody& snakeBody) const;
private:
    int mX, mY;
};

// Snake class should have no depency on the GUI library
class Snake
{
public:
    //Snake();
    Snake(int gameBoardWidth, int gameBoardHeight, int initialSnakeLength);
    // Set random seed
    void setRandomSeed();
    // Initialize snake
    void initializeSnake();
    // Initialize snake at specific position
    void initializeSnake(int startX, int startY);
    // Initialize snake at specific position with specific direction
    void initializeSnake(int startX, int startY, InitialDirection direction);
    // Check if the snake is on the coordinate
    // bool isSnakeOn(int x, int y);
    // Checking API for generating random food
    bool isPartOfSnake(int x, int y) const;
    void senseFood(SnakeBody food);
    void sensePoison(SnakeBody poison);  // 新增感知毒药
    void senseSpecialFood(SnakeBody specialFood);  // 新增感知特殊食物
    void senseRandomItem(SnakeBody randomItem);    // 新增感知随机道具
    void senseCorpseFoods(const std::vector<SnakeBody>& corpseFoods); // 新增感知尸体食物
    // Set map for collision detection
    void setMap(Map* map);
    // Check if hit wall
    bool hitWall();
    bool touchFood();
    bool touchPoison() const;  // 新增检测是否碰到毒药
    bool touchSpecialFood() const;  // 新增检测是否碰到特殊食物
    bool touchRandomItem() const;   // 新增检测是否碰到随机道具
    bool touchCorpseFood() const;   // 新增检测是否碰到尸体食物
    SnakeBody getEatenCorpseFood() const; // 新增获取被吃掉的尸体食物位置
    bool hitSelf();
    bool changeDirection(Direction newDirection);
    std::vector<SnakeBody>& getSnake();
    const std::vector<SnakeBody>& getSnake() const;
    int getLength() const;
    SnakeBody createNewHead() const;  // 添加const
    bool moveFoward();
    bool checkCollision() const;
    
    
    void setMap(const Map* map);
    
    
    // 转弯模式相关
    void setTurnMode(TurnMode mode);
    void singleKeyTurn();
    
    // 自动转向功能 - 根据当前位置和周围墙体情况自动选择转向方向
    void autoTurn();

    // 设置蛇的固定长度模式（用于第五关）
    void setFixedLength(bool fixed);
    bool isFixedLength() const;
    
    // 检查是否到达终点（用于第四关）
    bool reachedEndpoint(int x, int y) const;
    
    // 获取当前转弯模式
    TurnMode getTurnMode() const;

    // 设置下一次单键转弯的方向
    void setNextTurnDirection(bool isLeftTurn);

    Direction getDirection() const;
    
    // 护盾相关：撤销移动（将蛇头移回安全位置）
    void undoMove();
    
    // 无敌模式相关
    void setInvincible(bool invincible); // 设置无敌模式
    bool isInvincible() const; // 检查是否无敌
    
    // 生命值相关
    void setLives(int lives); // 设置生命值
    int getLives() const; // 获取当前生命值
    bool loseLife(); // 失去一条生命，返回是否还有剩余生命
    bool isAlive() const; // 检查是否还活着

private:
    std::vector<SnakeBody> mSnakeBody;
    Direction mDirection;
    int mGameBoardWidth;
    int mGameBoardHeight;
    int mInitLength;
    SnakeBody mFood;
    SnakeBody mPoison;  // 新增毒药位置
    SnakeBody mSpecialFood;  // 新增特殊食物位置
    SnakeBody mRandomItem;   // 新增随机道具位置
    std::vector<SnakeBody> mCorpseFoods; // 新增尸体食物位置列表
    
    // 地图指针，用于碰撞检测
    const Map* mPtrMap;
    
    // 转弯模式
    TurnMode mTurnMode = TurnMode::FourDirection;
    
    // 固定长度模式（用于第五关）
    bool mFixedLength = false;
    
    // 无敌模式
    bool mInvincible = false;
    
    // 护盾相关：记录上一帧蛇头位置
    SnakeBody mPreviousHead;
    
    // 生命值系统
    int mLives = 3; // 默认3条生命
    bool mIsAlive = true; // 是否还活着
};

#endif // SNAKE_H

