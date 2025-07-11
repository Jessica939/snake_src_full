#include "ai.h"
#include "map.h"
#include <vector>
#include <queue>
#include <algorithm>

AI::AI(int gameBoardWidth, int gameBoardHeight)
    : mGameBoardWidth(gameBoardWidth), mGameBoardHeight(gameBoardHeight) {}

Direction AI::findNextMove(const Map& map, const Snake& playerSnake, const Snake& aiSnake, const SnakeBody& food) {
    Point head = {aiSnake.getSnake().front().getX(), aiSnake.getSnake().front().getY()};
    Point foodPos = {food.getX(), food.getY()};
    Point tail = {aiSnake.getSnake().back().getX(), aiSnake.getSnake().back().getY()};

    // 1. Find a path to the food
    std::vector<Point> pathToFood = findPath(map, playerSnake, aiSnake, head, foodPos);

    if (!pathToFood.empty()) {
        // 2. Safety check: After eating the food, can the snake still reach its own tail?
        // This prevents the AI from trapping itself.
        Snake futureAiSnake = aiSnake;
        futureAiSnake.getSnake().insert(futureAiSnake.getSnake().begin(), food); // Simulate eating

        Point futureHead = {futureAiSnake.getSnake().front().getX(), futureAiSnake.getSnake().front().getY()};
        Point futureTail = {futureAiSnake.getSnake().back().getX(), futureAiSnake.getSnake().back().getY()};
        
        std::vector<Point> pathToTail = findPath(map, playerSnake, futureAiSnake, futureHead, futureTail);

        if (!pathToTail.empty()) {
            // It's safe to go for the food. Determine the direction.
            Point nextStep = pathToFood[1]; // pathToFood[0] is the current head
            if (nextStep.x > head.x) return Direction::Right;
            if (nextStep.x < head.x) return Direction::Left;
            if (nextStep.y > head.y) return Direction::Down;
            if (nextStep.y < head.y) return Direction::Up;
        }
    }

    // 3. Survival mode: If no safe path to food, find the longest path available (by chasing its tail)
    std::vector<Point> pathToSelf = findPath(map, playerSnake, aiSnake, head, tail);
    if (!pathToSelf.empty() && pathToSelf.size() > 1) {
        Point nextStep = pathToSelf[1];
        if (nextStep.x > head.x) return Direction::Right;
        if (nextStep.x < head.x) return Direction::Left;
        if (nextStep.y > head.y) return Direction::Down;
        if (nextStep.y < head.y) return Direction::Up;
    }

    // 4. Last resort: if all else fails, just move to an adjacent free space
    Point next_move = head;
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    Direction dirs[] = {Direction::Down, Direction::Up, Direction::Right, Direction::Left};

    for(int i = 0; i < 4; ++i) {
        Point p = {head.x + dx[i], head.y + dy[i]};
        if (!map.isWall(p.x, p.y) && !playerSnake.isPartOfSnake(p.x, p.y) && !aiSnake.isPartOfSnake(p.x, p.y)) {
             return dirs[i];
        }
    }

    return aiSnake.getDirection(); // Keep going straight if no other option
}

std::vector<Point> AI::findPath(const Map& map, const Snake& playerSnake, const Snake& aiSnake, const Point& start, const Point& end) {
    std::vector<std::vector<Node>> grid(mGameBoardHeight, std::vector<Node>(mGameBoardWidth));
    std::vector<std::vector<bool>> visited(mGameBoardHeight, std::vector<bool>(mGameBoardWidth, false));
    std::queue<Point> q;

    q.push(start);
    visited[start.y][start.x] = true;
    grid[start.y][start.x] = {start, {-1, -1}};

    // Mark obstacles
    for (int y = 0; y < mGameBoardHeight; ++y) {
        for (int x = 0; x < mGameBoardWidth; ++x) {
            if (map.isWall(x, y) || playerSnake.isPartOfSnake(x, y) || aiSnake.isPartOfSnake(x, y)) {
                visited[y][x] = true;
            }
        }
    }
     // The snake's head is a valid starting point, so un-visit it.
    visited[start.y][start.x] = false; 


    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    bool found = false;
    while (!q.empty()) {
        Point current = q.front();
        q.pop();

        if (current == end) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            Point next = {current.x + dx[i], current.y + dy[i]};
            if (next.x > 0 && next.x < mGameBoardWidth - 1 && next.y > 0 && next.y < mGameBoardHeight - 1 && !visited[next.y][next.x]) {
                visited[next.y][next.x] = true;
                q.push(next);
                grid[next.y][next.x] = {next, current};
            }
        }
    }

    std::vector<Point> path;
    if (found) {
        Point current = end;
        while (current.x != -1) {
            path.push_back(current);
            current = grid[current.y][current.x].parent;
        }
        std::reverse(path.begin(), path.end());
    }

    return path;
}