#include <string>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "snake.h"
#include "map.h"


SnakeBody::SnakeBody()
{
}


SnakeBody::SnakeBody(int x, int y): mX(x), mY(y)
{
}

int SnakeBody::getX() const
{
    return mX;
}

int SnakeBody::getY() const
{
    return mY;
}

bool SnakeBody::operator == (const SnakeBody& snakeBody)
{
    return (this->getX() == snakeBody.getX() && this->getY() == snakeBody.getY());
}

Snake::Snake(int gameBoardWidth, int gameBoardHeight, int initialSnakeLength): mGameBoardWidth(gameBoardWidth), mGameBoardHeight(gameBoardHeight), mInitialSnakeLength(initialSnakeLength), mMap(nullptr)
{
    this->initializeSnake();
    this->setRandomSeed();
}

void Snake::setRandomSeed()
{
    // use current time as seed for random generator
    std::srand(std::time(nullptr));
}

void Snake::initializeSnake()
{
    // Instead of using a random initialization algorithm
    // We always put the snake at the center of the game mWindows
    int centerX = this->mGameBoardWidth / 2;
    int centerY = this->mGameBoardHeight / 2;

    // 清空旧的蛇身
    mSnake.clear();
    
    for (int i = 0; i < this->mInitialSnakeLength; i ++)
    {
        this->mSnake.push_back(SnakeBody(centerX, centerY + i));
    }
    this->mDirection = Direction::Up;
}

void Snake::initializeSnake(int startX, int startY)
{
    // 清空旧的蛇身
    mSnake.clear();
    
    // 根据提供的起始位置创建蛇
    for (int i = 0; i < this->mInitialSnakeLength; i ++)
    {
        // 根据初始方向向下构建蛇身
        this->mSnake.push_back(SnakeBody(startX, startY + i));
    }
    this->mDirection = Direction::Up;
}

void Snake::initializeSnake(int startX, int startY, InitialDirection direction)
{
    // 清空旧的蛇身
    mSnake.clear();
    
    int dx = 0, dy = 0;
    
    // 根据方向设置增量
    switch (direction)
    {
        case InitialDirection::Up:
            dy = -1;
            break;
        case InitialDirection::Down:
            dy = 1;
            break;
        case InitialDirection::Left:
            dx = -1;
            break;
        case InitialDirection::Right:
            dx = 1;
            break;
    }
    
    // 根据提供的起始位置和方向创建蛇
    // 确保蛇头位于初始位置，身体沿着方向的反方向延伸
    this->mSnake.push_back(SnakeBody(startX, startY)); // 蛇头
    
    for (int i = 1; i < this->mInitialSnakeLength; i ++)
    {
        this->mSnake.push_back(SnakeBody(startX - i * dx, startY - i * dy));
    }
    
    // 设置蛇的移动方向与初始方向一致
    this->mDirection = convertInitialDirection(direction);
}

Direction Snake::convertInitialDirection(InitialDirection dir)
{
    // 初始方向决定了蛇的身体排列方向，移动方向应与身体方向相反
    switch (dir)
    {
        case InitialDirection::Up:    // 蛇身体朝上排列
            return Direction::Up;     // 头部在最上方，应向上移动
        case InitialDirection::Down:  // 蛇身体朝下排列
            return Direction::Down;   // 头部在最下方，应向下移动
        case InitialDirection::Left:  // 蛇身体朝左排列
            return Direction::Left;   // 头部在最左方，应向左移动
        case InitialDirection::Right: // 蛇身体朝右排列
            return Direction::Right;  // 头部在最右方，应向右移动
    }
    return Direction::Up; // 默认值
}

bool Snake::isPartOfSnake(int x, int y)
{
    SnakeBody temp = SnakeBody(x, y);
    for (int i = 0; i < this->mSnake.size(); i ++)
    {
        if (this->mSnake[i] == temp)
        {
            return true;
        }
    }
    return false;
}

void Snake::setMap(Map* map)
{
    this->mMap = map;
}

/*
 * Assumption:
 * Only the head would hit wall.
 */
bool Snake::hitWall()
{
    SnakeBody& head = this->mSnake[0];
    int headX = head.getX();
    int headY = head.getY();
    
    // Check boundary walls
    if (headX <= 0 || headX >= this->mGameBoardWidth - 1)
    {
        return true;
    }
    if (headY <= 0 || headY >= this->mGameBoardHeight - 1)
    {
        return true;
    }
    
    // Check map walls if map is available
    if (mMap != nullptr && mMap->isWall(headX, headY))
    {
        return true;
    }
    
    return false;
}

/*
 * The snake head is overlapping with its body
 */
bool Snake::hitSelf()
{
    SnakeBody& head = this->mSnake[0];
    // Exclude the snake head
    for (int i = 1; i < this->mSnake.size(); i ++)
    {
        if (this->mSnake[i] == head)
        {
            return true;
        }
    }
    return false;
}


bool Snake::touchFood()
{
    SnakeBody newHead = this->createNewHead();
    if (this->mFood == newHead)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Snake::senseFood(SnakeBody food)
{
    this->mFood = food;
}

std::vector<SnakeBody>& Snake::getSnake()
{
    return this->mSnake;
}

bool Snake::changeDirection(Direction newDirection)
{
    switch (this->mDirection)
    {
        case Direction::Up:
        {
            if (newDirection == Direction::Left || newDirection == Direction::Right)
            {
                this->mDirection = newDirection;
                return true;
            }
            else
            {
                return false;
            }
        }
        case Direction::Down:
        {
            if (newDirection == Direction::Left || newDirection == Direction::Right)
            {
                this->mDirection = newDirection;
                return true;
            }
            else
            {
                return false;
            }
        }
        case Direction::Left:
        {
            if (newDirection == Direction::Up || newDirection == Direction::Down)
            {
                this->mDirection = newDirection;
                return true;
            }
            else
            {
                return false;
            }
        }
        case Direction::Right:
        {
            if (newDirection == Direction::Up || newDirection == Direction::Down)
            {
                this->mDirection = newDirection;
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}


SnakeBody Snake::createNewHead()
{
    SnakeBody& head = this->mSnake[0];
    int headX = head.getX();
    int headY = head.getY();
    int headXNext;
    int headYNext;

    switch (this->mDirection)
    {
        case Direction::Up:
        {
            headXNext = headX;
            headYNext = headY - 1;
            break;
        }
        case Direction::Down:
        {
            headXNext = headX;
            headYNext = headY + 1;
            break;
        }
        case Direction::Left:
        {
            headXNext = headX - 1;
            headYNext = headY;
            break;
        }
        case Direction::Right:
        {
            headXNext = headX + 1;
            headYNext = headY;
            break;
        }
    }

    SnakeBody newHead = SnakeBody(headXNext, headYNext);

    return newHead;
}

/*
 * If eat food, return true, otherwise return false
 */
// snake.cpp
bool Snake::moveFoward()
{
    // 这个判断结果将决定本回合是否发生“普通增长”，并作为函数的最终返回值。
    bool ateMainFood = this->touchFood();

    // 确定新蛇头的位置
    SnakeBody newHead;
    if (ateMainFood) {
        newHead = this->mFood;
    } else {
        newHead = this->createNewHead();
    }

    // 无论是否增长，蛇头总要向前移动。我们将新蛇头插入到身体的最前端。
    this->mSnake.insert(this->mSnake.begin(), newHead);

    if (mGrowNum > 0) {
        mGrowNum--;
    }
    else if (ateMainFood) {
    }
    else {
        this->mSnake.pop_back();
    }
    return ateMainFood;
}

bool Snake::checkCollision()
{
    // 检查是否撞墙
    if (this->hitWall())
    {
        return true;
    }
    
    // 检查是否撞到自己
    if (this->hitSelf())
    {
        return true;
    }
    
    return false;
}


int Snake::getLength()
{
    return this->mSnake.size();
}

SnakeBody& Snake::getHead() 
{
    return this->mSnake[0];
}

Direction Snake::getDirection() const
{
    return this->mDirection;
}

Direction Snake::getOppositeDirection() const
{
    switch (this->mDirection)
    {
        case Direction::Up:    return Direction::Down;
        case Direction::Down:  return Direction::Up;
        case Direction::Left:  return Direction::Right;
        case Direction::Right: return Direction::Left;
    }
    return Direction::Up; 
}

SnakeBody Snake::getNextHeadPosition(Direction dir) const
{
    if (mSnake.empty()) return SnakeBody(0,0);

    const SnakeBody& head = mSnake[0];
    int nextX = head.getX();
    int nextY = head.getY();

    switch (dir)
    {
        case Direction::Up:    nextY--; break;
        case Direction::Down:  nextY++; break;
        case Direction::Left:  nextX--; break;
        case Direction::Right: nextX++; break;
    }
    return SnakeBody(nextX, nextY);
}

void Snake::grow(int num)
{
    this->mGrowNum += num;
}