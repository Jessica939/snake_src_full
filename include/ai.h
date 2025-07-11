#ifndef AI_H
#define AI_H

#include "snake.h"
#include <vector>
#include <queue>

class Map;

// A simple coordinate structure for pathfinding
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// Used for BFS path reconstruction
struct Node {
    Point pos;
    Point parent;
};

class AI {
public:
    AI(int gameBoardWidth, int gameBoardHeight);

    // The main function to determine the AI's next move
    Direction findNextMove(const Map& map, const Snake& playerSnake, const Snake& aiSnake, const SnakeBody& food);

private:
    // Pathfinding algorithm using Breadth-First Search
    std::vector<Point> findPath(const Map& map, const Snake& playerSnake, const Snake& aiSnake, const Point& start, const Point& end);

    int mGameBoardWidth;
    int mGameBoardHeight;
};

#endif // AI_H