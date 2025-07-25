#include <string>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <algorithm>

#include "snake.h"
#include "map.h"


SnakeBody::SnakeBody()
{

}

Snake::Snake(int gameBoardWidth, int gameBoardHeight, int initLength)
    : mGameBoardWidth(gameBoardWidth), mGameBoardHeight(gameBoardHeight), mInitLength(initLength),
      mPtrMap(nullptr), mFixedLength(false), mInvincible(false), mLives(3), mIsAlive(true)
{
    this->initializeSnake();
    this->setRandomSeed();
    this->mTurnMode = TurnMode::FourDirection;
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
    mSnakeBody.clear();
    
    for (int i = 0; i < this->mInitLength; i ++)
    {
        this->mSnakeBody.push_back(SnakeBody(centerX, centerY + i));
    }
    this->mDirection = Direction::Up;
}

void Snake::initializeSnake(int startX, int startY)
{
    // 清空旧的蛇身
    mSnakeBody.clear();
    
    // 根据提供的起始位置创建蛇
    for (int i = 0; i < this->mInitLength; i ++)
    {
        // 根据初始方向向下构建蛇身
        this->mSnakeBody.push_back(SnakeBody(startX, startY + i));
    }
    this->mDirection = Direction::Up;
}


void Snake::initializeSnake(int headX, int headY, InitialDirection dir)
{
    // 清空旧的蛇身
    mSnakeBody.clear();
    
    int dx = 0, dy = 0;
    
    // 根据方向设置增量
    switch (dir)
    {
        case InitialDirection::Up:
            dy = 1;
            this->mDirection = Direction::Up;
            break;
        case InitialDirection::Down:
            dy = -1;
            this->mDirection = Direction::Down;
            break;
        case InitialDirection::Left:
            dx = 1;
            this->mDirection = Direction::Left;
            break;
        case InitialDirection::Right:
            dx = -1;
            this->mDirection = Direction::Right;
            break;
    }
    
    // 创建蛇头
    mSnakeBody.push_back(SnakeBody(headX, headY));
    
    // 根据方向创建蛇身
    for (int i = 1; i < this->mInitLength; i++) {
        mSnakeBody.push_back(SnakeBody(headX + dx * i, headY + dy * i));
    }
}

bool Snake::isPartOfSnake(int x, int y) const
{
    SnakeBody temp = SnakeBody(x, y);
    for (size_t i = 0; i < this->mSnakeBody.size(); i++)
    {
        if (this->mSnakeBody[i] == temp)
        {
            return true;
        }
    }
    return false;
}

void Snake::setMap(Map* map)
{
    this->mPtrMap = map;
}

bool Snake::checkCollision() const
{
    // 如果处于无敌模式，不检测碰撞
    if (mInvincible) {
        return false;
    }
    
    // 获取蛇头
    const SnakeBody& head = this->mSnakeBody[0];
    int headX = head.getX();
    int headY = head.getY();
    
    // 检查是否撞到地图边界（确保只有蛇头真正进入边界时才判定为碰撞）
    // 第四关（单键转向模式）特殊处理：不检查y=18的边界限制
    if (mTurnMode == TurnMode::SingleKey) {
        // 仅检查x轴边界和y轴最小边界，允许y值可以达到18及更大
        if (headX < 0 || headX > this->mGameBoardWidth - 1 || headY < 0) {
            return true;
        }
    } else {
        // 其他关卡正常检查所有边界
        if (headX < 0 || headX > this->mGameBoardWidth - 1 ||
            headY < 0 || headY > this->mGameBoardHeight - 1) {
            return true;
        }
    }
    
    // 检查是否撞到地图中的墙（确保只有蛇头真正进入墙时才判定为碰撞）
    if (mPtrMap != nullptr && mPtrMap->isWall(headX, headY)) {
        return true;
    }
    
    // 检查是否撞到自己（除了蛇头外的身体部分）
    // 在第五关（固定长度模式）中，不检查蛇身碰撞
    if (!mFixedLength) {
        for (size_t i = 1; i < this->mSnakeBody.size(); i++) {
            if (head.getX() == this->mSnakeBody[i].getX() &&
                head.getY() == this->mSnakeBody[i].getY()) {
                return true;
            }
        }
    }
    
    return false;
}

void Snake::senseFood(SnakeBody food)
{
    this->mFood = food;
}

void Snake::sensePoison(SnakeBody poison)
{
    this->mPoison = poison;
}

void Snake::senseSpecialFood(SnakeBody specialFood)
{
    this->mSpecialFood = specialFood;
}

void Snake::senseRandomItem(SnakeBody randomItem)
{
    this->mRandomItem = randomItem;
}

void Snake::senseCorpseFoods(const std::vector<SnakeBody>& corpseFoods)
{
    this->mCorpseFoods = corpseFoods;
}

std::vector<SnakeBody>& Snake::getSnake()
{
    return this->mSnakeBody;
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

void Snake::setTurnMode(TurnMode mode)
{
    mTurnMode = mode;
}

void Snake::singleKeyTurn()
{
    // 获取蛇头位置
    if (mSnakeBody.empty() || mPtrMap == nullptr) return;
    
    const SnakeBody& head = mSnakeBody[0];
    int headX = head.getX();
    int headY = head.getY();
    
    // 检查左右两侧是否有墙
    int leftX = headX, leftY = headY;
    int rightX = headX, rightY = headY;
    
    // 计算左侧和右侧的坐标
    switch (mDirection)
    {
        case Direction::Up:
            leftX -= 1;  // 左侧
            rightX += 1; // 右侧
            break;
        case Direction::Down:
            leftX += 1;  // 左侧（相对于向下移动的蛇）
            rightX -= 1; // 右侧
            break;
        case Direction::Left:
            leftY += 1;  // 左侧（相对于向左移动的蛇）
            rightY -= 1; // 右侧
            break;
        case Direction::Right:
            leftY -= 1;  // 左侧（相对于向右移动的蛇）
            rightY += 1; // 右侧
            break;
    }
    
    bool leftWall = mPtrMap->isWall(leftX, leftY);
    bool rightWall = mPtrMap->isWall(rightX, rightY);
    
    // 根据左右墙壁情况决定转向方向
    if (!leftWall && rightWall)
    {
        // 左侧无墙，向左转
        switch (mDirection)
        {
            case Direction::Up:
                mDirection = Direction::Left;
                break;
            case Direction::Left:
                mDirection = Direction::Down;
                break;
            case Direction::Down:
                mDirection = Direction::Right;
                break;
            case Direction::Right:
                mDirection = Direction::Up;
                break;
        }
    }
    else if (leftWall && !rightWall)
    {
        // 右侧无墙，向右转
        switch (mDirection)
        {
            case Direction::Up:
                mDirection = Direction::Right;
                break;
            case Direction::Right:
                mDirection = Direction::Down;
                break;
            case Direction::Down:
                mDirection = Direction::Left;
                break;
            case Direction::Left:
                mDirection = Direction::Up;
                break;
        }
    }
    else
    {
        // 两侧都有墙或都没有墙，默认左转
        // 这种情况通常不应该发生在设计合理的迷宫中
        switch (mDirection)
        {
            case Direction::Up:
                mDirection = Direction::Left;
                break;
            case Direction::Left:
                mDirection = Direction::Down;
                break;
            case Direction::Down:
                mDirection = Direction::Right;
                break;
            case Direction::Right:
                mDirection = Direction::Up;
                break;
        }
    }
}

bool Snake::moveFoward()
{
    // 记录当前蛇头位置（用于护盾撤销移动）
    if (!mSnakeBody.empty()) {
        mPreviousHead = mSnakeBody[0];
    }
    
    if (this->touchFood())
    {
        SnakeBody newHead = this->mFood;
        this->mSnakeBody.insert(this->mSnakeBody.begin(), newHead);
        return true;
    }
    else if (this->touchCorpseFood())
    {
        // 处理尸体食物，蛇会增长
        SnakeBody newHead = this->createNewHead();
        this->mSnakeBody.insert(this->mSnakeBody.begin(), newHead);
        return true; // 返回true表示吃到了食物
    }
    else
    {
        this->mSnakeBody.pop_back();
        SnakeBody newHead = this->createNewHead();
        this->mSnakeBody.insert(this->mSnakeBody.begin(), newHead);
        return false;
    }
}


void Snake::setFixedLength(bool fixed)
{
    this->mFixedLength = fixed;
}

bool Snake::isFixedLength() const
{
    return this->mFixedLength;
}

bool Snake::reachedEndpoint(int x, int y) const
{
    // 判断蛇头是否到达指定坐标（终点）
    if (!mSnakeBody.empty()) {
        return (mSnakeBody[0].getX() == x && mSnakeBody[0].getY() == y);
    }
    return false;
}


int Snake::getLength() const
{
    return this->mSnakeBody.size();
}

const std::vector<SnakeBody>& Snake::getSnake() const {
    return this->mSnakeBody;
}

Direction Snake::getDirection() const {return mDirection;}

TurnMode Snake::getTurnMode() const 
{
    return mTurnMode;
}

bool Snake::hitWall()
{
    SnakeBody& head = this->mSnakeBody[0];
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
    if (mPtrMap != nullptr && mPtrMap->isWall(headX, headY))
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
    SnakeBody& head = this->mSnakeBody[0];
    // Exclude the snake head
    for (int i = 1; i < this->mSnakeBody.size(); i ++)
    {
        if (this->mSnakeBody[i] == head)
        {
            return true;
        }
    }
    return false;
}

SnakeBody Snake::createNewHead() const
{
    const SnakeBody& head = this->mSnakeBody[0];
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

void Snake::autoTurn()
{
    // 获取蛇头位置
    if (mSnakeBody.empty() || mPtrMap == nullptr) return;
    
    const SnakeBody& head = mSnakeBody[0];
    int headX = head.getX();
    int headY = head.getY();
    
    // 检查前方是否有墙
    int frontX = headX;
    int frontY = headY;
    // 计算前方坐标
    switch (mDirection)
    {
        case Direction::Up:
            frontY -= 1;
            break;
        case Direction::Down:
            frontY += 1;
            break;
        case Direction::Left:
            frontX -= 1;
            break;
        case Direction::Right:
            frontX += 1;
            break;
    }
    
    // 如果前方有墙，则需要转向
    bool frontWall = mPtrMap->isWall(frontX, frontY);
    if (frontWall)
    {
        // 检查左右两侧是否有墙
        int leftX = headX, leftY = headY;
        int rightX = headX, rightY = headY;
        
        // 计算左侧和右侧的坐标
        switch (mDirection)
        {
            case Direction::Up:
                leftX -= 1;  // 左侧
                rightX += 1; // 右侧
                break;
            case Direction::Down:
                leftX += 1;  // 左侧（相对于向下移动的蛇）
                rightX -= 1; // 右侧
                break;
            case Direction::Left:
                leftY += 1;  // 左侧（相对于向左移动的蛇）
                rightY -= 1; // 右侧
                break;
            case Direction::Right:
                leftY -= 1;  // 左侧（相对于向右移动的蛇）
                rightY += 1; // 右侧
                break;
        }
        
        bool leftWall = mPtrMap->isWall(leftX, leftY);
        bool rightWall = mPtrMap->isWall(rightX, rightY);
        
        // 根据左右墙壁情况决定转向方向
        if (!leftWall && rightWall)
        {
            // 左侧无墙，向左转
            switch (mDirection)
            {
                case Direction::Up:
                    mDirection = Direction::Left;
                    break;
                case Direction::Left:
                    mDirection = Direction::Down;
                    break;
                case Direction::Down:
                    mDirection = Direction::Right;
                    break;
                case Direction::Right:
                    mDirection = Direction::Up;
                    break;
            }
        }
        else if (leftWall && !rightWall)
        {
            // 右侧无墙，向右转
            switch (mDirection)
            {
                case Direction::Up:
                    mDirection = Direction::Right;
                    break;
                case Direction::Right:
                    mDirection = Direction::Down;
                    break;
                case Direction::Down:
                    mDirection = Direction::Left;
                    break;
                case Direction::Left:
                    mDirection = Direction::Up;
                    break;
            }
        }
        else
        {
            // 两侧都有墙或都没有墙，默认左转
            // 这种情况通常不应该发生在设计合理的迷宫中
            switch (mDirection)
            {
                case Direction::Up:
                    mDirection = Direction::Left;
                    break;
                case Direction::Left:
                    mDirection = Direction::Down;
                    break;
                case Direction::Down:
                    mDirection = Direction::Right;
                    break;
                case Direction::Right:
                    mDirection = Direction::Up;
                    break;
            }
        }
    }
}
bool Snake::touchPoison() const
{
    SnakeBody newHead = this->createNewHead();
    if (this->mPoison == newHead)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Snake::touchSpecialFood() const
{
    SnakeBody newHead = this->createNewHead();
    if (this->mSpecialFood == newHead)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Snake::touchRandomItem() const
{
    SnakeBody newHead = this->createNewHead();
    if (this->mRandomItem == newHead)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Snake::touchCorpseFood() const
{
    SnakeBody newHead = this->createNewHead();
    // 检查是否碰到任何尸体食物
    for (const auto& corpseFood : mCorpseFoods) {
        if (corpseFood == newHead) {
            return true;
        }
    }
    return false;
}

SnakeBody Snake::getEatenCorpseFood() const
{
    SnakeBody newHead = this->createNewHead();
    // 返回被吃掉的尸体食物位置
    for (const auto& corpseFood : mCorpseFoods) {
        if (corpseFood == newHead) {
            return corpseFood;
        }
    }
    return SnakeBody(-1, -1); // 返回无效位置
}

// 无敌模式相关函数实现
void Snake::setInvincible(bool invincible) {
    mInvincible = invincible;
}

bool Snake::isInvincible() const {
    return mInvincible;
}

// 生命值相关函数实现
void Snake::setLives(int lives) {
    mLives = lives;
    mIsAlive = (lives > 0);
}

int Snake::getLives() const {
    return mLives;
}

bool Snake::loseLife() {
    if (mLives > 0) {
        mLives--;
        if (mLives <= 0) {
            mIsAlive = false;
        }
        return mIsAlive; // 返回是否还有剩余生命
    }
    return false;
}

bool Snake::isAlive() const {
    return mIsAlive;
}

void Snake::undoMove()
{
    // 将蛇头移回上一帧的安全位置
    if (!mSnakeBody.empty()) {
        mSnakeBody[0] = mPreviousHead;
    }
}
