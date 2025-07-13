#ifndef AI_H
#define AI_H

#include "snake.h"
#include "map.h"
#include "food_type.h"
#include <vector>

class AI {
public:
    AI(int gameBoardWidth, int gameBoardHeight);

    // 简化的AI决策函数
    Direction findNextMove(const Map& map, const Snake& playerSnake, const Snake& aiSnake, 
                          const SnakeBody& normalFood, 
                          const SnakeBody& specialFood = SnakeBody(-1, -1),
                          const SnakeBody& poison = SnakeBody(-1, -1),
                          FoodType specialFoodType = FoodType::Normal,
                          bool hasSpecialFood = false,
                          bool hasPoison = false);

private:
    // 计算曼哈顿距离
    int calculateDistance(int x1, int y1, int x2, int y2) const;
    
    // 检查位置是否安全
    bool isSafePosition(const Map& map, const Snake& playerSnake, const Snake& aiSnake, int x, int y) const;
    
    // 检查移动方向是否安全
    bool isDirectionSafe(const Map& map, const Snake& playerSnake, const Snake& aiSnake, 
                        int headX, int headY, Direction dir) const;
    
    // 选择最佳移动方向
    Direction chooseBestDirection(const Map& map, const Snake& playerSnake, const Snake& aiSnake,
                                 int headX, int headY, int targetX, int targetY) const;
    
    // 避开毒药
    Direction avoidPoison(const Map& map, const Snake& playerSnake, const Snake& aiSnake, 
                         int headX, int headY, int poisonX, int poisonY) const;
    
    // 路径是否死路
    bool isDeadEnd(const Map& map, const Snake& playerSnake, const Snake& aiSnake, int x, int y) const;
    
    // 评估食物目标的综合分数
    float evaluateTargetScore(const Map& map, const Snake& playerSnake, const Snake& aiSnake, int headX, int headY, int targetX, int targetY, int foodValue, bool isPoison) const;
    
    // 计算玩家蛇的惩罚分数
    float calculatePlayerSnakePenalty(const Snake& playerSnake, int targetX, int targetY) const;
    
    // 检查移动后是否会撞到自己（考虑蛇身移动）
    bool willHitSelfAfterMove(const Snake& aiSnake, Direction dir) const;
    
    // 寻找安全的路径到目标
    Direction findSafePathToTarget(const Map& map, const Snake& playerSnake, const Snake& aiSnake,
                                  int headX, int headY, int targetX, int targetY) const;
    
    // 检查是否在食物附近打转
    bool isSpinningNearFood(const Snake& aiSnake, int targetX, int targetY) const;

    int mGameBoardWidth;
    int mGameBoardHeight;
};

#endif // AI_H
