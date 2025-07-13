#include "ai.h"
#include "map.h"
#include <vector>
#include <queue>
#include <algorithm>

AI::AI(int gameBoardWidth, int gameBoardHeight)
    : mGameBoardWidth(gameBoardWidth), mGameBoardHeight(gameBoardHeight) {}

Direction AI::findNextMove(const Map& map, const Snake& playerSnake, const Snake& aiSnake, const SnakeBody& food) {
    int headX = aiSnake.getSnake().front().getX();
    int headY = aiSnake.getSnake().front().getY();
    int foodX = food.getX();
    int foodY = food.getY();

    struct Move { int dx, dy; Direction dir; };
    std::vector<Move> moves;

    // 先按食物方向排序
    if (headX < foodX) moves.push_back({1, 0, Direction::Right});
    if (headX > foodX) moves.push_back({-1, 0, Direction::Left});
    if (headY < foodY) moves.push_back({0, 1, Direction::Down});
    if (headY > foodY) moves.push_back({0, -1, Direction::Up});
    // 补全其他方向
    if (std::find_if(moves.begin(), moves.end(), [](const Move& m){return m.dir==Direction::Right;}) == moves.end())
        moves.push_back({1, 0, Direction::Right});
    if (std::find_if(moves.begin(), moves.end(), [](const Move& m){return m.dir==Direction::Left;}) == moves.end())
        moves.push_back({-1, 0, Direction::Left});
    if (std::find_if(moves.begin(), moves.end(), [](const Move& m){return m.dir==Direction::Down;}) == moves.end())
        moves.push_back({0, 1, Direction::Down});
    if (std::find_if(moves.begin(), moves.end(), [](const Move& m){return m.dir==Direction::Up;}) == moves.end())
        moves.push_back({0, -1, Direction::Up});

    // 判断某个格子是否安全（不会撞墙/撞蛇/撞对方）
    auto isSafe = [&](int x, int y) {
        if (map.isWall(x, y)) return false;
        if (playerSnake.isPartOfSnake(x, y)) return false;
        if (aiSnake.isPartOfSnake(x, y)) return false;
        return true;
    };
    // 判断下一步后还有没有出路（简单一格预判）
    auto hasNextMove = [&](int x, int y) {
        int dxs[4] = {1, -1, 0, 0};
        int dys[4] = {0, 0, 1, -1};
        for (int i = 0; i < 4; ++i) {
            int nx = x + dxs[i];
            int ny = y + dys[i];
            if (isSafe(nx, ny)) return true;
        }
        return false;
    };

    // 优先选择安全且下一步还有出路的方向
    for (const auto& move : moves) {
        int nx = headX + move.dx;
        int ny = headY + move.dy;
        if (isSafe(nx, ny) && hasNextMove(nx, ny)) {
            return move.dir;
        }
    }
    // 如果没有，退而求其次，选择安全但不保证下一步有出路的方向
    for (const auto& move : moves) {
        int nx = headX + move.dx;
        int ny = headY + move.dy;
        if (isSafe(nx, ny)) {
            return move.dir;
        }
    }
    // 实在不行就保持原方向
    return aiSnake.getDirection();
}

// ai.cpp

std::vector<Point> AI::findPath(const Map& map, const Snake& playerSnake, const Snake& aiSnake, const Point& start, const Point& end) {
    std::vector<std::vector<Node>> grid(mGameBoardHeight, std::vector<Node>(mGameBoardWidth));
    std::vector<std::vector<bool>> visited(mGameBoardHeight, std::vector<bool>(mGameBoardWidth, false));
    std::queue<Point> q;

    // --- 修正后的障碍物标记逻辑 ---

    // 1. 先标记所有的墙和对方的蛇
    for (int y = 0; y < mGameBoardHeight; ++y) {
        for (int x = 0; x < mGameBoardWidth; ++x) {
            if (map.isWall(x, y) || playerSnake.isPartOfSnake(x, y)) {
                visited[y][x] = true;
            }
        }
    }

    // 2. 单独标记AI自己的蛇，但要排除作为终点的蛇尾
    const auto& aiSnakeBody = aiSnake.getSnake();
    for (const auto& part : aiSnakeBody) {
        // 如果当前身体部分不是寻路终点，就将其标记为障碍
        if (part.getX() != end.x || part.getY() != end.y) {
            visited[part.getY()][part.getX()] = true;
        }
    }
    
    // 3. 确保起点（蛇头）一定是可通行的
    visited[start.y][start.x] = false;

    // --- BFS 寻路算法（保持不变）---
    
    q.push(start);
    grid[start.y][start.x] = {start, {-1, -1}};

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
            // 修改前：if (next.x > 0 && next.x < mGameBoardWidth - 1 && next.y > 0 && next.y < mGameBoardHeight - 1 && !visited[next.y][next.x]) {
            // 修改后：允许走到边界
            if (next.x >= 0 && next.x < mGameBoardWidth && next.y >= 0 && next.y < mGameBoardHeight && !visited[next.y][next.x]) {
                visited[next.y][next.x] = true;
                q.push(next);
                grid[next.y][next.x] = {next, current};
            }
        }
    }

    // 重建路径
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
