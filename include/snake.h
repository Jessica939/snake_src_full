#ifndef SNAKE_H
#define SNAKE_H

#include <vector>

// 前向声明
class Map;
enum class InitialDirection;

enum class Direction
{
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3,
};

// 添加转弯模式枚举
enum class TurnMode
{
    Normal = 0,  // 正常模式：使用方向键控制
    SingleKey = 1,  // 单键模式：按一次键自动左转/右转
};

class SnakeBody
{
public:
    SnakeBody();
    SnakeBody(int x, int y);
    int getX() const;
    int getY() const;
    bool operator == (const SnakeBody& snakeBody);
private:
    int mX;
    int mY;
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
    bool isPartOfSnake(int x, int y);
    void senseFood(SnakeBody food);
    // Set map for collision detection
    void setMap(Map* map);
    // Check if hit wall
    bool hitWall();
    bool touchFood();
    bool hitSelf();
    bool checkCollision();
    bool changeDirection(Direction newDirection);
    std::vector<SnakeBody>& getSnake();
    int getLength();
    SnakeBody createNewHead();
    bool moveFoward();
    
    // 设置转弯模式
    void setTurnMode(TurnMode mode);
    // 获取当前转弯模式
    TurnMode getTurnMode() const;
    // 单键转弯控制
    bool singleKeyTurn();
    // 设置下一次单键转弯的方向
    void setNextTurnDirection(bool isLeftTurn);
    // 检查是否达到了终点
    bool reachedEndpoint(int endX, int endY) const;

private:
    const int mGameBoardWidth;
    const int mGameBoardHeight;
    // Snake information
    const int mInitialSnakeLength;
    Direction mDirection;
    SnakeBody mFood;
    std::vector<SnakeBody> mSnake;
    // Map pointer for collision detection
    Map* mMap;
    
    // 单键转向模式相关属性
    TurnMode mTurnMode = TurnMode::Normal;  // 默认为普通模式
    bool mNextTurnIsLeft = true;  // 下一次转向是左转还是右转
    
    // Convert InitialDirection to Direction
    Direction convertInitialDirection(InitialDirection dir);
};

#endif
