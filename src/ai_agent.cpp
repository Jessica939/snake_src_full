#include "ai_agent.h"
#include "map.h"
#include <map>
#include <cmath>

// isMoveSafe 现在直接接收两条蛇
bool SmartAIAgent::isMoveSafe(const SnakeBody& pos, const Map& map, Snake& snake1, Snake& snake2) const
{
    if (map.isWall(pos.getX(), pos.getY())) return false;
    if (snake1.isPartOfSnake(pos.getX(), pos.getY())) return false;
    if (snake2.isPartOfSnake(pos.getX(), pos.getY())) return false;
    return true;
}

// calculateSpaceScore 也使用新的 isMoveSafe
int SmartAIAgent::calculateSpaceScore(const SnakeBody& pos, const Map& map, Snake& snake1, Snake& snake2) const
{
    std::queue<SnakeBody> q;
    q.push(pos);
    std::map<int, bool> visited;
    visited[pos.getY() * map.getWidth() + pos.getX()] = true;
    int reachable_count = 0;
    int search_limit = 150;

    while(!q.empty() && reachable_count < search_limit) {
        reachable_count++;
        SnakeBody current = q.front();
        q.pop();

        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};

        for(int i=0; i<4; ++i) {
            SnakeBody next(current.getX() + dx[i], current.getY() + dy[i]);
            if(isMoveSafe(next, map, snake1, snake2) && visited.find(next.getY() * map.getWidth() + next.getX()) == visited.end()) {
                visited[next.getY() * map.getWidth() + next.getX()] = true;
                q.push(next);
            }
        }
    }
    return reachable_count;
}

// getNextDirection 的实现
Direction SmartAIAgent::getNextDirection(const Map& map, Snake& self, Snake& opponent,const SnakeBody& foodPos)
{
    SnakeBody head = self.getHead();
    Direction currentDir = self.getDirection();
    
    std::vector<Direction> possibleMoves = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    possibleMoves.erase(std::remove(possibleMoves.begin(), possibleMoves.end(), self.getOppositeDirection()), possibleMoves.end());

    std::map<Direction, int> moveScores;
    for (Direction moveDir : possibleMoves) {
        SnakeBody nextPos = self.getNextHeadPosition(moveDir);
        
        if (!isMoveSafe(nextPos, map, self, opponent)) {
            moveScores[moveDir] = -10000;
            continue;
        }

        int distanceToFood = std::abs(nextPos.getX() - foodPos.getX()) + std::abs(nextPos.getY() - foodPos.getY());
        int foodScore = (map.getWidth() - distanceToFood) * 10;
        int spaceScore = calculateSpaceScore(nextPos, map, self, opponent);

        moveScores[moveDir] = foodScore + spaceScore;
    }
    
    if (moveScores.empty()) {
        return currentDir;
    }

    Direction bestMove = currentDir;
    int maxScore = -99999;
    for (const auto& pair : moveScores) {
        if (pair.second > maxScore) {
            maxScore = pair.second;
            bestMove = pair.first;
        }
    }

    return bestMove;
}