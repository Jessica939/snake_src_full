#ifndef SNAKE_H
#define SNAKE_H

#include <vector>

// 前向声明
class Map;

// 蛇初始方向枚举
enum class InitialDirection
{
    Left,
    Right,
    Up,
    Down
};

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
    SingleKey      // 单键转弯模式（第四关）
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
    // Set map for collision detection
    void setMap(Map* map);
    // Check if hit wall
    bool hitWall();
    bool touchFood();
    bool hitSelf();
    bool changeDirection(Direction newDirection);
    std::vector<SnakeBody>& getSnake();
    const std::vector<SnakeBody>& getSnake() const;
    int getLength() const;
    SnakeBody createNewHead();
    bool moveFoward();
    bool checkCollision() const;
    bool isPartOfSnake(int x, int y) const;
    void initializeSnake();
    void initializeSnake(int headX, int headY, InitialDirection dir = InitialDirection::Right);
    void setMap(const Map* map);
    int getLength() const;
    std::vector<SnakeBody>& getSnake();
    
    // 添加setRandomSeed方法声明
    void setRandomSeed();
    
    // 转弯模式相关
    void setTurnMode(TurnMode mode);
    void singleKeyTurn();
    
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

private:
    std::vector<SnakeBody> mSnakeBody;
    Direction mDirection;
    int mGameBoardWidth;
    int mGameBoardHeight;
    int mInitLength;
    SnakeBody mFood;
    
    // 地图指针，用于碰撞检测
    const Map* mPtrMap;
    
    // 转弯模式
    TurnMode mTurnMode = TurnMode::FourDirection;
    
    // 固定长度模式（用于第五关）
    bool mFixedLength = false;
};

#endif
