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

bool SnakeBody::operator == (const SnakeBody& snakeBody) const
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

bool Snake::isPartOfSnake(int x, int y) const
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
    // 优化：防止蛇180度掉头，但允许保持原方向（实现直行）
    if (mDirection == Direction::Up && newDirection == Direction::Down) return false;
    if (mDirection == Direction::Down && newDirection == Direction::Up) return false;
    if (mDirection == Direction::Left && newDirection == Direction::Right) return false;
    if (mDirection == Direction::Right && newDirection == Direction::Left) return false;
    
    this->mDirection = newDirection;
    return true; // 返回true表示方向已成功改变或保持
}

// 设置转弯模式
void Snake::setTurnMode(TurnMode mode)
{
    mTurnMode = mode;
}

// 获取当前转弯模式
TurnMode Snake::getTurnMode() const
{
    return mTurnMode;
}

// 单键转弯控制
bool Snake::singleKeyTurn()
{
    // 根据当前方向和下一次转向是左转还是右转，确定新方向
    Direction newDirection;
    
    switch (this->mDirection)
    {
        case Direction::Up:
            newDirection = mNextTurnIsLeft ? Direction::Left : Direction::Right;
            break;
        case Direction::Down:
            newDirection = mNextTurnIsLeft ? Direction::Right : Direction::Left;
            break;
        case Direction::Left:
            newDirection = mNextTurnIsLeft ? Direction::Down : Direction::Up;
            break;
        case Direction::Right:
            newDirection = mNextTurnIsLeft ? Direction::Up : Direction::Down;
            break;
    }
    
    // 更新下一次转向方向（交替左右转）
    mNextTurnIsLeft = !mNextTurnIsLeft;
    
    // 改变方向
    this->mDirection = newDirection;
    return true;
}

// 设置下一次单键转弯的方向
void Snake::setNextTurnDirection(bool isLeftTurn)
{
    mNextTurnIsLeft = isLeftTurn;
}

// 检查是否达到了终点
bool Snake::reachedEndpoint(int endX, int endY) const
{
    // 检查蛇头是否位于终点位置
    const SnakeBody& head = this->mSnake[0];
    return (head.getX() == endX && head.getY() == endY);
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
bool Snake::moveFoward()
{
    if (this->touchFood())
    {
        SnakeBody newHead = this->mFood;
        this->mSnake.insert(this->mSnake.begin(), newHead); 
        return true;
    }
    else
    {
        this->mSnake.pop_back();
        SnakeBody newHead = this->createNewHead();
        this->mSnake.insert(this->mSnake.begin(), newHead); 
        return false;
    }
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


int Snake::getLength() const
{
    return this->mSnake.size();
}

const std::vector<SnakeBody>& Snake::getSnake() const {
    return this->mSnake;
}

Direction Snake::getDirection() const {return mDirection;}