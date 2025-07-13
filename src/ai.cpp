#include "ai.h"
#include <algorithm>
#include <cmath>

AI::AI(int gameBoardWidth, int gameBoardHeight)
    : mGameBoardWidth(gameBoardWidth), mGameBoardHeight(gameBoardHeight) {}

Direction AI::findNextMove(const Map& map, const Snake& playerSnake, const Snake& aiSnake, 
                          const SnakeBody& normalFood, 
                          const SnakeBody& specialFood,
                          const SnakeBody& poison,
                          FoodType specialFoodType,
                          bool hasSpecialFood,
                          bool hasPoison) {
    int headX = aiSnake.getSnake().front().getX();
    int headY = aiSnake.getSnake().front().getY();

    struct Target {
        int x, y, value;
        bool isPoison;
    };
    std::vector<Target> targets;
    // 特殊食物优先
    if (hasSpecialFood && specialFood.getX() >= 0 && specialFood.getY() >= 0) {
        int val = 1;
        switch (specialFoodType) {
            case FoodType::Special1: val = 2; break;
            case FoodType::Special2: val = 3; break;
            case FoodType::Special3: val = 5; break;
            default: val = 1; break;
        }
        targets.push_back({specialFood.getX(), specialFood.getY(), val, false});
    }
    // 普通食物
    if (normalFood.getX() >= 0 && normalFood.getY() >= 0) {
        targets.push_back({normalFood.getX(), normalFood.getY(), 1, false});
    }
    // 毒药
    if (hasPoison && poison.getX() >= 0 && poison.getY() >= 0) {
        targets.push_back({poison.getX(), poison.getY(), -1, true});
    }

    // 评估所有目标，选择分数最高且路径安全的
    float bestScore = -1e9;
    Target bestTarget = { -1, -1, 0, false };
    for (const auto& t : targets) {
        float score = evaluateTargetScore(map, playerSnake, aiSnake, headX, headY, t.x, t.y, t.value, t.isPoison);
        if (score > bestScore) {
            bestScore = score;
            bestTarget = t;
        }
    }
    // 如果没有合适目标或分数都很低，优先保命
    if (bestScore < 0.1f) {
        // 找一个安全方向活下去
        std::vector<Direction> dirs = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
        for (Direction dir : dirs) {
            if (isDirectionSafe(map, playerSnake, aiSnake, headX, headY, dir)) {
                int nx = headX, ny = headY;
                switch (dir) {
                    case Direction::Up: ny--; break;
                    case Direction::Down: ny++; break;
                    case Direction::Left: nx--; break;
                    case Direction::Right: nx++; break;
                }
                if (!isDeadEnd(map, playerSnake, aiSnake, nx, ny)) {
                    return dir;
                }
            }
        }
        // 实在不行随便选个安全方向
        for (Direction dir : dirs) {
            if (isDirectionSafe(map, playerSnake, aiSnake, headX, headY, dir)) {
                return dir;
            }
        }
        return aiSnake.getDirection();
    }
    // 有合适目标，走向目标
    return chooseBestDirection(map, playerSnake, aiSnake, headX, headY, bestTarget.x, bestTarget.y);
}

float AI::evaluateTargetScore(const Map& map, const Snake& playerSnake, const Snake& aiSnake, int headX, int headY, int targetX, int targetY, int foodValue, bool isPoison) const {
    int dist = calculateDistance(headX, headY, targetX, targetY);
    if (dist == 0) return -1e9f; // 不能吃自己
    if (dist > 30) return -100.0f; // 太远不考虑
    
    // 路径死路惩罚
    if (isDeadEnd(map, playerSnake, aiSnake, targetX, targetY)) return -1000.0f;
    
    // 靠近毒药惩罚
    float poisonPenalty = 0.0f;
    if (isPoison) poisonPenalty = 100.0f;
    
    // 靠近墙/蛇身惩罚
    float wallPenalty = 0.0f;
    if (!isSafePosition(map, playerSnake, aiSnake, targetX, targetY)) wallPenalty = 100.0f;
    
    // 新增：玩家蛇躲避惩罚
    float playerSnakePenalty = calculatePlayerSnakePenalty(playerSnake, targetX, targetY);
    
    // 综合分数
    float score = (float)foodValue / (dist + 1) - poisonPenalty - wallPenalty - playerSnakePenalty;
    return score;
}

float AI::calculatePlayerSnakePenalty(const Snake& playerSnake, int targetX, int targetY) const {
    const auto& playerBody = playerSnake.getSnake();
    if (playerBody.empty()) return 0.0f;
    
    float penalty = 0.0f;
    
    // 检查目标位置是否与玩家蛇身重叠
    for (const auto& part : playerBody) {
        int dist = calculateDistance(targetX, targetY, part.getX(), part.getY());
        if (dist == 0) {
            penalty += 500.0f; // 直接重叠，严重惩罚
        } else if (dist <= 2) {
            penalty += 50.0f / dist; // 距离越近惩罚越重
        }
    }
    
    // 检查玩家蛇头位置，给予更高惩罚
    const auto& playerHead = playerBody[0];
    int headDist = calculateDistance(targetX, targetY, playerHead.getX(), playerHead.getY());
    if (headDist <= 3) {
        penalty += 100.0f / (headDist + 1); // 玩家蛇头附近惩罚更重
    }
    
    return penalty;
}

bool AI::isDeadEnd(const Map& map, const Snake& playerSnake, const Snake& aiSnake, int x, int y) const {
    // 简单判定：周围安全格子数<=1视为死路
    int safeCount = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (std::abs(dx) + std::abs(dy) != 1) continue;
            int nx = x + dx, ny = y + dy;
            if (isSafePosition(map, playerSnake, aiSnake, nx, ny)) safeCount++;
        }
    }
    return safeCount <= 1;
}

Direction AI::avoidPoison(const Map& map, const Snake& playerSnake, const Snake& aiSnake, 
                         int headX, int headY, int poisonX, int poisonY) const {
    
    // 计算到毒药的距离
    int poisonDist = calculateDistance(headX, headY, poisonX, poisonY);
    
    // 如果毒药很远，可以忽略
    if (poisonDist > 3) {
        return aiSnake.getDirection();
    }
    
    // 尝试远离毒药的方向
    std::vector<Direction> directions = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    
    // 按远离毒药的程度排序
    std::sort(directions.begin(), directions.end(), [&](Direction a, Direction b) {
        int newX1 = headX, newY1 = headY, newX2 = headX, newY2 = headY;
        
        switch (a) {
            case Direction::Up: newY1--; break;
            case Direction::Down: newY1++; break;
            case Direction::Left: newX1--; break;
            case Direction::Right: newX1++; break;
        }
        
        switch (b) {
            case Direction::Up: newY2--; break;
            case Direction::Down: newY2++; break;
            case Direction::Left: newX2--; break;
            case Direction::Right: newX2++; break;
        }
        
        int dist1 = calculateDistance(newX1, newY1, poisonX, poisonY);
        int dist2 = calculateDistance(newX2, newY2, poisonX, poisonY);
        
        return dist1 > dist2; // 距离越远越好
    });
    
    // 选择安全且远离毒药的方向
    for (Direction dir : directions) {
        if (isDirectionSafe(map, playerSnake, aiSnake, headX, headY, dir)) {
            return dir;
        }
    }
    
    // 如果无法避开，保持当前方向
    return aiSnake.getDirection();
}

Direction AI::chooseBestDirection(const Map& map, const Snake& playerSnake, const Snake& aiSnake,
                                 int headX, int headY, int targetX, int targetY) const {
    
    // 检查是否在食物附近打转
    if (isSpinningNearFood(aiSnake, targetX, targetY)) {
        // 如果正在打转，选择一个不同的方向
        return findSafePathToTarget(map, playerSnake, aiSnake, headX, headY, targetX, targetY);
    }
    
    // 尝试直接路径
    int distX = targetX - headX;
    int distY = targetY - headY;
    
    // 优先选择水平或垂直方向中距离较大的
    std::vector<Direction> preferredDirections;
    
    if (std::abs(distX) > std::abs(distY)) {
        // 水平距离更大，优先水平移动
        if (distX > 0) {
            preferredDirections = {Direction::Right, Direction::Up, Direction::Down, Direction::Left};
        } else {
            preferredDirections = {Direction::Left, Direction::Up, Direction::Down, Direction::Right};
        }
    } else {
        // 垂直距离更大，优先垂直移动
        if (distY > 0) {
            preferredDirections = {Direction::Down, Direction::Right, Direction::Left, Direction::Up};
        } else {
            preferredDirections = {Direction::Up, Direction::Right, Direction::Left, Direction::Down};
        }
    }
    
    // 选择第一个安全且不会撞到自己的方向
    for (Direction dir : preferredDirections) {
        if (isDirectionSafe(map, playerSnake, aiSnake, headX, headY, dir) && 
            !willHitSelfAfterMove(aiSnake, dir)) {
            return dir;
        }
    }
    
    // 如果直接路径不安全，寻找安全路径
    return findSafePathToTarget(map, playerSnake, aiSnake, headX, headY, targetX, targetY);
}

Direction AI::findSafePathToTarget(const Map& map, const Snake& playerSnake, const Snake& aiSnake,
                                  int headX, int headY, int targetX, int targetY) const {
    
    // 简单的A*寻路算法
    std::vector<std::vector<bool>> visited(mGameBoardHeight, std::vector<bool>(mGameBoardWidth, false));
    std::vector<std::vector<std::pair<int, int>>> parent(mGameBoardHeight, std::vector<std::pair<int, int>>(mGameBoardWidth, {-1, -1}));
    
    // 标记障碍物（包括玩家蛇）
    for (int y = 0; y < mGameBoardHeight; ++y) {
        for (int x = 0; x < mGameBoardWidth; ++x) {
            if (map.isWall(x, y) || playerSnake.isPartOfSnake(x, y) || aiSnake.isPartOfSnake(x, y)) {
                visited[y][x] = true;
            }
        }
    }
    
    // 额外标记玩家蛇头附近的区域为危险区域
    const auto& playerBody = playerSnake.getSnake();
    if (!playerBody.empty()) {
        int playerHeadX = playerBody[0].getX();
        int playerHeadY = playerBody[0].getY();
        
        // 标记玩家蛇头周围2格为危险区域
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                int nx = playerHeadX + dx;
                int ny = playerHeadY + dy;
                if (nx >= 0 && nx < mGameBoardWidth && ny >= 0 && ny < mGameBoardHeight) {
                    // 不直接标记为障碍，但增加寻路成本
                    if (std::abs(dx) + std::abs(dy) <= 1) {
                        visited[ny][nx] = true; // 紧邻玩家蛇头的位置标记为障碍
                    }
                }
            }
        }
    }
    
    // BFS寻路
    std::vector<std::pair<int, int>> queue = {{headX, headY}};
    visited[headY][headX] = true;
    
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    Direction dirs[] = {Direction::Down, Direction::Up, Direction::Right, Direction::Left};
    
    for (size_t i = 0; i < queue.size(); ++i) {
        int x = queue[i].first;
        int y = queue[i].second;
        
        if (x == targetX && y == targetY) {
            // 找到目标，回溯路径
            std::vector<std::pair<int, int>> path;
            while (x != headX || y != headY) {
                path.push_back({x, y});
                auto p = parent[y][x];
                x = p.first;
                y = p.second;
            }
            
            if (!path.empty()) {
                // 返回第一步的方向
                int firstX = path.back().first;
                int firstY = path.back().second;
                int stepX = firstX - headX;
                int stepY = firstY - headY;
                
                if (stepX == 1) return Direction::Right;
                if (stepX == -1) return Direction::Left;
                if (stepY == 1) return Direction::Down;
                if (stepY == -1) return Direction::Up;
            }
            break;
        }
        
        for (int j = 0; j < 4; ++j) {
            int nx = x + dx[j];
            int ny = y + dy[j];
            
            if (nx >= 0 && nx < mGameBoardWidth && ny >= 0 && ny < mGameBoardHeight && 
                !visited[ny][nx]) {
                visited[ny][nx] = true;
                parent[ny][nx] = {x, y};
                queue.push_back({nx, ny});
            }
        }
    }
    
    // 如果找不到路径，选择一个安全方向
    std::vector<Direction> safeDirs = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    for (Direction dir : safeDirs) {
        if (isDirectionSafe(map, playerSnake, aiSnake, headX, headY, dir) && 
            !willHitSelfAfterMove(aiSnake, dir)) {
            return dir;
        }
    }
    
    // 实在不行保持当前方向
    return aiSnake.getDirection();
}

bool AI::willHitSelfAfterMove(const Snake& aiSnake, Direction dir) const {
    const auto& snakeBody = aiSnake.getSnake();
    if (snakeBody.empty()) return false;
    
    int headX = snakeBody[0].getX();
    int headY = snakeBody[0].getY();
    
    // 计算移动后的位置
    int newX = headX, newY = headY;
    switch (dir) {
        case Direction::Up: newY--; break;
        case Direction::Down: newY++; break;
        case Direction::Left: newX--; break;
        case Direction::Right: newX++; break;
    }
    
    // 检查是否会撞到自己的身体（除了尾部，因为尾部会移动）
    for (size_t i = 0; i < snakeBody.size() - 1; ++i) {
        if (snakeBody[i].getX() == newX && snakeBody[i].getY() == newY) {
            return true;
        }
    }
    
    return false;
}

bool AI::isSpinningNearFood(const Snake& aiSnake, int targetX, int targetY) const {
    const auto& snakeBody = aiSnake.getSnake();
    if (snakeBody.size() < 4) return false;
    
    // 检查最近几步是否在食物附近打转
    int foodDist = calculateDistance(snakeBody[0].getX(), snakeBody[0].getY(), targetX, targetY);
    if (foodDist > 3) return false; // 不在食物附近
    
    // 检查最近几步的移动模式
    std::vector<std::pair<int, int>> recentMoves;
    for (size_t i = 0; i < std::min(snakeBody.size() - 1, size_t(4)); ++i) {
        int dx = snakeBody[i].getX() - snakeBody[i + 1].getX();
        int dy = snakeBody[i].getY() - snakeBody[i + 1].getY();
        recentMoves.push_back({dx, dy});
    }
    
    // 如果最近几步的移动模式重复，说明在打转
    if (recentMoves.size() >= 4) {
        // 检查是否有重复的移动模式
        for (size_t i = 0; i < recentMoves.size() - 2; ++i) {
            for (size_t j = i + 2; j < recentMoves.size(); ++j) {
                if (recentMoves[i] == recentMoves[j]) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

int AI::calculateDistance(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2); // 曼哈顿距离
}

bool AI::isSafePosition(const Map& map, const Snake& playerSnake, const Snake& aiSnake, int x, int y) const {
    // 检查边界
    if (x < 0 || x >= mGameBoardWidth || y < 0 || y >= mGameBoardHeight) {
        return false;
    }
    
    // 检查墙壁
    if (map.isWall(x, y)) {
        return false;
    }
    
    // 检查是否与玩家蛇身重叠
    if (playerSnake.isPartOfSnake(x, y)) {
        return false;
    }
    
    // 检查是否与AI蛇身重叠
    if (aiSnake.isPartOfSnake(x, y)) {
        return false;
    }
    
    return true;
}

bool AI::isDirectionSafe(const Map& map, const Snake& playerSnake, const Snake& aiSnake, 
                        int headX, int headY, Direction dir) const {
    
    int newX = headX, newY = headY;
    
    switch (dir) {
        case Direction::Up: newY--; break;
        case Direction::Down: newY++; break;
        case Direction::Left: newX--; break;
        case Direction::Right: newX++; break;
    }
    
    return isSafePosition(map, playerSnake, aiSnake, newX, newY);
}
