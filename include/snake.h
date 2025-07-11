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

    //AI蛇需要的相关函数
    SnakeBody& getHead();
    // 获取当前移动方向，AI需要
    Direction getDirection() const;
    // 获取当前方向的相反方向，用于防止AI掉头
    Direction getOppositeDirection() const;
    // 预测下一个头部位置，这是智能AI的核心辅助函数
    SnakeBody getNextHeadPosition(Direction dir) const;

    void grow(int num);


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
    
    // Convert InitialDirection to Direction
    Direction convertInitialDirection(InitialDirection dir);

    int mGrowNum = 0;
};

#endif

