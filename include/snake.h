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
    // 默认构造函数
    SnakeBody() : mX(0), mY(0) {}
    
    SnakeBody(int x, int y)
    {
        this->mX = x;
        this->mY = y;
    }

    int getX() const
    {
        return this->mX;
    }

    int getY() const
    {
        return this->mY;
    }

    void setX(int x)
    {
        this->mX = x;
    }

    void setY(int y)
    {
        this->mY = y;
    }

    bool operator==(const SnakeBody& sb) const
    {
        return ((this->mX == sb.mX) && (this->mY == sb.mY));
    }

private:
    int mX, mY;
};

// Snake class should have no depency on the GUI library
class Snake
{
public:
    Snake(int gameBoardWidth, int gameBoardHeight, int initLength);
    bool moveFoward();
    bool checkCollision() const;
    bool isPartOfSnake(int x, int y) const;
    void changeDirection(const Direction& dir);
    void initializeSnake();
    void initializeSnake(int headX, int headY, InitialDirection dir = InitialDirection::Right);
    void setMap(const Map* map);
    void senseFood(const SnakeBody& food);
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
