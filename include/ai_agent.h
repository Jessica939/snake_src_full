#ifndef AI_AGENT_H
#define AI_AGENT_H

#include "snake.h"
#include "map.h" 
#include <vector>
#include <queue>

// AI算法基类
class AIAgent {
public:
    virtual ~AIAgent() = default;
    
    // --- 核心修改：简化接口，直接传入两条蛇 ---
    virtual Direction getNextDirection(const Map& map,  Snake& self, Snake& opponent, const SnakeBody& foodPos) = 0;
};

// 实现了躲避障碍的智能AI
class SmartAIAgent : public AIAgent {
public:
    Direction getNextDirection(const Map& map, Snake& self, Snake& opponent, const SnakeBody& foodPos) override;

private:
    // 辅助函数的参数也相应简化
    bool isMoveSafe(const SnakeBody& pos, const Map& map, Snake& snake1, Snake& snake2) const;
    int calculateSpaceScore(const SnakeBody& pos, const Map& map, Snake& snake1, Snake& snake2) const;
};

#endif // AI_AGENT_H