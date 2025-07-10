#ifndef SNAKE_H
#define SNAKE_H

#include <vector>

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
    bool operator == (const SnakeBody& snakeBody) const;
private:
    int mX;
    int mY;
};


enum class FoodType { Normal, Special2, Special4, Poison };

struct ExtraFood {
    SnakeBody pos;
    FoodType type;
    int lifetime;  //特殊食物的存活时间
    bool active;

    ExtraFood() : active(false) {}
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
    // Check if the snake is on the coordinate
    // bool isSnakeOn(int x, int y);
    // Checking API for generating random food
    bool isPartOfSnake(int x, int y);
    void senseFood(SnakeBody food);
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
    void shrink();
    void grow(int num);

    SnakeBody getHead() const;
    void updateBoundary(int width, int height);

    int getLives () const;
    void setLives(int lives);
    void loseLife () ;
    bool isDead () const;

    void resetSnake() ;

    Direction getDirection() const;


private:
    const int mGameBoardWidth;
    const int mGameBoardHeight;
    int mEffectiveWidth; //有效边界的宽度，在边界缩小中有用
    int mEffectiveHeight;
    // Snake information
    const int mInitialSnakeLength;
    Direction mDirection;
    SnakeBody mFood;
    std::vector<SnakeBody> mSnake;
    int mGrowNum = 0;
    int mLives = 3;//初始生命值为3
};

#endif
