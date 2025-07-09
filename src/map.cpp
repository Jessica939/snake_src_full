#include <fstream>
#include "map.h"

Map::Map(int width, int height) : mWidth(width), mHeight(height)
{
    initializeEmptyMap();
}

Map::~Map()
{
}

void Map::initializeEmptyMap()
{
    mTiles.resize(mHeight);
    for (int y = 0; y < mHeight; y++)
    {
        mTiles[y].resize(mWidth, TileType::Empty);
        
        // Set borders as walls
        if (y == 0 || y == mHeight - 1)
        {
            for (int x = 0; x < mWidth; x++)
            {
                mTiles[y][x] = TileType::Wall;
            }
        }
        else
        {
            mTiles[y][0] = TileType::Wall;
            mTiles[y][mWidth - 1] = TileType::Wall;
        }
    }
}

void Map::loadDefaultMap()
{
    initializeEmptyMap();
    
    // Add some walls to make the game more interesting
    int centerX = mWidth / 2;
    int centerY = mHeight / 2;
    
    // Create a small obstacle near the center
    for (int i = -2; i <= 2; i++)
    {
        int x = centerX + i;
        int y = centerY - 5;
        if (x > 0 && x < mWidth - 1 && y > 0 && y < mHeight - 1)
        {
            mTiles[y][x] = TileType::Wall;
        }
    }
    
    // Create another obstacle
    for (int i = -2; i <= 2; i++)
    {
        int x = centerX + i;
        int y = centerY + 5;
        if (x > 0 && x < mWidth - 1 && y > 0 && y < mHeight - 1)
        {
            mTiles[y][x] = TileType::Wall;
        }
    }
}

bool Map::loadMapFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }
    
    int width, height;
    file >> width >> height;
    
    if (width <= 0 || height <= 0 || width > 1000 || height > 1000)
    {
        return false;
    }
    
    mWidth = width;
    mHeight = height;
    
    mTiles.resize(mHeight);
    for (int y = 0; y < mHeight; y++)
    {
        mTiles[y].resize(mWidth);
        for (int x = 0; x < mWidth; x++)
        {
            int tileValue;
            file >> tileValue;
            mTiles[y][x] = static_cast<TileType>(tileValue);
        }
    }
    
    file.close();
    return true;
}

bool Map::saveMapToFile(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        return false;
    }
    
    file << mWidth << " " << mHeight << std::endl;
    
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            file << static_cast<int>(mTiles[y][x]) << " ";
        }
        file << std::endl;
    }
    
    file.close();
    return true;
}

TileType Map::getTile(int x, int y) const
{
    if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
    {
        return TileType::Wall;
    }
    return mTiles[y][x];
}

void Map::setTile(int x, int y, TileType type)
{
    if (x >= 0 && y >= 0 && x < mWidth && y < mHeight)
    {
        mTiles[y][x] = type;
    }
}

int Map::getWidth() const
{
    return mWidth;
}

int Map::getHeight() const
{
    return mHeight;
}

bool Map::isWall(int x, int y) const
{
    if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
    {
        return true; // 边界外全部视为墙
    }
    return mTiles[y][x] == TileType::Wall;
}

std::vector<SnakeBody> Map::getEmptyPositions(const std::vector<SnakeBody>& snake) const
{
    std::vector<SnakeBody> emptyPositions;
    
    for (int y = 1; y < mHeight - 1; y++)
    {
        for (int x = 1; x < mWidth - 1; x++)
        {
            // 只考虑非墙壁位置
            if (mTiles[y][x] != TileType::Wall)
            {
                bool isSnakePart = false;
                
                // 检查是否是蛇身体的一部分
                for (const auto& part : snake)
                {
                    if (part.getX() == x && part.getY() == y)
                    {
                        isSnakePart = true;
                        break;
                    }
                }
                
                if (!isSnakePart)
                {
                    emptyPositions.push_back(SnakeBody(x, y));
                }
            }
        }
    }
    
    return emptyPositions;
} 

bool Map::canPlaceSnake(int startX, int startY, InitialDirection direction, int length) const
{
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
    
    // 检查每个蛇身体节点的位置
    for (int i = 0; i < length; i++)
    {
        int x = startX + i * dx;
        int y = startY + i * dy;
        
        // 检查位置是否在地图内且不是墙
        if (x < 0 || y < 0 || x >= mWidth || y >= mHeight || mTiles[y][x] == TileType::Wall)
        {
            return false;
        }
    }
    
    return true;
}

bool Map::hasEnoughForwardSpace(int x, int y, InitialDirection direction, int minSpace) const
{
    // 根据方向设置增量
    int dx = 0, dy = 0;
    
    // 注意：这里使用的方向是蛇初始化的方向，也是蛇头的移动方向
    switch (direction)
    {
        case InitialDirection::Up:
            dy = -1; // 向上移动，Y减小
            break;
        case InitialDirection::Down:
            dy = 1;  // 向下移动，Y增大
            break;
        case InitialDirection::Left:
            dx = -1; // 向左移动，X减小
            break;
        case InitialDirection::Right:
            dx = 1;  // 向右移动，X增大
            break;
    }
    
    // 检查前方是否有足够的空间
    int emptyCount = 0;
    int totalChecked = 0;
    bool foundObstacle = false;
    
    // 我们检查前方minSpace个格子，并计算空白格子的百分比
    for (int i = 1; i <= minSpace && !foundObstacle; i++)
    {
        int checkX = x + i * dx;
        int checkY = y + i * dy;
        totalChecked++;
        
        // 如果超出地图边界或碰到墙，则停止计数并标记找到障碍物
        if (checkX <= 0 || checkY <= 0 || checkX >= mWidth - 1 || checkY >= mHeight - 1 || 
            mTiles[checkY][checkX] == TileType::Wall)
        {
            foundObstacle = true;
        }
        else
        {
            emptyCount++;
        }
    }
    
    // 没有格子被检查时视为没有足够空间
    if (totalChecked == 0)
    {
        return false;
    }
    
    // 如果空格占比大于40%，或者至少有2个空格，则认为有足够空间
    return (emptyCount >= 2) || ((double)emptyCount / totalChecked >= 0.4);
}

std::vector<std::pair<SnakeBody, InitialDirection>> Map::getValidSnakePositions(int length, int minSpace) const
{
    std::vector<std::pair<SnakeBody, InitialDirection>> validPositions;
    
    // 遍历所有可能的起始位置
    for (int y = 1; y < mHeight - 1; y++)
    {
        for (int x = 1; x < mWidth - 1; x++)
        {
            // 跳过墙壁位置
            if (mTiles[y][x] == TileType::Wall)
            {
                continue;
            }
            
            // 检查每个方向
            for (int dir = 0; dir < 4; dir++)
            {
                InitialDirection direction = static_cast<InitialDirection>(dir);
                
                // 检查蛇是否可以放置
                if (canPlaceSnake(x, y, direction, length))
                {
                    // 检查前方是否有足够空间
                    if (hasEnoughForwardSpace(x, y, direction, minSpace))
                    {
                        validPositions.emplace_back(SnakeBody(x, y), direction);
                    }
                }
            }
        }
    }
    
    return validPositions;
} 