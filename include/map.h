#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>
#include "snake.h"

enum class TileType
{
    Empty = 0,
    Wall = 1,
    Food = 2
};

// InitialDirection 已在 snake.h 中定义，这里不再重复定义

class Map
{
public:
    Map(int width, int height);
    ~Map();

    void loadDefaultMap();
    bool loadMapFromFile(const std::string& filename);
    bool saveMapToFile(const std::string& filename);
    
    TileType getTile(int x, int y) const;
    void setTile(int x, int y, TileType type);
    
    int getWidth() const;
    int getHeight() const;
    
    // Check if a position is a wall
    bool isWall(int x, int y) const;
    
    // Get all empty positions where food can be placed
    std::vector<SnakeBody> getEmptyPositions(const std::vector<SnakeBody>& snake) const;
    
    // Check if a snake can be placed at a specific position with a given direction and length
    bool canPlaceSnake(int startX, int startY, InitialDirection direction, int length) const;
    
    // Check if there is enough forward space based on direction
    bool hasEnoughForwardSpace(int x, int y, InitialDirection direction, int minSpace = 5) const;
    
    // Get valid positions where a snake of given length can be placed with enough forward space
    std::vector<std::pair<SnakeBody, InitialDirection>> getValidSnakePositions(int length, int minSpace = 5) const;

private:
    int mWidth;
    int mHeight;
    std::vector<std::vector<TileType>> mTiles;
    
    void initializeEmptyMap();
};

#endif 