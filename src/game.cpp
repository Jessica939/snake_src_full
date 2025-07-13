#include <string>
#include <iostream>
#include <cmath>

// For terminal delay
#include <chrono>
#include <thread>

#include <fstream>
#include <algorithm>
#include <filesystem>

#include "game.h"
#include "map.h"
#include "ai.h"

Game::Game()
{
    // Separate the screen to three windows
    this->mWindows.resize(3);
    
    // 获取屏幕尺寸
    getmaxyx(stdscr, this->mScreenHeight, this->mScreenWidth);
    this->mGameBoardWidth = this->mScreenWidth - this->mInstructionWidth;
    this->mGameBoardHeight = this->mScreenHeight - this->mInformationHeight;

    this->createInformationBoard();
    this->createGameBoard();
    this->createInstructionBoard();

    // Initialize the leader board to be all zeros
    this->mLeaderBoard.assign(this->mNumLeaders, 0);
    
    // 初始化关卡状态列表
    this->mLevelStatus.assign(this->mMaxLevel, LevelStatus::Locked);
    // 第一关默认解锁
    this->mLevelStatus[0] = LevelStatus::Unlocked;
    // 第四五关也解锁，用于测试，待修改
    this->mLevelStatus[3] = LevelStatus::Unlocked;
    this->mLevelStatus[4] = LevelStatus::Unlocked;
    // 加载已保存的关卡进度
    this->loadLevelProgress();
    
    // Create maps directory if it doesn't exist
    std::filesystem::create_directory("maps");
    
    // 创建默认的关卡地图文件
    createDefaultLevelMaps();

    mPtrAI = std::make_unique<AI>(mGameBoardWidth, mGameBoardHeight);

    loadPlayerProfile();
    loadItemInventory();
}

// 创建默认的关卡地图文件（如果不存在）
void Game::createDefaultLevelMaps()
{
    // 确保每个关卡都有对应的地图文件
    for (int level = 1; level <= mMaxLevel; level++) {
        std::string mapFilePath = "maps/level" + std::to_string(level) + ".txt";
        
        // 检查文件是否存在
        std::ifstream checkFile(mapFilePath);
        if (!checkFile.good()) {
            // 如果文件不存在，则创建一个默认地图
            // 根据关卡特点创建不同的默认地图
            std::ofstream mapFile(mapFilePath);
            if (mapFile.is_open()) {
                int width = mGameBoardWidth;
                int height = mGameBoardHeight;
                
                // 写入地图尺寸
                mapFile << width << " " << height << std::endl;
                
                // 根据关卡类型创建不同的地图布局
                switch (level) {
                    case 1: // 第一关：简单边界
                        for (int y = 0; y < height; y++) {
                            for (int x = 0; x < width; x++) {
                                if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                                    mapFile << "1 ";
                                } else {
                                    mapFile << "0 ";
                                }
                            }
                            mapFile << std::endl;
                        }
                        break;
                    case 2: // 第二关：速度挑战，简单布局
                        for (int y = 0; y < height; y++) {
                            for (int x = 0; x < width; x++) {
                                if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                                    mapFile << "1 ";
                                } else {
                                    mapFile << "0 ";
                                }
                            }
                            mapFile << std::endl;
                        }
                        break;
                    case 3: // 第三关：迷宫布局
                        for (int y = 0; y < height; y++) {
                            for (int x = 0; x < width; x++) {
                                if (x == 0 || y == 0 || x == width - 1 || y == height - 1 ||
                                    (x % 10 == 5 && y % 8 != 0) ||
                                    (y % 8 == 4 && x % 10 != 0)) {
                                    mapFile << "1 ";
                                } else {
                                    mapFile << "0 ";
                                }
                            }
                            mapFile << std::endl;
                        }
                        break;
                    case 4: // 第四关：自定义关卡1
                        for (int y = 0; y < height; y++) {
                            for (int x = 0; x < width; x++) {
                                if (x == 0 || y == 0 || x == width - 1 || y == height - 1 ||
                                    (x == width/2 && y < height/2) ||
                                    (y == height/2 && x > width/2)) {
                                    mapFile << "1 ";
                                } else {
                                    mapFile << "0 ";
                                }
                            }
                            mapFile << std::endl;
                        }
                        break;
                    case 5: // 第五关：自定义关卡2
                        for (int y = 0; y < height; y++) {
                            for (int x = 0; x < width; x++) {
                                if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                                    mapFile << "1 ";
                                } else {
                                    mapFile << "0 ";
                                }
                            }
                            mapFile << std::endl;
                        }
                        break;
                    default:
                        // 默认情况：简单边界
                        for (int y = 0; y < height; y++) {
                            for (int x = 0; x < width; x++) {
                                if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                                    mapFile << "1 ";
                                } else {
                                    mapFile << "0 ";
                                }
                            }
                            mapFile << std::endl;
                        }
                }
                
                mapFile.close();
            }
        }
    }
}

Game::~Game()
{
    for (size_t i = 0; i < this->mWindows.size(); i ++)
    {
        delwin(this->mWindows[i]);
    }

    savePlayerProfile();
    saveItemInventory();
}

void Game::createInformationBoard()
{
    int startY = 0;
    int startX = 0;
    this->mWindows[0] = newwin(this->mInformationHeight, this->mScreenWidth, startY, startX);
}

void Game::renderInformationBoard() const
{
    mvwprintw(this->mWindows[0], 1, 1, "Welcome to The Snake Game!");
    mvwprintw(this->mWindows[0], 2, 1, "Author: Lei Mao");
    mvwprintw(this->mWindows[0], 3, 1, "Website: https://github.com/leimao/");
    mvwprintw(this->mWindows[0], 4, 1, "Implemented using C++ and libncurses library.");
    
    // 在经典模式中显示生命数
    if (mCurrentMode == GameMode::Classic && mPtrSnake != nullptr) {
        mvwprintw(this->mWindows[0], 5, 1, "Lives: %d", mPtrSnake->getLives());
    }
    
    // 显示护盾激活状态
    if (isShieldActive()) {
        wattron(this->mWindows[0], COLOR_PAIR(4)); // 红色
        mvwprintw(this->mWindows[0], 2, 30, "[护盾保护中]");
        wattroff(this->mWindows[0], COLOR_PAIR(4));
    }
    
}

void Game::createGameBoard()
{
    int startY = this->mInformationHeight;
    int startX = 0;
    this->mWindows[1] = newwin(this->mScreenHeight - this->mInformationHeight, this->mScreenWidth - this->mInstructionWidth, startY, startX);
}

void Game::renderGameBoard() const
{
    wrefresh(this->mWindows[1]);
}

void Game::createInstructionBoard()
{
    int startY = this->mInformationHeight;
    int startX = this->mScreenWidth - this->mInstructionWidth;
    this->mWindows[2] = newwin(this->mScreenHeight - this->mInformationHeight, this->mInstructionWidth, startY, startX);
}

void Game::renderInstructionBoard() const
{
    // 清空侧边栏内容，防止残留
    werase(this->mWindows[2]);
    
    int row = 1;
    // Manual
    mvwprintw(this->mWindows[2], row++, 1, "Manual");
    mvwprintw(this->mWindows[2], row++, 2, "Up: W");
    mvwprintw(this->mWindows[2], row++, 2, "Down: S");
    mvwprintw(this->mWindows[2], row++, 2, "Left: A");
    mvwprintw(this->mWindows[2], row++, 2, "Right: D");
    mvwprintw(this->mWindows[2], row++, 2, "Save:  F");
    row++; // 空一行
    // Difficulty
    mvwprintw(this->mWindows[2], row++, 1, "Difficulty");
    mvwprintw(this->mWindows[2], row++, 2, "%d", mDifficulty);
    row++; // 空一行
    // Level
    mvwprintw(this->mWindows[2], row++, 1, "Level");
    mvwprintw(this->mWindows[2], row++, 2, "%d", mCurrentLevel);
    row++; // 空一行
    
    // --- 剩余时间 (仅限时模式) ---
    if (mCurrentMode == GameMode::Timed) {
        mvwprintw(this->mWindows[2], 14, 1, "Time Left:");
        mvwprintw(this->mWindows[2], 15, 2, "%d s", mTimeRemaining);
    }
    // Points
    mvwprintw(this->mWindows[2], row++, 1, "Points");
    mvwprintw(this->mWindows[2], row++, 2, "%d", mPoints);
    row++; // 空一行
    // Items
    mvwprintw(this->mWindows[2], row++, 1, "Items:");
    for (int i = 0; i <= (int)ItemType::Poison; ++i) {
        ItemType type = static_cast<ItemType>(i);
        int count = 0;
        auto it = mItemInventory.find(type);
        if (it != mItemInventory.end()) count = it->second;
        std::string itemName;
        switch (type) {
            case ItemType::Portal: itemName = "Portal"; break;
            case ItemType::RandomBox: itemName = "RandomBox"; break;
            case ItemType::Cheat: itemName = "Cheat"; break;
            case ItemType::Attack: itemName = "Attack"; break;
            case ItemType::Shield: itemName = "Shield"; break;
            case ItemType::Poison: itemName = "Poison"; break;
            default: itemName = "Unknown"; break;
        }
        mvwprintw(this->mWindows[2], row++, 2, "%s: %d", itemName.c_str(), count);
    }
    row++; // Items和排行榜之间再加空行
    // 最后一行显示保存提示
    wrefresh(this->mWindows[2]);
}


void Game::renderLeaderBoard() const
{
    int startRow = 28;
    mvwprintw(this->mWindows[2], startRow, 1, "Leader Board");
    std::string pointString;
    std::string rank;
    for (int i = 0; i < std::min(this->mNumLeaders, this->mScreenHeight - this->mInformationHeight - startRow - 2); i ++)
    {
        pointString = std::to_string(this->mLeaderBoard[i]);
        rank = "#" + std::to_string(i + 1) + ":";
        mvwprintw(this->mWindows[2], startRow + (i + 1), 1, "%s", rank.c_str());
        mvwprintw(this->mWindows[2], startRow + (i + 1), 5, "%s", pointString.c_str());
    }
    wrefresh(this->mWindows[2]);
}

bool Game::renderRestartMenu() const
{
    WINDOW * menu;
    int width = this->mGameBoardWidth * 0.5;
    int height = this->mGameBoardHeight * 0.5;
    int startX = this->mGameBoardWidth * 0.25;
    int startY = this->mGameBoardHeight * 0.25 + this->mInformationHeight;

    menu = newwin(height, width, startY, startX);
    box(menu, 0, 0);
    std::vector<std::string> menuItems = {"Restart", "Quit"};

    int index = 0;
    int offset = 4;
    mvwprintw(menu, 1, 1, "Your Final Score:");
    std::string pointString = std::to_string(this->mPoints);
    mvwprintw(menu, 2, 1, "%s", pointString.c_str());
    wattron(menu, A_STANDOUT);
    mvwprintw(menu, 0 + offset, 1, "%s", menuItems[0].c_str());
    wattroff(menu, A_STANDOUT);
    mvwprintw(menu, 1 + offset, 1, "%s", menuItems[1].c_str());

    wrefresh(menu);

    int key;
    while (true)
    {
        key = getch();
        switch(key)
        {
            case 'W':
            case 'w':
            case KEY_UP:
            {
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                index--;
                index = (index < 0) ? static_cast<int>(menuItems.size()) - 1 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            }
            case 'S':
            case 's':
            case KEY_DOWN:
            {
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                index++;
                index = (index > static_cast<int>(menuItems.size()) - 1) ? 0 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            }
        }
        wrefresh(menu);
        if (key == ' ' || key == 10)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    delwin(menu);

    if (index == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

bool Game::renderRestartMenu(bool isBattleMode) const
{
    WINDOW * menu;
    int width = this->mGameBoardWidth * 0.5;
    int height = this->mGameBoardHeight * 0.5;
    int startX = this->mGameBoardWidth * 0.25;
    int startY = this->mGameBoardHeight * 0.25 + this->mInformationHeight;

    menu = newwin(height, width, startY, startX);
    box(menu, 0, 0);

    // 只有两个选项
    std::vector<std::string> menuItems = {"Restart", "Quit"};

    int index = 0;
    int offset = 4;

    // 根据模式显示不同标题
    if (isBattleMode) {
        mvwprintw(menu, 1, 1, "Battle Over!");
        mvwprintw(menu, 2, 1, "Player1: %d | Player2: %d", mPoints, mPoints2);
    } else {
        mvwprintw(menu, 1, 1, "Your Final Score:");
        std::string pointString = std::to_string(this->mPoints);
        mvwprintw(menu, 2, 1, pointString.c_str());
    }

    wattron(menu, A_STANDOUT);
    mvwprintw(menu, 0 + offset, 1, menuItems[0].c_str());
    wattroff(menu, A_STANDOUT);
    mvwprintw(menu, 1 + offset, 1, menuItems[1].c_str());

    wrefresh(menu);

    int key;
    while (true)
    {
        key = getch();
        switch(key)
        {
            case 'W': case 'w': case KEY_UP:
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index = (index == 0) ? 1 : 0; // 在两个选项间切换
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            case 'S': case 's': case KEY_DOWN:
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index = (index == 0) ? 1 : 0; // 在两个选项间切换
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
        }
        wrefresh(menu);
        if (key == ' ' || key == 10) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    delwin(menu);

    if (index == 0) {
        return true; // Restart -> playAgain = true
    } else {
        return false; // Quit -> playAgain = false
    }
}

bool Game::selectMap()
{
    WINDOW * menu;
    int width = this->mGameBoardWidth * 0.6;
    int height = this->mGameBoardHeight * 0.6;
    int startX = this->mGameBoardWidth * 0.2;
    int startY = this->mGameBoardHeight * 0.2 + this->mInformationHeight;

    menu = newwin(height, width, startY, startX);
    box(menu, 0, 0);
    
    std::vector<std::string> menuItems;
    menuItems.push_back(mDefaultMapName);
    
    // Check which map files exist
    for (const auto& mapFile : mMapFiles) {
        std::ifstream file(mapFile);
        if (file.good()) {
            menuItems.push_back(mapFile);
        }
    }

    int index = 0;
    int offset = 3;
    mvwprintw(menu, 1, 1, "Select a Map:");
    
    wattron(menu, A_STANDOUT);
    mvwprintw(menu, 0 + offset, 1, "%s", menuItems[0].c_str());
    wattroff(menu, A_STANDOUT);
    
    for (size_t i = 1; i < menuItems.size(); i++) {
        mvwprintw(menu, static_cast<int>(i) + offset, 1, "%s", menuItems[i].c_str());
    }

    wrefresh(menu);

    int key;
    while (true)
    {
        key = getch();
        switch(key)
        {
            case 'W':
            case 'w':
            case KEY_UP:
            {
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                index--;
                index = (index < 0) ? static_cast<int>(menuItems.size()) - 1 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            }
            case 'S':
            case 's':
            case KEY_DOWN:
            {
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                index++;
                index = (index > static_cast<int>(menuItems.size()) - 1) ? 0 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            }
        }
        wrefresh(menu);
        if (key == ' ' || key == 10)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    delwin(menu);
    
    // 创建地图
    mPtrMap = std::make_unique<Map>(mGameBoardWidth, mGameBoardHeight);
    
    // 如果选择默认地图
    if (index == 0) {
        mPtrMap->loadDefaultMap();
    }
    // 否则加载指定的地图文件
    else {
        mPtrMap->loadMapFromFile(menuItems[index]);
    }
    
    return true;
}

void Game::renderPoints() const
{
    std::string pointString = std::to_string(this->mPoints);
    mvwprintw(this->mWindows[2], 15, 2, "%s", pointString.c_str());
    wrefresh(this->mWindows[2]);
}

void Game::renderDifficulty() const
{
    std::string difficultyString = std::to_string(this->mDifficulty);
    mvwprintw(this->mWindows[2], 9, 2, "%s", difficultyString.c_str());
    wrefresh(this->mWindows[2]);
}

void Game::renderLevel() const
{
    std::string levelString = std::to_string(this->mCurrentLevel);
    mvwprintw(this->mWindows[2], 12, 2, "%s", levelString.c_str());
    wrefresh(this->mWindows[2]);
}

void Game::initializeGame()
{
    // 先选择地图
    this->selectMap();
    
    // 然后创建蛇
    this->mPtrSnake.reset(new Snake(this->mGameBoardWidth, this->mGameBoardHeight, this->mInitialSnakeLength));
    
    // 为蛇设置地图
    this->mPtrSnake->setMap(this->mPtrMap.get());
    
    // 尝试寻找合适的蛇初始位置，使用逐步降低要求的策略
    bool snakeInitialized = false;
    
    // 首先尝试最理想的空间要求 (6格)
    std::vector<std::pair<SnakeBody, InitialDirection>> validPositions =
        this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 6);
    
    if (!validPositions.empty()) {
        int idx = std::rand() % validPositions.size();
        auto [startPos, direction] = validPositions[idx];
        this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
        snakeInitialized = true;
    }
    
    // 如果没有找到理想的位置，尝试降低空间要求（3格）
    if (!snakeInitialized) {
        validPositions = this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 3);
        
        if (!validPositions.empty()) {
            int idx = std::rand() % validPositions.size();
            auto [startPos, direction] = validPositions[idx];
            this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
            snakeInitialized = true;
        }
    }
    
    // 如果仍然没有找到合适位置，再降低要求（2格）
    if (!snakeInitialized) {
        validPositions = this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 2);
        
        if (!validPositions.empty()) {
            int idx = std::rand() % validPositions.size();
            auto [startPos, direction] = validPositions[idx];
            this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
            snakeInitialized = true;
        }
    }
    
    // 如果实在找不到任何合适位置，使用最小要求（1格）
    if (!snakeInitialized) {
        validPositions = this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 1);
        
        if (!validPositions.empty()) {
            int idx = std::rand() % validPositions.size();
            auto [startPos, direction] = validPositions[idx];
            this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
            snakeInitialized = true;
        }
        else {
            // 如果即使最低要求都满足不了，使用默认初始化
            this->mPtrSnake->initializeSnake();
        }
    }
    
    // 创建普通食物
    this->createRamdonFood();
    this->mPtrSnake->senseFood(this->mFood);
    
    // 创建特殊食物或毒药（100%概率生成）
    int specialRand = std::rand() % 100;
    if (specialRand < 70) {
        // 70%概率生成特殊食物
        this->createSpecialFood();
        this->mPtrSnake->senseSpecialFood(this->mSpecialFood);
    } else {
        // 30%概率生成毒药
        this->createPoison();
        this->mPtrSnake->sensePoison(this->mPoison);
    }
    
    // 创建随机道具（有10%概率生成）
    if (std::rand() % 100 < 10) {
        this->createRandomItem();
        this->mPtrSnake->senseRandomItem(this->mRandomItem);
    } else {
        mHasRandomItem = false;
    }
    
    this->mDifficulty = 0;
    this->mPoints = 0;
    this->mDelay = this->mBaseDelay;
    
    // 在经典模式中设置蛇的生命数
    if (mCurrentMode == GameMode::Classic) {
        this->mPtrSnake->setLives(3);
    }
}

void Game::createRamdonFood()
{
    // 使用地图提供的空白位置生成食物
    std::vector<SnakeBody> availableGrids;
    if (this->mPtrMap) {
        std::vector<SnakeBody> allParts;
        // 将两条蛇的身体合并到一个列表中
        if(mPtrSnake) allParts.insert(allParts.end(), mPtrSnake->getSnake().begin(), mPtrSnake->getSnake().end());
        if(mPtrSnake2) allParts.insert(allParts.end(), mPtrSnake2->getSnake().begin(), mPtrSnake2->getSnake().end());
        // 添加尸体食物位置到排除列表
        allParts.insert(allParts.end(), mCorpseFoods.begin(), mCorpseFoods.end());

        // 查找时排除所有蛇的身体部分和尸体食物
        availableGrids = this->mPtrMap->getEmptyPositions(allParts);
    } else {
        // 回退到原来的生成方法
        for (int i = 1; i < this->mGameBoardHeight - 1; i ++)
        {
            for (int j = 1; j < this->mGameBoardWidth - 1; j ++)
            {
                if(this->mPtrSnake->isPartOfSnake(j, i))
                {
                    continue;
                }
                else if(std::find_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                    [j, i](const SnakeBody& corpse) {
                        return corpse.getX() == j && corpse.getY() == i;
                    }) != mCorpseFoods.end())
                {
                    continue; // 排除尸体食物位置
                }
                else
                {
                    availableGrids.push_back(SnakeBody(j, i));
                }
            }
        }
    }

    // 如果没有可用的格子，游戏结束
    if (availableGrids.empty()) {
        return;
    }

    // Randomly select a grid that is not occupied by the snake
    int random_idx = std::rand() % availableGrids.size();
    this->mFood = availableGrids[random_idx];
}

void Game::createPoison()
{
    // 使用地图提供的空白位置生成毒药
    std::vector<SnakeBody> availableGrids;
    if (this->mPtrMap) {
        std::vector<SnakeBody> allParts;
        // 将两条蛇的身体合并到一个列表中
        if(mPtrSnake) allParts.insert(allParts.end(), mPtrSnake->getSnake().begin(), mPtrSnake->getSnake().end());
        if(mPtrSnake2) allParts.insert(allParts.end(), mPtrSnake2->getSnake().begin(), mPtrSnake2->getSnake().end());
        // 添加尸体食物位置到排除列表
        allParts.insert(allParts.end(), mCorpseFoods.begin(), mCorpseFoods.end());

        // 查找时排除所有蛇的身体部分、食物位置和尸体食物
        availableGrids = this->mPtrMap->getEmptyPositions(allParts);
        
        // 排除食物位置
        availableGrids.erase(
            std::remove_if(availableGrids.begin(), availableGrids.end(),
                [this](const SnakeBody& pos) {
                    return pos.getX() == mFood.getX() && pos.getY() == mFood.getY();
                }),
            availableGrids.end()
        );
    } else {
        // 回退到原来的生成方法
        for (int i = 1; i < this->mGameBoardHeight - 1; i ++)
        {
            for (int j = 1; j < this->mGameBoardWidth - 1; j ++)
            {
                if(this->mPtrSnake->isPartOfSnake(j, i))
                {
                    continue;
                }
                else if(j == mFood.getX() && i == mFood.getY())
                {
                    continue; // 排除食物位置
                }
                else if(std::find_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                    [j, i](const SnakeBody& corpse) {
                        return corpse.getX() == j && corpse.getY() == i;
                    }) != mCorpseFoods.end())
                {
                    continue; // 排除尸体食物位置
                }
                else
                {
                    availableGrids.push_back(SnakeBody(j, i));
                }
            }
        }
    }

    // 如果没有可用的格子，不生成毒药
    if (availableGrids.empty()) {
        mHasPoison = false;
        return;
    }

    // 随机选择位置生成毒药
    int random_idx = std::rand() % availableGrids.size();
    this->mPoison = availableGrids[random_idx];
    mHasPoison = true;
    mPoisonSpawnTime = std::chrono::steady_clock::now();
}

void Game::createSpecialFood()
{
    // 使用地图提供的空白位置生成特殊食物
    std::vector<SnakeBody> availableGrids;
    if (this->mPtrMap) {
        std::vector<SnakeBody> allParts;
        // 将两条蛇的身体合并到一个列表中
        if(mPtrSnake) allParts.insert(allParts.end(), mPtrSnake->getSnake().begin(), mPtrSnake->getSnake().end());
        if(mPtrSnake2) allParts.insert(allParts.end(), mPtrSnake2->getSnake().begin(), mPtrSnake2->getSnake().end());
        // 添加尸体食物位置到排除列表
        allParts.insert(allParts.end(), mCorpseFoods.begin(), mCorpseFoods.end());

        // 查找时排除所有蛇的身体部分、普通食物位置、毒药位置和尸体食物
        availableGrids = this->mPtrMap->getEmptyPositions(allParts);
        
        // 排除普通食物位置
        availableGrids.erase(
            std::remove_if(availableGrids.begin(), availableGrids.end(),
                [this](const SnakeBody& pos) {
                    return pos.getX() == mFood.getX() && pos.getY() == mFood.getY();
                }),
            availableGrids.end()
        );
        
        // 排除毒药位置
        if (mHasPoison) {
            availableGrids.erase(
                std::remove_if(availableGrids.begin(), availableGrids.end(),
                    [this](const SnakeBody& pos) {
                        return pos.getX() == mPoison.getX() && pos.getY() == mPoison.getY();
                    }),
                availableGrids.end()
            );
        }
    } else {
        // 回退到原来的生成方法
        for (int i = 1; i < this->mGameBoardHeight - 1; i ++)
        {
            for (int j = 1; j < this->mGameBoardWidth - 1; j ++)
            {
                if(this->mPtrSnake->isPartOfSnake(j, i))
                {
                    continue;
                }
                else if(j == mFood.getX() && i == mFood.getY())
                {
                    continue; // 排除普通食物位置
                }
                else if(mHasPoison && j == mPoison.getX() && i == mPoison.getY())
                {
                    continue; // 排除毒药位置
                }
                else if(std::find_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                    [j, i](const SnakeBody& corpse) {
                        return corpse.getX() == j && corpse.getY() == i;
                    }) != mCorpseFoods.end())
                {
                    continue; // 排除尸体食物位置
                }
                else
                {
                    availableGrids.push_back(SnakeBody(j, i));
                }
            }
        }
    }

    // 如果没有可用的格子，不生成特殊食物
    if (availableGrids.empty()) {
        mHasSpecialFood = false;
        return;
    }

    // 随机选择位置生成特殊食物
    int random_idx = std::rand() % availableGrids.size();
    this->mSpecialFood = availableGrids[random_idx];
    mHasSpecialFood = true;
    mSpecialFoodSpawnTime = std::chrono::steady_clock::now();
    
    // 随机选择特殊食物类型
    int foodTypeRand = std::rand() % 100;
    if (foodTypeRand < 50) {
        mCurrentFoodType = FoodType::Special1; // 50%概率 +2
    } else if (foodTypeRand < 80) {
        mCurrentFoodType = FoodType::Special2; // 30%概率 +3
    } else {
        mCurrentFoodType = FoodType::Special3; // 20%概率 +5
    }
}

void Game::createRandomItem()
{
    // 使用地图提供的空白位置生成随机道具
    std::vector<SnakeBody> availableGrids;
    if (this->mPtrMap) {
        std::vector<SnakeBody> allParts;
        // 将两条蛇的身体合并到一个列表中
        if(mPtrSnake) allParts.insert(allParts.end(), mPtrSnake->getSnake().begin(), mPtrSnake->getSnake().end());
        if(mPtrSnake2) allParts.insert(allParts.end(), mPtrSnake2->getSnake().begin(), mPtrSnake2->getSnake().end());
        // 添加尸体食物位置到排除列表
        allParts.insert(allParts.end(), mCorpseFoods.begin(), mCorpseFoods.end());

        // 查找时排除所有蛇的身体部分、食物位置、毒药位置和尸体食物
        availableGrids = this->mPtrMap->getEmptyPositions(allParts);
        
        // 排除普通食物位置
        availableGrids.erase(
            std::remove_if(availableGrids.begin(), availableGrids.end(),
                [this](const SnakeBody& pos) {
                    return pos.getX() == mFood.getX() && pos.getY() == mFood.getY();
                }),
            availableGrids.end()
        );
        
        // 排除毒药位置
        if (mHasPoison) {
            availableGrids.erase(
                std::remove_if(availableGrids.begin(), availableGrids.end(),
                    [this](const SnakeBody& pos) {
                        return pos.getX() == mPoison.getX() && pos.getY() == mPoison.getY();
                    }),
                availableGrids.end()
            );
        }
        
        // 排除特殊食物位置
        if (mHasSpecialFood) {
            availableGrids.erase(
                std::remove_if(availableGrids.begin(), availableGrids.end(),
                    [this](const SnakeBody& pos) {
                        return pos.getX() == mSpecialFood.getX() && pos.getY() == mSpecialFood.getY();
                    }),
                availableGrids.end()
            );
        }
    } else {
        // 回退到原来的生成方法
        for (int i = 1; i < this->mGameBoardHeight - 1; i ++)
        {
            for (int j = 1; j < this->mGameBoardWidth - 1; j ++)
            {
                if(this->mPtrSnake->isPartOfSnake(j, i))
                {
                    continue;
                }
                else if(j == mFood.getX() && i == mFood.getY())
                {
                    continue; // 排除普通食物位置
                }
                else if(mHasPoison && j == mPoison.getX() && i == mPoison.getY())
                {
                    continue; // 排除毒药位置
                }
                else if(mHasSpecialFood && j == mSpecialFood.getX() && i == mSpecialFood.getY())
                {
                    continue; // 排除特殊食物位置
                }
                else if(std::find_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                    [j, i](const SnakeBody& corpse) {
                        return corpse.getX() == j && corpse.getY() == i;
                    }) != mCorpseFoods.end())
                {
                    continue; // 排除尸体食物位置
                }
                else
                {
                    availableGrids.push_back(SnakeBody(j, i));
                }
            }
        }
    }

    // 如果没有可用的格子，不生成随机道具
    if (availableGrids.empty()) {
        mHasRandomItem = false;
        return;
    }

    // 随机选择位置生成随机道具
    int random_idx = std::rand() % availableGrids.size();
    this->mRandomItem = availableGrids[random_idx];
    mHasRandomItem = true;
    mRandomItemSpawnTime = std::chrono::steady_clock::now();
    
    // 随机选择道具类型
    int itemTypeRand = std::rand() % 100;
    if (itemTypeRand < 35) {
        mCurrentRandomItemType = ItemType::Portal; // 35%概率传送门
    } else if (itemTypeRand < 55) {
        mCurrentRandomItemType = ItemType::Shield; // 20%概率护盾
    } else if (itemTypeRand < 75) {
        mCurrentRandomItemType = ItemType::Cheat; // 20%概率作弊
    } else if (itemTypeRand < 90) {
        mCurrentRandomItemType = ItemType::Attack; // 15%概率攻击
    } else {
        mCurrentRandomItemType = ItemType::Poison; // 10%概率毒药
    }
}

void Game::createCorpseFoods(const std::vector<SnakeBody>& snakeBody)
{
    // 将蛇的尸体（包括头和身体）转换为食物，但排除在墙上的部分
    mCorpseFoods.clear(); // 清除之前的尸体食物
    
    for (const auto& bodyPart : snakeBody) {
        // 检查是否在墙上
        bool isOnWall = false;
        
        // 检查地图边界
        if (bodyPart.getX() < 0 || bodyPart.getX() >= mGameBoardWidth ||
            bodyPart.getY() < 0 || bodyPart.getY() >= mGameBoardHeight) {
            isOnWall = true;
        }
        // 检查地图中的墙
        else if (mPtrMap && mPtrMap->isWall(bodyPart.getX(), bodyPart.getY())) {
            isOnWall = true;
        }
        
        // 只有不在墙上的部分才转换为食物
        if (!isOnWall) {
            mCorpseFoods.push_back(bodyPart);
        }
    }
}

void Game::renderFood() const
{
    mvwaddch(this->mWindows[1], this->mFood.getY(), this->mFood.getX(), this->mFoodSymbol);
    wrefresh(this->mWindows[1]);
}

void Game::renderPoison() const
{
    if (mHasPoison) {
        // 使用红色显示毒药
        wattron(this->mWindows[1], COLOR_PAIR(4)); // 红色
        mvwaddch(this->mWindows[1], this->mPoison.getY(), this->mPoison.getX(), this->mPoisonSymbol);
        wattroff(this->mWindows[1], COLOR_PAIR(4));
        wrefresh(this->mWindows[1]);
    }
}

void Game::renderSpecialFood() const
{
    if (mHasSpecialFood) {
        // 使用紫色显示特殊食物，避免与AI蛇的黄色冲突
        wattron(this->mWindows[1], COLOR_PAIR(6)); // 紫色
        mvwaddch(this->mWindows[1], this->mSpecialFood.getY(), this->mSpecialFood.getX(), this->mSpecialFoodSymbol);
        wattroff(this->mWindows[1], COLOR_PAIR(6));
        wrefresh(this->mWindows[1]);
    }
}

void Game::renderRandomItem() const
{
    if (mHasRandomItem) {
        // 使用青色显示随机道具
        wattron(this->mWindows[1], COLOR_PAIR(1)); // 青色
        mvwaddch(this->mWindows[1], this->mRandomItem.getY(), this->mRandomItem.getX(), this->mRandomItemSymbol);
        wattroff(this->mWindows[1], COLOR_PAIR(1));
        wrefresh(this->mWindows[1]);
    }
}

void Game::renderCorpseFoods() const
{
    // 渲染所有尸体食物
    wattron(this->mWindows[1], COLOR_PAIR(3)); // 使用亮红色显示尸体食物，更明显
    for (const auto& corpseFood : mCorpseFoods) {
        mvwaddch(this->mWindows[1], corpseFood.getY(), corpseFood.getX(), this->mCorpseFoodSymbol);
    }
    wattroff(this->mWindows[1], COLOR_PAIR(3));
    wrefresh(this->mWindows[1]);
}

void Game::renderMap() const
{
    // 渲染地图上的墙体
    for (int y = 0; y < this->mGameBoardHeight; y++) {
        for (int x = 0; x < this->mGameBoardWidth; x++) {
            if (this->mPtrMap->isWall(x, y)) {
                mvwaddch(this->mWindows[1], y, x, this->mWallSymbol);
            }
        }
    }
    
    // 如果是第四关，还需要渲染终点
    if (mCurrentLevel == 4 && mHasEndpoint) {
        this->renderEndpoint();
    }
}

void Game::renderSnake() const
{
    int snakeLength = this->mPtrSnake->getLength();
    const std::vector<SnakeBody>& snake = this->mPtrSnake->getSnake();
    short color_pair = 1; // 默认青色
    switch (mCurrentSkin) {
        case SnakeSkin::Default: color_pair = 1; break;
        case SnakeSkin::Red:     color_pair = 4; break;
        case SnakeSkin::Blue:    color_pair = 5; break;
        case SnakeSkin::Green:   color_pair = 6; break;
        case SnakeSkin::Yellow:  color_pair = 2; break;
    }
    
    // 如果正在加速，使用特殊的加速颜色（亮红色）
    if (mAccelerating) {
        color_pair = 3; // 亮红色，表示加速状态
    }
    
    wattron(this->mWindows[1], COLOR_PAIR(color_pair));
    for (int i = 0; i < snakeLength; i++)
    {
        mvwaddch(this->mWindows[1], snake[i].getY(), snake[i].getX(), this->mSnakeSymbol);
    }
    wattroff(this->mWindows[1], COLOR_PAIR(color_pair));
    wrefresh(this->mWindows[1]);
}

void Game::controlSnake() const
{
    int key;
    key = getch();
    
    // 处理存档功能
    if (key == 'f' || key == 'F') {
        const_cast<Game*>(this)->saveGame();
        // 显示保存成功信息
        WINDOW* saveWin = newwin(3, 30, mGameBoardHeight/2 + mInformationHeight, mGameBoardWidth/2 - 15);
        box(saveWin, 0, 0);
        mvwprintw(saveWin, 1, 1, "Game Saved!");
        wrefresh(saveWin);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        delwin(saveWin);
        return;
    }
    
    // 处理道具使用
    const_cast<Game*>(this)->handleItemUsage(key);
    
    // 处理加速功能
    const_cast<Game*>(this)->handleAcceleration(key);
    
    // 如果是第四关，使用单键转弯控制
    if (mCurrentLevel == 4) {
        // 在第四关中，只需要一个按键 'T' 或空格键来转弯
        if (key == mSingleKeyTurnSymbol || key == 't' || key == ' ')
        {
            this->mPtrSnake->singleKeyTurn();
        }
        return;
    }
    
    // 正常的方向控制
    switch(key)
    {
        case 'W':
        case 'w':
        case KEY_UP:
        {
            this->mPtrSnake->changeDirection(Direction::Up);
            break;
        }
        case 'S':
        case 's':
        case KEY_DOWN:
        {
            this->mPtrSnake->changeDirection(Direction::Down);
            break;
        }
        case 'A':
        case 'a':
        case KEY_LEFT:
        {
            this->mPtrSnake->changeDirection(Direction::Left);
            break;
        }
        case 'D':
        case 'd':
        case KEY_RIGHT:
        {
            this->mPtrSnake->changeDirection(Direction::Right);
            break;
        }
        default:
        {
            break;
        }
    }
}

void Game::renderBoards() const
{
    for (size_t i = 0; i < this->mWindows.size(); i ++)
    {
        werase(this->mWindows[i]);
    }
    this->renderInformationBoard();
    this->renderGameBoard();
    this->renderInstructionBoard();
    for (size_t i = 0; i < this->mWindows.size(); i ++)
    {
        box(this->mWindows[i], 0, 0);
        wrefresh(this->mWindows[i]);
    }
    this->renderLeaderBoard();
}


void Game::adjustDelay()
{
    this->mDifficulty = this->mPoints / 5;
    if (mPoints % 5 == 0)
    {
        // 限制最大难度，防止蛇跑得太快
        int maxDifficulty = 10; // 最大难度限制
        int actualDifficulty = std::min(this->mDifficulty, maxDifficulty);
        
        // 使用更平缓的减速公式，并设置最小延迟
        int calculatedDelay = this->mBaseDelay * pow(0.95, actualDifficulty);
        int minDelay = 30; // 最小延迟30毫秒
        this->mDelay = std::max(calculatedDelay, minDelay);
    }
}

void Game::adjustBattleDelay()
{
    // 对战模式的延迟调整，基于蛇的长度而不是分数
    int p1Length = mPtrSnake ? mPtrSnake->getSnake().size() : 3;
    int p2Length = mPtrSnake2 ? mPtrSnake2->getSnake().size() : 3;
    int maxLength = std::max(p1Length, p2Length);
    
    // 基础延迟150毫秒，根据长度调整
    int baseDelay = 150;
    
    // 长度阈值和对应的延迟调整
    if (maxLength >= 15) {
        baseDelay = 80; // 长蛇：80毫秒
    } else if (maxLength >= 10) {
        baseDelay = 100; // 中等长度：100毫秒
    } else if (maxLength >= 5) {
        baseDelay = 120; // 短蛇：120毫秒
    }
    // 默认保持150毫秒
    
    // 更新对战模式的基础延迟
    mBattleBaseDelay = baseDelay;
}

void Game::runGame()
{
    while (true)
    {
        this->controlSnake();
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 更新作弊模式状态
        this->updateCheatMode();
        
        // 渲染地图
        this->renderMap();
        
        bool eatFood = this->mPtrSnake->moveFoward();
        bool eatPoison = this->mPtrSnake->touchPoison();
        bool eatSpecialFood = this->mPtrSnake->touchSpecialFood();
        bool eatCorpseFood = this->mPtrSnake->touchCorpseFood();
        bool eatRandomItem = this->mPtrSnake->touchRandomItem();
        bool collision = this->mPtrSnake->checkCollision();
        if (collision == true)
        {
            if (isShieldActive()) {
                // 护盾激活时，撤销移动（将蛇头移回安全位置）
                this->mPtrSnake->undoMove();
                // 自动反向
                Direction curDir = this->mPtrSnake->getDirection();
                Direction newDir;
                switch(curDir) {
                    case Direction::Up: newDir = Direction::Down; break;
                    case Direction::Down: newDir = Direction::Up; break;
                    case Direction::Left: newDir = Direction::Right; break;
                    case Direction::Right: newDir = Direction::Left; break;
                }
                this->mPtrSnake->changeDirection(newDir);
                deactivateShield();
                continue;
            } else {
                // 使用生命系统而不是直接结束游戏
                if (!this->mPtrSnake->loseLife()) {
                    // 没有剩余生命了，游戏结束
                    break;
                } else {
                    // 还有剩余生命，重置蛇的位置
                    this->mPtrSnake->initializeSnake();
                    continue;
                }
            }
        }
        this->renderSnake();
        if (eatFood == true)
        {
            // 处理普通食物效果
            handleFoodEffect(FoodType::Normal);
            this->createRamdonFood();
            this->mPtrSnake->senseFood(this->mFood);
            this->adjustDelay();
            
            // 重新生成特殊食物或毒药（100%概率生成）
            int specialRand = std::rand() % 100;
            if (specialRand < 70) {
                // 70%概率生成特殊食物
                this->createSpecialFood();
                this->mPtrSnake->senseSpecialFood(this->mSpecialFood);
            } else {
                // 30%概率生成毒药
                this->createPoison();
                this->mPtrSnake->sensePoison(this->mPoison);
            }
            
            // 重新生成随机道具（有10%概率）
            if (std::rand() % 100 < 10) {
                this->createRandomItem();
                this->mPtrSnake->senseRandomItem(this->mRandomItem);
            } else {
                mHasRandomItem = false;
            }
            if (this->isLevelCompleted())
            {
                // 如果达到目标分数，关卡通过
                this->renderFood();
                this->renderPoison();
                this->renderSpecialFood();
                this->renderCorpseFoods();
                this->renderRandomItem();
                this->renderDifficulty();
                this->renderPoints();
                this->renderLevel();
                refresh();
                break;
            }
        }
        if (eatSpecialFood == true && mHasSpecialFood)
        {
            // 处理特殊食物效果
            handleFoodEffect(mCurrentFoodType);
            mHasSpecialFood = false; // 特殊食物消失
        }
        if (eatPoison == true && mHasPoison)
        {
            // 处理毒药效果
            handleFoodEffect(FoodType::Poison);
            mHasPoison = false; // 毒药消失
        }
        if (eatCorpseFood == true && !mCorpseFoods.empty())
        {
            // 只在对战模式下处理尸体食物
            if (mCurrentMode == GameMode::Battle) {
                // 处理尸体食物效果
                handleFoodEffect(FoodType::Normal); // 尸体食物按普通食物处理
                // 移除被吃掉的尸体食物
                SnakeBody eatenCorpse = this->mPtrSnake->getEatenCorpseFood();
                if (eatenCorpse.getX() != -1 && eatenCorpse.getY() != -1) {
                    mCorpseFoods.erase(
                        std::remove_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                            [&eatenCorpse](const SnakeBody& corpse) {
                                return corpse.getX() == eatenCorpse.getX() && corpse.getY() == eatenCorpse.getY();
                            }),
                        mCorpseFoods.end()
                    );
                    // 更新蛇感知的尸体食物列表
                    this->mPtrSnake->senseCorpseFoods(this->mCorpseFoods);
                }
            }
        }
        if (eatRandomItem == true && mHasRandomItem)
        {
            // 处理随机道具效果
            addItem(mCurrentRandomItemType, 1); // 添加到库存
            mHasRandomItem = false; // 随机道具消失
        }
        this->renderFood();
        this->renderPoison();
        this->renderSpecialFood();
        this->renderCorpseFoods();
        this->renderRandomItem();
        this->renderDifficulty();
        this->renderPoints();
        // 即使在普通模式下，也显示当前为第1关
        this->renderLevel();

        // 根据加速状态调整延迟
        int currentDelay = mAccelerating ? mAccelerateDelay : this->mDelay;
        std::this_thread::sleep_for(std::chrono::milliseconds(currentDelay));

        refresh();

        // 检查特殊食物/毒药/道具是否超时消失
        auto now = std::chrono::steady_clock::now();
        if (mHasSpecialFood && std::chrono::duration_cast<std::chrono::seconds>(now - mSpecialFoodSpawnTime).count() > mSpecialFoodDuration) {
            mHasSpecialFood = false;
        }
        if (mHasPoison && std::chrono::duration_cast<std::chrono::seconds>(now - mPoisonSpawnTime).count() > mPoisonDuration) {
            mHasPoison = false;
        }
        if (mHasRandomItem && std::chrono::duration_cast<std::chrono::seconds>(now - mRandomItemSpawnTime).count() > mRandomItemDuration) {
            mHasRandomItem = false;
        }
    }
}

void Game::startGame()
{
    refresh();
    
    
    while(true){
        mReturnToModeSelect = false;
        if (!selectLevel()) {
            break; //用户选择退出
        }
            bool playAgain = true;
            while (playAgain) {
            switch(mCurrentMode) {
                case GameMode::Classic:
                case GameMode::Timed: {
                    if (mCurrentMode == GameMode::Classic) {
                        initializeGame();
                    } else {
                        initializeTimeAttack();
                    }
                    renderBoards();
                    if (mCurrentMode == GameMode::Classic) {
                        runGame();
                    } else {
                        runTimeAttack();
                    }
                    updateLeaderBoard();
                    writeLeaderBoard();
                    // 游戏结束时自动保存
                    saveGame();
                    playAgain = renderRestartMenu();
                    break;
                }
                case GameMode::Level:
                // 关卡模式
                this->readLeaderBoard();
                this->renderBoards();
        
                // 先让用户选择一个已解锁的关卡
                if (!this->selectLevelInLevelMode()) {
                // 用户选择退出
                  return;
                }
        
                // 运行选择的关卡
                while (true) {
                // 初始化并运行当前关卡
                    this->initializeLevel(mCurrentLevel);
                    this->runLevel();
            
                    // 检查是否通过当前关卡
                    if (this->isLevelCompleted()) {
                    // 标记当前关卡为已完成
                        this->mLevelStatus[mCurrentLevel - 1] = LevelStatus::Completed;
                
                        // 如果不是最后一关，解锁下一关
                        if (mCurrentLevel < mMaxLevel) {
                            this->unlockLevel(mCurrentLevel + 1);
                        }
                
                        // 保存关卡进度
                        this->saveLevelProgress();
                
                        // 显示通关信息
                        WINDOW* levelCompleteWin;
                        int width = this->mGameBoardWidth * 0.5;
                        int height = 5;
                        int startX = this->mGameBoardWidth * 0.25;
                        int startY = this->mGameBoardHeight * 0.25 + this->mInformationHeight;
                
                        levelCompleteWin = newwin(height, width, startY, startX);
                        box(levelCompleteWin, 0, 0);
                
                        mvwprintw(levelCompleteWin, 1, 1, "Level %d Completed!", mCurrentLevel);
                        mvwprintw(levelCompleteWin, 2, 1, "Your Score: %d", this->mPoints);
                
                        // 根据是否是最后一关显示不同的提示
                        if (mCurrentLevel < mMaxLevel) {
                            mvwprintw(levelCompleteWin, 3, 1, "Level %d Unlocked!", mCurrentLevel + 1);
                        } else {
                            mvwprintw(levelCompleteWin, 3, 1, "You completed all levels!");
                        }
                
                        mvwprintw(levelCompleteWin, 4, 1, "Press Space to continue...");
                        wrefresh(levelCompleteWin);
                
                        // 等待用户按空格继续
                        int key;
                        while (true) {
                            key = getch();
                            if (key == ' ' || key == 10)
                                break;
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                
                        delwin(levelCompleteWin);
                
                        // 如果所有关卡都完成了，显示胜利信息
                        if (mCurrentLevel >= mMaxLevel) {
                            WINDOW* gameCompleteWin;
                            gameCompleteWin = newwin(height, width, startY, startX);
                            box(gameCompleteWin, 0, 0);
                    
                            mvwprintw(gameCompleteWin, 1, 1, "Congratulations!");
                            mvwprintw(gameCompleteWin, 2, 1, "You completed all levels!");
                            mvwprintw(gameCompleteWin, 3, 1, "Final Score: %d", this->mPoints);
                            wrefresh(gameCompleteWin);
                    
                            // 等待用户按空格继续
                            while (true) {
                                key = getch();
                                if (key == ' ' || key == 10)
                                    break;
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                    
                            delwin(gameCompleteWin);
                        }
                
                        // 返回关卡选择界面
                        if (!this->selectLevelInLevelMode()) {
                            // 用户选择退出
                            break;
                        }
                
                    } else {
                        // 关卡失败，显示游戏结束信息
                        this->updateLeaderBoard();
                        this->writeLeaderBoard();
                        // 游戏结束时自动保存
                        this->saveGame();
                        WINDOW * menu;
                        int width = this->mGameBoardWidth * 0.5;
                        int height = this->mGameBoardHeight * 0.5;
                        int startX = this->mGameBoardWidth * 0.25;
                        int startY = this->mGameBoardHeight * 0.25 + this->mInformationHeight;

                        menu = newwin(height, width, startY, startX);
                        box(menu, 0, 0);
                        std::vector<std::string> menuItems = {"Retry Level", "Select Level", "Mode Select", "Quit"};

                        int index = 0;
                        int offset = 4;
                        mvwprintw(menu, 1, 1, "Your Final Score:");
                        std::string pointString = std::to_string(this->mPoints);
                        mvwprintw(menu, 2, 1, "%s", pointString.c_str());
                        wattron(menu, A_STANDOUT);
                        mvwprintw(menu, 0 + offset, 1, "%s", menuItems[0].c_str());
                        wattroff(menu, A_STANDOUT);
                        mvwprintw(menu, 1 + offset, 1, "%s", menuItems[1].c_str());
                        mvwprintw(menu, 2 + offset, 1, "%s", menuItems[2].c_str());
                        mvwprintw(menu, 3 + offset, 1, "%s", menuItems[3].c_str());

                        wrefresh(menu);

                        int key;
                        while (true)
                        {
                            key = getch();
                            switch(key)
                            {
                                case 'W':
                                case 'w':
                                case KEY_UP:
                                {
                                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                                    index--;
                                    index = (index < 0) ? static_cast<int>(menuItems.size()) - 1 : index;
                                    wattron(menu, A_STANDOUT);
                                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                                    wattroff(menu, A_STANDOUT);
                                    break;
                                }
                                case 'S':
                                case 's':
                                case KEY_DOWN:
                                {
                                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                                    index++;
                                    index = (index > static_cast<int>(menuItems.size()) - 1) ? 0 : index;
                                    wattron(menu, A_STANDOUT);
                                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                                    wattroff(menu, A_STANDOUT);
                                    break;
                                }
                            }
                            wrefresh(menu);
                            if (key == ' ' || key == 10)
                            {
                                break;
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                        delwin(menu);

                        if (index == 0) {
                            // 重新尝试当前关卡，不做任何改变
                            continue;
                        } else if (index == 1) {
                            // 返回关卡选择界面
                            if (!this->selectLevelInLevelMode()) {
                                // 用户选择退出
                                break;
                            }
                        } else if (index == 2) {
                            // 返回到模式选择
                            mReturnToModeSelect = true;
                            break;
                        } else {
                            // 退出游戏
                            break;
                        }
                    }
                }
                    break;
                case GameMode::Battle: {
                    if (!selectBattleType()) {
                        playAgain = false; // 如果用户从类型选择返回，则退出playAgain循环
                        break;
                    }
                    initializeBattle(mCurrentBattleType);
                    renderBoards();
                    runBattle();
                    // 游戏结束时自动保存
                    saveGame();
                    // 传入 true，让菜单显示 "Battle Over!"
                    playAgain = renderRestartMenu(true);
                    break;
                }
                case GameMode::Shop: {
                    showShopMenu();
                    playAgain = false; // 从商店返回后不重新开始游戏
                    break;
                }

            }
            
        }

    }
    
}

// https://en.cppreference.com/w/cpp/io/basic_fstream
bool Game::readLeaderBoard()
{
    std::fstream fhand(this->mRecordBoardFilePath, fhand.binary | fhand.in);
    if (!fhand.is_open())
    {
        return false;
    }
    int temp;
    int i = 0;
    while ((!fhand.eof()) && (i < mNumLeaders))
    {
        fhand.read(reinterpret_cast<char*>(&temp), sizeof(temp));
        this->mLeaderBoard[i] = temp;
        i ++;
    }
    fhand.close();
    return true;
}

bool Game::updateLeaderBoard()
{
    bool updated = false;
    int newScore = this->mPoints;
    for (int i = 0; i < this->mNumLeaders; i ++)
    {
        if (this->mLeaderBoard[i] >= this->mPoints)
        {
            continue;
        }
        int oldScore = this->mLeaderBoard[i];
        this->mLeaderBoard[i] = newScore;
        newScore = oldScore;
        updated = true;
    }
    return updated;
}

bool Game::writeLeaderBoard()
{
    // trunc: clear the data file
    std::fstream fhand(this->mRecordBoardFilePath, fhand.binary | fhand.trunc | fhand.out);
    if (!fhand.is_open())
    {
        return false;
    }
    for (int i = 0; i < this->mNumLeaders; i ++)
    {
        fhand.write(reinterpret_cast<char*>(&this->mLeaderBoard[i]), sizeof(this->mLeaderBoard[i]));;
    }
    fhand.close();
    return true;
}

bool Game::selectLevel()
{
    while (true) { // 用循环包裹，方便从Shop返回后重新显示菜单
        clear();
        refresh();

        WINDOW * menu;
        int width = this->mGameBoardWidth * 0.6;
        int height = this->mGameBoardHeight * 0.6;
        int startX = this->mGameBoardWidth * 0.2;
        int startY = this->mGameBoardHeight * 0.2 + this->mInformationHeight;

        menu = newwin(height, width, startY, startX);
        box(menu, 0, 0);

        std::vector<std::string> menuItems = {
            "Classic Mode",
            "Level Mode",
            "Timed Mode",
            "Battle Mode",
            "Shop",
            "Load Game",
            "Exit Game"
        };

        int index = 0;
        int offset = 3;
        mvwprintw(menu, 1, 1, "Select Game Mode:");

        wattron(menu, A_STANDOUT);
        mvwprintw(menu, 0 + offset, 1, "%s", menuItems[0].c_str());
        wattroff(menu, A_STANDOUT);

        for (size_t i = 1; i < menuItems.size(); i++) {
            mvwprintw(menu, static_cast<int>(i) + offset, 1, "%s", menuItems[i].c_str());
        }

        wrefresh(menu);

        int key;
        while (true)
        {
            key = getch();
            switch(key)
            {
                case 'W':
                case 'w':
                case KEY_UP:
                {
                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                    index--;
                    index = (index < 0) ? static_cast<int>(menuItems.size()) - 1 : index;
                    wattron(menu, A_STANDOUT);
                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                    wattroff(menu, A_STANDOUT);
                    break;
                }
                case 'S':
                case 's':
                case KEY_DOWN:
                {
                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                    index++;
                    index = (index > static_cast<int>(menuItems.size()) - 1) ? 0 : index;
                    wattron(menu, A_STANDOUT);
                    mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                    wattroff(menu, A_STANDOUT);
                    break;
                }
            }
            wrefresh(menu);
            if (key == ' ' || key == 10)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        delwin(menu);

        if (index == 6) { // Exit Game
            return false;
        } else if (index == 4) { // Shop
            showShopMenu(); // 这里调用你的商店界面函数
            continue; // 回到主菜单
        } else if (index == 5) { // Load Game
            if (hasSaveFile()) {
                if (loadGame()) {
                    mReturnToModeSelect = false;
                    return true; // 成功加载存档，开始游戏
                } else {
                    // 加载失败，显示错误信息
                    WINDOW* errorWin = newwin(5, 40, startY + height/2, startX + width/2 - 20);
                    box(errorWin, 0, 0);
                    mvwprintw(errorWin, 1, 1, "Failed to load save file!");
                    mvwprintw(errorWin, 2, 1, "Press any key to continue...");
                    wrefresh(errorWin);
                    getch();
                    delwin(errorWin);
                    continue; // 回到主菜单
                }
            } else {
                // 没有存档文件，显示提示
                WINDOW* noSaveWin = newwin(5, 40, startY + height/2, startX + width/2 - 20);
                box(noSaveWin, 0, 0);
                mvwprintw(noSaveWin, 1, 1, "No save file found!");
                mvwprintw(noSaveWin, 2, 1, "Press any key to continue...");
                wrefresh(noSaveWin);
                getch();
                delwin(noSaveWin);
                continue; // 回到主菜单
            }
        } else {
            mCurrentMode = static_cast<GameMode>(index);
            mReturnToModeSelect = false;
            return true;
        }
    }
}
void Game::initializeLevel(int level)
{
    mCurrentLevel = level;
    
    // 根据关卡设置关卡类型
    switch (level) {
        case 1:
            mCurrentLevelType = LevelType::Normal;
            mLevelTargetPoints = 5;
            break;
        case 2:
            mCurrentLevelType = LevelType::Speed;
            mLevelTargetPoints = 8;
            break;
        case 3:
            mCurrentLevelType = LevelType::Maze;
            mLevelTargetPoints = 10;
            break;
        case 4:
            mCurrentLevelType = LevelType::Custom1;
            mLevelTargetPoints = 12;
            // 初始化第四关特殊设置
            this->initializeLevel4();
            return; // 第四关有特殊初始化，直接返回
        case 5:
            mCurrentLevelType = LevelType::Custom2;
            mLevelTargetPoints = 15;
            // 初始化第五关特殊设置
            this->initializeLevel5();
            return; // 第五关有特殊初始化，直接返回
        default:
            mCurrentLevelType = LevelType::Normal;
            mLevelTargetPoints = 5;
            break;
    }
    
    // 加载关卡对应的地图
    std::string mapFilePath;
    if (level >= 1 && level <= mMaxLevel) {
        mapFilePath = mLevelMapFiles[level - 1];
    } else {
        mapFilePath = mLevelMapFiles[0];
    }
    
    // 创建地图
    mPtrMap = std::make_unique<Map>(mGameBoardWidth, mGameBoardHeight);
    
    // 检查地图文件是否存在
    std::ifstream mapFile(mapFilePath);
    if (mapFile.good()) {
        mapFile.close();
        mPtrMap->loadMapFromFile(mapFilePath);
    } else {
        // 如果地图文件不存在，加载默认地图
        mPtrMap->loadDefaultMap();
    }
    
    // 创建蛇
    this->mPtrSnake.reset(new Snake(this->mGameBoardWidth, this->mGameBoardHeight, this->mInitialSnakeLength));
    this->mPtrSnake->setMap(this->mPtrMap.get());
    
    // 尝试寻找合适的蛇初始位置
    bool snakeInitialized = false;
    std::vector<std::pair<SnakeBody, InitialDirection>> validPositions =
        this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 6);
    
    if (!validPositions.empty()) {
        int idx = std::rand() % validPositions.size();
        auto [startPos, direction] = validPositions[idx];
        this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
        snakeInitialized = true;
    }
    
    // 如果没有找到理想的位置，尝试降低空间要求
    if (!snakeInitialized) {
        validPositions = this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 3);
        
        if (!validPositions.empty()) {
            int idx = std::rand() % validPositions.size();
            auto [startPos, direction] = validPositions[idx];
            this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
            snakeInitialized = true;
        }
    }
    
    if (!snakeInitialized) {
        this->mPtrSnake->initializeSnake();
    }
    
    // 创建普通食物
    this->createRamdonFood();
    this->mPtrSnake->senseFood(this->mFood);
    
    // 创建特殊食物或毒药（100%概率生成）
    int specialRand = std::rand() % 100;
    if (specialRand < 70) {
        // 70%概率生成特殊食物
        this->createSpecialFood();
        this->mPtrSnake->senseSpecialFood(this->mSpecialFood);
    } else {
        // 30%概率生成毒药
        this->createPoison();
        this->mPtrSnake->sensePoison(this->mPoison);
    }
    
    // 创建随机道具（有10%概率生成）
    if (std::rand() % 100 < 10) {
        this->createRandomItem();
        this->mPtrSnake->senseRandomItem(this->mRandomItem);
    } else {
        mHasRandomItem = false;
    }
    
    // 设置难度参数（不同关卡可以有不同的初始难度）
    switch (mCurrentLevelType) {
        case LevelType::Speed:
            this->mDifficulty = 2;  // 速度关卡初始难度更高
            break;
        case LevelType::Custom1:
        case LevelType::Custom2:
            this->mDifficulty = 1;  // 自定义关卡初始难度
            break;
        default:
            this->mDifficulty = 0;  // 普通关卡初始难度
            break;
    }
    
    this->mPoints = 0;
    this->mDelay = this->mBaseDelay * pow(0.75, this->mDifficulty);
}

// 初始化第四关特殊设置
void Game::initializeLevel4()
{
    // 加载地图
    std::string mapFilePath = mLevelMapFiles[3]; // level4.txt
    mPtrMap = std::make_unique<Map>(mGameBoardWidth, mGameBoardHeight);
    
    // 检查地图文件是否存在
    std::ifstream mapFile(mapFilePath);
    if (mapFile.good()) {
        mapFile.close();
        mPtrMap->loadMapFromFile(mapFilePath);
    } else {
        // 如果地图文件不存在，加载默认地图
        mPtrMap->loadDefaultMap();
    }
    
    // 创建蛇
    this->mPtrSnake.reset(new Snake(this->mGameBoardWidth, this->mGameBoardHeight, this->mInitialSnakeLength));
    this->mPtrSnake->setMap(this->mPtrMap.get());
    
    // 第四关使用单键转向模式
    this->mPtrSnake->setTurnMode(TurnMode::SingleKey);
    
    // 设置蛇的初始位置（从迷宫的入口开始）
    this->mPtrSnake->initializeSnake(2, 1, InitialDirection::Down);
    
    // 设置终点位置（迷宫出口）
    mEndpoint = SnakeBody(63, 11);
    mHasEndpoint = true;
    
    // 第四关不需要创建食物，直接将食物设置在不可能到达的位置
    mFood = SnakeBody(0, 0);
    this->mPtrSnake->senseFood(this->mFood);
    
    // 设置难度
    this->mDifficulty = 1;
    this->mDelay = this->mBaseDelay * pow(1.25, this->mDifficulty);
    this->mPoints = 0;
}

void Game::renderEndpoint() const
{
    if (!mHasEndpoint) return;
    
    // 在游戏面板上绘制终点标记
    mvwaddch(this->mWindows[1], mEndpoint.getY(), mEndpoint.getX(), mEndpointSymbol);
}

// 第四关蛇的单键控制
void Game::controlSnakeLevel4() const
{
    int key;
    key = getch();
    
    // 在第四关中，只需要一个按键 'T' 或空格键来转弯
    if (key == mSingleKeyTurnSymbol || key == 't' || key == ' ')
    {
        this->mPtrSnake->singleKeyTurn();
    }
}

// 运行第四关特殊逻辑
void Game::runLevel4()
{
    // 更新信息面板，显示关卡提示
    mvwprintw(this->mWindows[0], 1, 1, "Level 4: Single Path Challenge");
    mvwprintw(this->mWindows[0], 2, 1, "Press 'T' or SPACE to turn");
    mvwprintw(this->mWindows[0], 3, 1, "Reach the 'X' mark to win!");
    wrefresh(this->mWindows[0]);
    
    while (true)
    {
        // 使用单键控制
        this->controlSnakeLevel4();
        
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 渲染地图
        this->renderMap();
        
        // 渲染终点
        this->renderEndpoint();
        
        // 移动蛇
        this->mPtrSnake->moveFoward();
        
        // 检查是否撞墙或自己
        bool collision = this->mPtrSnake->checkCollision();
        if (collision == true)
        {
            // 如果碰撞，关卡失败
            break;
        }
        
        // 检查是否到达终点
        if (this->mPtrSnake->reachedEndpoint(mEndpoint.getX(), mEndpoint.getY()))
        {
            // 如果达到终点，关卡通过
            this->renderSnake();
            this->renderLevel();
            refresh();
            
            // 设置完成关卡的标志
            mPoints = mLevelTargetPoints;
            break;
        }
        
        this->renderSnake();
        this->renderLevel();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        
        refresh();
    }
}

void Game::loadNextLevel()
{
    if (mCurrentLevel < mMaxLevel) {
        mCurrentLevel++;
        initializeLevel(mCurrentLevel);
    }
}

bool Game::isLevelCompleted()
{
    // 当前关卡完成条件：达到目标分数
    return (mPoints >= mLevelTargetPoints);
}

void Game::runLevel()
{
    // 如果是第四关，使用特殊的运行逻辑
    if (mCurrentLevel == 4) {
        this->runLevel4();
        return;
    }
    
    // 如果是第五关，使用Boss战逻辑
    if (mCurrentLevel == 5) {
        this->runLevel5();
        return;
    }
    
    // 其他关卡的运行逻辑
    while (true)
    {
        this->controlSnake();
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 渲染地图
        this->renderMap();
        
        bool eatFood = this->mPtrSnake->moveFoward();
        bool eatPoison = this->mPtrSnake->touchPoison();
        bool eatSpecialFood = this->mPtrSnake->touchSpecialFood();
        bool eatCorpseFood = this->mPtrSnake->touchCorpseFood();
        bool eatRandomItem = this->mPtrSnake->touchRandomItem();
        bool collision = this->mPtrSnake->checkCollision();
        
        if (collision == true)
        {
            if (isShieldActive()) {
                // 护盾激活时，撤销移动（将蛇头移回安全位置）
                this->mPtrSnake->undoMove();
                // 自动反向
                Direction curDir = this->mPtrSnake->getDirection();
                Direction newDir;
                switch(curDir) {
                    case Direction::Up: newDir = Direction::Down; break;
                    case Direction::Down: newDir = Direction::Up; break;
                    case Direction::Left: newDir = Direction::Right; break;
                    case Direction::Right: newDir = Direction::Left; break;
                }
                this->mPtrSnake->changeDirection(newDir);
                deactivateShield();
                continue;
            } else {
                break;
            }
        }
        
        this->renderSnake();
        if (eatFood == true)
        {
            // 处理普通食物效果
            handleFoodEffect(FoodType::Normal);
            this->createRamdonFood();
            this->mPtrSnake->senseFood(this->mFood);
            this->adjustDelay();
            
            // 重新生成特殊食物或毒药（100%概率生成）
            int specialRand = std::rand() % 100;
            if (specialRand < 70) {
                // 70%概率生成特殊食物
                this->createSpecialFood();
                this->mPtrSnake->senseSpecialFood(this->mSpecialFood);
            } else {
                // 30%概率生成毒药
                this->createPoison();
                this->mPtrSnake->sensePoison(this->mPoison);
            }
            
            // 重新生成随机道具（有10%概率）
            if (std::rand() % 100 < 10) {
                this->createRandomItem();
                this->mPtrSnake->senseRandomItem(this->mRandomItem);
            } else {
                mHasRandomItem = false;
            }
            if (this->isLevelCompleted())
            {
                // 如果达到目标分数，关卡通过
                this->renderFood();
                this->renderPoison();
                this->renderSpecialFood();
                this->renderCorpseFoods();
                this->renderRandomItem();
                this->renderDifficulty();
                this->renderPoints();
                this->renderLevel();
                refresh();
                break;
            }
        }
        if (eatSpecialFood == true && mHasSpecialFood)
        {
            // 处理特殊食物效果
            handleFoodEffect(mCurrentFoodType);
            mHasSpecialFood = false; // 特殊食物消失
        }
        if (eatPoison == true && mHasPoison)
        {
            // 处理毒药效果
            handleFoodEffect(FoodType::Poison);
            mHasPoison = false; // 毒药消失
        }
        if (eatCorpseFood == true && !mCorpseFoods.empty())
        {
            // 只在对战模式下处理尸体食物
            if (mCurrentMode == GameMode::Battle) {
                // 处理尸体食物效果
                handleFoodEffect(FoodType::Normal); // 尸体食物按普通食物处理
                // 移除被吃掉的尸体食物
                SnakeBody eatenCorpse = this->mPtrSnake->getEatenCorpseFood();
                if (eatenCorpse.getX() != -1 && eatenCorpse.getY() != -1) {
                    mCorpseFoods.erase(
                        std::remove_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                            [&eatenCorpse](const SnakeBody& corpse) {
                                return corpse.getX() == eatenCorpse.getX() && corpse.getY() == eatenCorpse.getY();
                            }),
                        mCorpseFoods.end()
                    );
                    // 更新蛇感知的尸体食物列表
                    this->mPtrSnake->senseCorpseFoods(this->mCorpseFoods);
                }
            }
        }
        if (eatRandomItem == true && mHasRandomItem)
        {
            // 处理随机道具效果
            addItem(mCurrentRandomItemType, 1); // 添加到库存
            mHasRandomItem = false; // 随机道具消失
        }
        this->renderFood();
        this->renderPoison();
        this->renderSpecialFood();
        this->renderCorpseFoods();
        this->renderRandomItem();
        this->renderDifficulty();
        this->renderPoints();
        this->renderLevel();
        
        // 根据关卡类型调整游戏逻辑
        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        
        refresh();

        // 检查特殊食物/毒药/道具是否超时消失
        auto now = std::chrono::steady_clock::now();
        if (mHasSpecialFood && std::chrono::duration_cast<std::chrono::seconds>(now - mSpecialFoodSpawnTime).count() > mSpecialFoodDuration) {
            mHasSpecialFood = false;
        }
        if (mHasPoison && std::chrono::duration_cast<std::chrono::seconds>(now - mPoisonSpawnTime).count() > mPoisonDuration) {
            mHasPoison = false;
        }
        if (mHasRandomItem && std::chrono::duration_cast<std::chrono::seconds>(now - mRandomItemSpawnTime).count() > mRandomItemDuration) {
            mHasRandomItem = false;
        }
    }
}

bool Game::selectLevelInLevelMode()
{
    // 清除屏幕并刷新，以确保界面正确显示
    clear();
    refresh();
    
    WINDOW * menu;
    int width = this->mGameBoardWidth * 0.6;
    int height = this->mGameBoardHeight * 0.6;
    int startX = this->mGameBoardWidth * 0.2;
    int startY = this->mGameBoardHeight * 0.2 + this->mInformationHeight;

    menu = newwin(height, width, startY, startX);
    box(menu, 0, 0);
    
    std::vector<std::string> menuItems;
    
    // 添加已解锁的关卡到菜单
    for (int i = 0; i < mMaxLevel; i++) {
        if (mLevelStatus[i] != LevelStatus::Locked) {
            std::string itemText = "Level " + std::to_string(i + 1);
            
            // // 如果关卡已完成，添加完成标记
            // if (mLevelStatus[i] == LevelStatus::Completed) {
            //     itemText += " (Completed)";
            // }
            
            menuItems.push_back(itemText);
        } else {
            // 锁定的关卡显示为锁定状态
            menuItems.push_back("Level " + std::to_string(i + 1) + " (Locked)");
        }
    }
    
    // 添加返回选项
    menuItems.push_back("Back to Mode Selection");
    menuItems.push_back("Quit Game");

    int index = 0;
    int offset = 3;
    mvwprintw(menu, 1, 1, "Select Level:");
    
    // 渲染菜单项
    for (size_t i = 0; i < menuItems.size(); i++) {
        if (i == 0) {
            wattron(menu, A_STANDOUT);
        }
        
        // 如果是已锁定的关卡，显示为灰色
        if (i < static_cast<size_t>(mMaxLevel) && mLevelStatus[i] == LevelStatus::Locked) {
            wattron(menu, A_DIM);
        }
        
        mvwprintw(menu, static_cast<int>(i) + offset, 1, "%s", menuItems[i].c_str());
        
        if (i < static_cast<size_t>(mMaxLevel) && mLevelStatus[i] == LevelStatus::Locked) {
            wattroff(menu, A_DIM);
        }
        
        if (i == 0) {
            wattroff(menu, A_STANDOUT);
        }
    }

    wrefresh(menu);

    int key;
    while (true)
    {
        key = getch();
        switch(key)
        {
            case 'W':
            case 'w':
            case KEY_UP:
            {
                // 移除当前选项的高亮
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattroff(menu, A_DIM);
                }
                
                // 选择前一个选项
                index--;
                index = (index < 0) ? static_cast<int>(menuItems.size()) - 1 : index;
                
                // 高亮新选项
                wattron(menu, A_STANDOUT);
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattroff(menu, A_DIM);
                }
                wattroff(menu, A_STANDOUT);
                break;
            }
            case 'S':
            case 's':
            case KEY_DOWN:
            {
                // 移除当前选项的高亮
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattroff(menu, A_DIM);
                }
                
                // 选择下一个选项
                index++;
                index = (index > static_cast<int>(menuItems.size()) - 1) ? 0 : index;
                
                // 高亮新选项
                wattron(menu, A_STANDOUT);
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattroff(menu, A_DIM);
                }
                wattroff(menu, A_STANDOUT);
                break;
            }
        }
        wrefresh(menu);
        if (key == ' ' || key == 10)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    delwin(menu);
    
    // 如果选择了返回模式选择
    if (static_cast<size_t>(index) == menuItems.size() - 2) {
        mReturnToModeSelect = true;
        return false;
    }
    // 如果选择了退出游戏
    else if (static_cast<size_t>(index) == menuItems.size() - 1) {
        return false;
    }
    // 如果选择了一个锁定的关卡
    else if (index >= 0 && index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
        // 显示提示信息
        WINDOW* lockedWin;
        int lockWidth = this->mGameBoardWidth * 0.4;
        int lockHeight = 4;
        int lockStartX = this->mGameBoardWidth * 0.3;
        int lockStartY = this->mGameBoardHeight * 0.4 + this->mInformationHeight;
        
        lockedWin = newwin(lockHeight, lockWidth, lockStartY, lockStartX);
        box(lockedWin, 0, 0);
        
        mvwprintw(lockedWin, 1, 1, "Level %d is locked!", index + 1);
        mvwprintw(lockedWin, 2, 1, "Complete previous level first.");
        wrefresh(lockedWin);
        
        // 等待用户按任意键继续
        getch();
        
        delwin(lockedWin);
        
        // 递归调用自身，让用户重新选择关卡
        return selectLevelInLevelMode();
    }
    // 选择了有效的关卡
    else {
        mCurrentLevel = index + 1;
        return true;
    }
}

void Game::unlockLevel(int level)
{
    if (level > 0 && level <= mMaxLevel) {
        mLevelStatus[level - 1] = LevelStatus::Unlocked;
    }
}

bool Game::saveLevelProgress()
{
    std::fstream fhand(this->mLevelProgressFilePath, std::ios::binary | std::ios::trunc | std::ios::out);
    if (!fhand.is_open())
    {
        return false;
    }
    
    // 保存所有关卡状态
    for (int i = 0; i < mMaxLevel; i++)
    {
        int status = static_cast<int>(mLevelStatus[i]);
        fhand.write(reinterpret_cast<char*>(&status), sizeof(status));
    }
    
    fhand.close();
    return true;
}

bool Game::loadLevelProgress()
{
    std::fstream fhand(this->mLevelProgressFilePath, std::ios::binary | std::ios::in);
    if (!fhand.is_open())
    {
        // 如果文件不存在，第一关默认解锁
        this->mLevelStatus[0] = LevelStatus::Unlocked;
        return false;
    }
    
    int status;
    int i = 0;
    while ((!fhand.eof()) && (i < mMaxLevel))
    {
        fhand.read(reinterpret_cast<char*>(&status), sizeof(status));
        if (!fhand.eof()) {
            this->mLevelStatus[i] = static_cast<LevelStatus>(status);
        }
        i++;
    }
    
    fhand.close();
    return true;
}



void Game::initializeTimeAttack()
{
    // 调用经典的初始化函数来完成大部分工作
    initializeGame();
    
    // 设置限时模式的特定参数
    mTimeAttackDurationSeconds = 120; // 设定120秒
    mTimeRemaining = mTimeAttackDurationSeconds;
    mTimeAttackStartTime = std::chrono::steady_clock::now(); // 开始计时！
}

// 在侧边栏渲染计时器
void Game::renderTimer() const
{
    mvwprintw(this->mWindows[2], 14, 1, "Time Left:");
    std::string timeString = std::to_string(mTimeRemaining) + " s";
    mvwprintw(this->mWindows[2], 15, 2, "%10s", ""); // 清空旧内容
    mvwprintw(this->mWindows[2], 15, 2, "%s", timeString.c_str());
    wrefresh(this->mWindows[2]);
}

// 限时模式的主循环
void Game::runTimeAttack()
{
    while (true)
    {
        //计时器逻辑
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - mTimeAttackStartTime);
        mTimeRemaining = mTimeAttackDurationSeconds - elapsed.count();

        // 游戏结束条件
        bool collision = mPtrSnake->checkCollision();
        if (collision || mTimeRemaining <= 0) {
            // 如果撞墙或时间到，则结束游戏
            break;
        }

        // 游戏循环核心
        this->controlSnake();
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        this->renderMap();
        
        bool eatFood = this->mPtrSnake->moveFoward();
        this->renderSnake();

        if (eatFood == true)
        {
            this->mPoints += 1;
            this->createRamdonFood();
            this->mPtrSnake->senseFood(this->mFood);
            this->adjustDelay();
            addCoins(1);
        }

        this->renderFood();
        this->renderPoison();
        this->renderSpecialFood();
        this->renderRandomItem();
        this->renderDifficulty();
        this->renderPoints();
        this->renderTimer(); // 在每一帧都渲染计时器

        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        refresh();
    }
}

// 添加第五关初始化函数
void Game::initializeLevel5()
{
    // 加载地图
    std::string mapFilePath = mLevelMapFiles[4]; // level5.txt
    mPtrMap = std::make_unique<Map>(mGameBoardWidth, mGameBoardHeight);
    
    // 检查地图文件是否存在
    std::ifstream mapFile(mapFilePath);
    if (mapFile.good()) {
        mapFile.close();
        mPtrMap->loadMapFromFile(mapFilePath);
    } else {
        // 如果地图文件不存在，加载默认地图
        mPtrMap->loadDefaultMap();
    }
    
    // 创建蛇（固定长度）
    this->mPtrSnake.reset(new Snake(this->mGameBoardWidth, this->mGameBoardHeight, this->mInitialSnakeLength));
    this->mPtrSnake->setMap(this->mPtrMap.get());
    this->mPtrSnake->setFixedLength(true); // 设置蛇为固定长度
    
    // 初始化Boss属性
    mBossHP = 5;
    mBossSize = 5; // 5x5的方形
    
    // 计算Boss位置（场地中央）
    int centerX = this->mGameBoardWidth / 2;
    int centerY = this->mGameBoardHeight / 2;
    mBossPosition.first = centerX - mBossSize/2;
    mBossPosition.second = centerY - mBossSize/2;
    
    // 初始化Boss状态
    mBossState = BossState::Red;
    mBossStateStartTime = std::chrono::steady_clock::now();
    mBossStateDuration = 0.0f;
    
    // 初始化无敌状态
    mSnakeInvincible = false;
    
    // 初始化攻击点
    updateBossAttackPoint();
    
    // 初始化旋转激光
    mLaserAngle = 0.0;
    mLaserRotationSpeed = 3.0; // 每帧旋转的角度，略微增加旋转速度
    mLaserLength = std::max(mGameBoardWidth, mGameBoardHeight) * 2; // 确保激光能覆盖整个场地
    
    // 初始化计分
    this->mPoints = 0;
    this->mDifficulty = 2; // Boss关卡难度较高
    this->mDelay = this->mBaseDelay * 2.0; // 将速度降低为原来的1/2（延迟增加为2倍）
    
    // 选择一个远离Boss的位置作为蛇的起始位置
    // 选择四个角落中的一个
    int startX, startY;
    int cornerChoice = std::rand() % 4; // 随机选择四个角落之一
    
    switch (cornerChoice) {
        case 0: // 左上角
            startX = 5;
            startY = 5;
            this->mPtrSnake->initializeSnake(startX, startY, InitialDirection::Right);
            break;
            
        case 1: // 右上角
            startX = this->mGameBoardWidth - 10;
            startY = 5;
            this->mPtrSnake->initializeSnake(startX, startY, InitialDirection::Left);
            break;
            
        case 2: // 左下角
            startX = 5;
            startY = this->mGameBoardHeight - 10;
            this->mPtrSnake->initializeSnake(startX, startY, InitialDirection::Right);
            break;
            
        case 3: // 右下角
            startX = this->mGameBoardWidth - 10;
            startY = this->mGameBoardHeight - 10;
            this->mPtrSnake->initializeSnake(startX, startY, InitialDirection::Left);
            break;
    }
}

// 添加第五关运行函数
void Game::runLevel5()
{
    // 更新信息面板，显示Boss战提示
    mvwprintw(this->mWindows[0], 1, 1, "Level 5: Boss Battle");
    mvwprintw(this->mWindows[0], 2, 1, "Defeat the Core! Boss HP: %d/5", mBossHP);
    mvwprintw(this->mWindows[0], 3, 1, "Avoid rotating lasers!");
    wrefresh(this->mWindows[0]);
    
    while (true)
    {
        // 控制蛇的移动
        this->controlSnake();
        
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 渲染地图
        this->renderMap();
        
        // 更新Boss状态
        updateBossState();
        
        // 渲染Boss
        renderBoss();
        
        // 更新并渲染激光（激光一直存在）
        updateAndRenderLasers();
        
        // 移动蛇
        this->mPtrSnake->moveFoward();
        
        // 检查蛇是否撞墙或自己
        bool collision = this->mPtrSnake->checkCollision();
        if (collision)
        {
            // 如果碰撞，关卡失败
            break;
        }
        
        // 检查蛇是否撞到激光（如果不是无敌状态）
        if (!mSnakeInvincible && checkLaserCollision())
        {
            // 如果碰到激光，关卡失败
            break;
        }
        
        // 如果是绿色状态，检查蛇是否攻击到Boss的攻击点
        if (mBossState == BossState::Green && checkBossAttack())
        {
            // 减少Boss血量
            mBossHP--;
            
            // 更新信息面板上的Boss血量
            mvwprintw(this->mWindows[0], 2, 1, "Defeat the Core! Boss HP: %d/5", mBossHP);
            mvwprintw(this->mWindows[0], 3, 1, "You're invincible! Move away!");
            wrefresh(this->mWindows[0]);
            
            // 增加得分
            this->mPoints += 3;
            
            // 开启无敌状态
            mSnakeInvincible = true;
            mInvincibleStartTime = std::chrono::steady_clock::now();
            
            // 切换到红色状态
            mBossState = BossState::Red;
            mBossStateStartTime = std::chrono::steady_clock::now();
            
            // 如果Boss血量为0，玩家胜利
            if (mBossHP <= 0)
            {
                // 设置分数以满足关卡通关条件
                this->mPoints = mLevelTargetPoints;
                break;
            }
        }
        
        this->renderSnake();
        this->renderPoints();
        this->renderLevel();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        
        refresh();
    }
}

// 更新Boss状态
void Game::updateBossState()
{
    // 计算当前状态已经持续的时间（秒）
    auto now = std::chrono::steady_clock::now();
    auto elapsedSeconds = std::chrono::duration<float>(now - mBossStateStartTime).count();
    mBossStateDuration = elapsedSeconds;
    
    // 如果蛇处于无敌状态，检查是否结束
    if (mSnakeInvincible)
    {
        auto invincibleElapsed = std::chrono::duration<float>(now - mInvincibleStartTime).count();
        if (invincibleElapsed >= mInvincibleDuration)
        {
            mSnakeInvincible = false;
        }
    }
    
    // 根据当前状态和已经过的时间决定是否需要转换状态
    switch (mBossState)
    {
        case BossState::Red:
            // 红色状态持续6秒后，自动转换为绿色状态
            if (mBossStateDuration >= mRedStateDuration)
            {
                // 转换为绿色状态
                mBossState = BossState::Green;
                mBossStateStartTime = now;
                
                // 生成新的攻击点
                updateBossAttackPoint();
                
                // 显示提示信息
                mvwprintw(this->mWindows[0], 3, 1, "Attack the Boss now!        ");
                wrefresh(this->mWindows[0]);
            }
            break;
            
        case BossState::Green:
            // 绿色状态持续3秒，如果没有被攻击，转换回红色状态
            if (mBossStateDuration >= mGreenStateDuration)
            {
                // 转换为红色状态
                mBossState = BossState::Red;
                mBossStateStartTime = now;
                
                // 显示提示信息
                mvwprintw(this->mWindows[0], 3, 1, "Avoid the lasers!           ");
                wrefresh(this->mWindows[0]);
            }
            break;
    }
}

// 渲染Boss
void Game::renderBoss()
{
    int startX = mBossPosition.first;
    int startY = mBossPosition.second;
    
    // 根据Boss的状态选择不同的字符表示
    char bossSymbol;
    
    switch (mBossState)
    {
        case BossState::Red:
            bossSymbol = 'R';
            break;
        case BossState::Green:
            bossSymbol = 'G';
            break;
        default:
            bossSymbol = 'B';
            break;
    }
    
    // 渲染Boss的方形区域
    for (int y = 0; y < mBossSize; y++)
    {
        for (int x = 0; x < mBossSize; x++)
        {
            mvwaddch(this->mWindows[1], startY + y, startX + x, bossSymbol);
        }
    }
    
    // 如果是绿色状态，显示攻击点
    if (mBossState == BossState::Green)
    {
        // 用特殊符号标记攻击点
        mvwaddch(this->mWindows[1], mBossAttackPoint.getY(), mBossAttackPoint.getX(), '@');
    }
}

// 更新并渲染激光
void Game::updateAndRenderLasers()
{
    // 更新激光旋转角度
    mLaserAngle += mLaserRotationSpeed;
    if (mLaserAngle >= 360.0)
    {
        mLaserAngle -= 360.0;
    }
    
    // 计算激光起点（Boss中心）
    int centerX = mBossPosition.first + mBossSize / 2;
    int centerY = mBossPosition.second + mBossSize / 2;
    
    // 生成多条激光，均匀分布在360度范围内
    const int laserCount = 4; // 减少为4条激光
    for (int i = 0; i < laserCount; i++)
    {
        double angle = mLaserAngle + (360.0 / laserCount) * i;
        double radians = angle * M_PI / 180.0;
        int endX = static_cast<int>(centerX + mLaserLength * cos(radians));
        int endY = static_cast<int>(centerY + mLaserLength * sin(radians));
        
        renderLaser(centerX, centerY, endX, endY, mWallSymbol);
    }
}

// 渲染单个激光
void Game::renderLaser(int x1, int y1, int x2, int y2, char symbol)
{
    // 使用Bresenham算法绘制线段
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    // 添加用于创建间隙的计数器
    int gapCounter = 0;
    const int gapInterval = 4; // 每4个单位创建一个间隙 (原来是5)
    const int gapSize = 3;    // 间隙大小为3个单位 (原来是2)
    
    while (true)
    {
        // 增加计数器
        gapCounter++;
        
        // 如果不在间隙内（创建有规律的间隙）
        if (gapCounter % (gapInterval + gapSize) >= gapSize)
        {
            // 如果点在游戏区域内，则绘制
            if (x1 >= 0 && x1 <= mGameBoardWidth - 1 && y1 >= 0 && y1 <= mGameBoardHeight - 1)
            {
                // 不在Boss区域内才绘制激光
                if (!(x1 >= mBossPosition.first && x1 < mBossPosition.first + mBossSize &&
                      y1 >= mBossPosition.second && y1 < mBossPosition.second + mBossSize))
                {
                    // 不在墙上才绘制激光
                    if (!mPtrMap->isWall(x1, y1))
                    {
                        // 使用墙的符号来渲染激光
                        mvwaddch(this->mWindows[1], y1, x1, this->mWallSymbol);
                    }
                }
            }
        }
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

// 检查蛇是否碰到激光
bool Game::checkLaserCollision()
{
    // 如果蛇处于无敌状态，不会受到激光伤害
    if (mSnakeInvincible)
    {
        return false;
    }
    
    // 只获取蛇头，不再检查整个蛇身
    const SnakeBody& head = this->mPtrSnake->getSnake()[0];
    
    // 计算激光起点（Boss中心）
    int centerX = mBossPosition.first + mBossSize / 2;
    int centerY = mBossPosition.second + mBossSize / 2;
    
    // 检查多条激光的碰撞
    const int laserCount = 4; // 与updateAndRenderLasers中相同
    for (int i = 0; i < laserCount; i++)
    {
        double angle = mLaserAngle + (360.0 / laserCount) * i;
        double radians = angle * M_PI / 180.0;
        int endX = static_cast<int>(centerX + mLaserLength * cos(radians));
        int endY = static_cast<int>(centerY + mLaserLength * sin(radians));
        
        if (checkSnakeLaserCollision({head}, centerX, centerY, endX, endY))
        {
            return true;
        }
    }
    
    return false;
}

// 检查蛇与单个激光的碰撞
bool Game::checkSnakeLaserCollision(const std::vector<SnakeBody>& snake, int x1, int y1, int x2, int y2)
{
    // 定义与renderLaser相同的间隙参数
    const int gapInterval = 4; // 每4个单位创建一个间隙 (原来是5)
    const int gapSize = 3;    // 间隙大小为3个单位 (原来是2)
    
    // 对蛇的每个部分进行检查
    for (const auto& body : snake)
    {
        int snakeX = body.getX();
        int snakeY = body.getY();
        
        // 检查蛇是否在墙上，如果在则不会碰撞（与renderLaser保持一致）
        if (mPtrMap != nullptr && mPtrMap->isWall(snakeX, snakeY))
        {
            continue;
        }
        
        // 检查蛇是否在游戏区域内，如果不在则不会碰撞
        if (snakeX < 0 || snakeX > mGameBoardWidth - 1 ||
            snakeY < 0 || snakeY > mGameBoardHeight - 1)
        {
            continue;
        }
        
        // 检查蛇是否在Boss区域内，如果在则不会碰撞（与renderLaser保持一致）
        if (snakeX >= mBossPosition.first && snakeX < mBossPosition.first + mBossSize &&
            snakeY >= mBossPosition.second && snakeY < mBossPosition.second + mBossSize)
        {
            continue;
        }
        
        // 使用更精确的网格判定方式，检查蛇是否与激光在同一个格子
        // 计算激光路径上的所有格子
        std::vector<std::pair<int, int>> laserCells;
        int x = x1, y = y1;
        const int dx = abs(x2 - x1);
        const int dy = abs(y2 - y1);
        const int sx = (x1 < x2) ? 1 : -1;
        const int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;
        int gapCounter = 0;
        
        while (true)
        {
            // 增加计数器
            gapCounter++;
            
            // 如果不在间隙内（创建有规律的间隙）
            if (gapCounter % (gapInterval + gapSize) >= gapSize)
            {
                // 如果点在游戏区域内且不在墙上，添加到激光路径
                if (x >= 0 && x <= mGameBoardWidth - 1 && y >= 0 && y <= mGameBoardHeight - 1)
                {
                    // 不在Boss区域内且不在墙上才算作激光路径
                    if (!(x >= mBossPosition.first && x < mBossPosition.first + mBossSize &&
                          y >= mBossPosition.second && y < mBossPosition.second + mBossSize) &&
                        !mPtrMap->isWall(x, y))
                    {
                        laserCells.emplace_back(x, y);
                    }
                }
            }
            
            if (x == x2 && y == y2) break;
            
            int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y += sy;
            }
        }
        
        // 检查蛇是否在激光路径上的格子中
        for (const auto& cell : laserCells)
        {
            if (snakeX == cell.first && snakeY == cell.second)
            {
                return true;
            }
        }
    }
    
    return false;
}

// 计算点到线段的距离
double Game::pointToLineDistance(int x0, int y0, int x1, int y1, int x2, int y2)
{
    // 计算线段长度的平方
    double lineLength2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    
    // 如果线段长度为0，则返回点到端点的距离
    if (lineLength2 == 0)
    {
        return sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1));
    }
    
    // 计算投影比例
    double t = ((x0 - x1) * (x2 - x1) + (y0 - y1) * (y2 - y1)) / lineLength2;
    
    // 限制t在[0,1]范围内，即投影点在线段上
    t = std::max(0.0, std::min(1.0, t));
    
    // 计算投影点坐标
    double projX = x1 + t * (x2 - x1);
    double projY = y1 + t * (y2 - y1);
    
    // 计算点到投影点的距离
    return sqrt((x0 - projX) * (x0 - projX) + (y0 - projY) * (y0 - projY));
}

// 检查蛇是否攻击到Boss
bool Game::checkBossAttack()
{
    // 只有在Boss处于绿色状态时才可以被攻击
    if (mBossState != BossState::Green)
    {
        return false;
    }
    
    // 获取蛇的头部
    const SnakeBody& head = this->mPtrSnake->getSnake()[0];
    int headX = head.getX();
    int headY = head.getY();
    
    // 检查蛇头是否接触到Boss攻击点
    return (headX == mBossAttackPoint.getX() && headY == mBossAttackPoint.getY());
}

// 更新Boss攻击点位置
void Game::updateBossAttackPoint()
{
    // 在Boss区域内随机选择一点
    int offsetX = std::rand() % mBossSize;
    int offsetY = std::rand() % mBossSize;
    
    // 更新攻击点位置
    mBossAttackPoint = SnakeBody(mBossPosition.first + offsetX, mBossPosition.second + offsetY);
}

bool Game::selectBattleType() {
    clear();
    refresh();
    WINDOW * menu;
    int width = this->mGameBoardWidth * 0.6;
    int height = this->mGameBoardHeight * 0.6;
    int startX = this->mGameBoardWidth * 0.2;
    int startY = this->mGameBoardHeight * 0.2 + this->mInformationHeight;

    menu = newwin(height, width, startY, startX);
    box(menu, 0, 0);

    std::vector<std::string> menuItems = {"Player vs Player", "Player vs AI", "Back"};
    int index = 0;
    int offset = 3;
    mvwprintw(menu, 1, 1, "Select Battle Type:");
    wattron(menu, A_STANDOUT);
    mvwprintw(menu, 0 + offset, 1, menuItems[0].c_str());
    wattroff(menu, A_STANDOUT);
    for (size_t i = 1; i < menuItems.size(); i++) {
        mvwprintw(menu, i + offset, 1, menuItems[i].c_str());
    }
    wrefresh(menu);

    int key;
    while (true) {
        key = getch();
        switch(key) {
            case 'W': case 'w': case KEY_UP:
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index = (index - 1 + menuItems.size()) % menuItems.size();
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            case 'S': case 's': case KEY_DOWN:
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index = (index + 1) % menuItems.size();
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
        }
        wrefresh(menu);
        if (key == ' ' || key == 10) break;
    }
    delwin(menu);

    if (index == 0) mCurrentBattleType = BattleType::PlayerVsPlayer;
    else if (index == 1) mCurrentBattleType = BattleType::PlayerVsAI;
    else return false; // 用户选择 "Back"

    return true;
}


void Game::initializeBattle(BattleType type) {
    mPtrMap = std::make_unique<Map>(mGameBoardWidth, mGameBoardHeight);
    mPtrMap->initializeEmptyMap(); // 对战使用简单的开放地图

    mPtrSnake.reset(new Snake(mGameBoardWidth, mGameBoardHeight, mInitialSnakeLength));
    mPtrSnake2.reset(new Snake(mGameBoardWidth, mGameBoardHeight, mInitialSnakeLength));

    // 设置生命值
    mPtrSnake->setLives(mPlayerLives);
    mPtrSnake2->setLives(mPlayer2Lives);

    // 将蛇放置在相对的角落，都向右移动
    mPtrSnake->initializeSnake(5, 5, InitialDirection::Right);
    mPtrSnake2->initializeSnake(mGameBoardWidth - 10, mGameBoardHeight - 10, InitialDirection::Right);

    mPtrSnake->setMap(mPtrMap.get());
    mPtrSnake2->setMap(mPtrMap.get());

    // 同步尸体食物信息（初始为空）
    mPtrSnake->senseCorpseFoods(mCorpseFoods);
    mPtrSnake2->senseCorpseFoods(mCorpseFoods);

    // 创建食物（在蛇设置地图之后）
    createRamdonFood();
    
    // 确保食物被正确创建，如果没有可用位置，创建一个默认食物
    if (mFood.getX() == 0 && mFood.getY() == 0) {
        // 在中心位置创建一个食物
        mFood = SnakeBody(mGameBoardWidth / 2, mGameBoardHeight / 2);
    }
    
    // 创建特殊食物或毒药（100%概率生成，battle mode专用）
    int specialRand = std::rand() % 100;
    if (specialRand < 70) {
        // 70%概率生成特殊食物
        this->createSpecialFood();
        this->mPtrSnake->senseSpecialFood(this->mSpecialFood);
        this->mPtrSnake2->senseSpecialFood(this->mSpecialFood);
    } else {
        // 30%概率生成毒药
        this->createPoison();
        this->mPtrSnake->sensePoison(this->mPoison);
        this->mPtrSnake2->sensePoison(this->mPoison);
    }
    
    // Battle mode不生成随机道具
    mHasRandomItem = false;
    
    // 清空尸体食物列表
    mCorpseFoods.clear();
    
    mPoints = 0;
    mPoints2 = 0;
    mDelay = mBaseDelay;
    mBattleBaseDelay = 150; // 初始化对战模式基础延迟
    mAccelerating = false; // 重置加速状态
    mLastKeyDirection = Direction::Right; // 重置按键方向
    mLastKeyPressTime = std::chrono::steady_clock::now(); // 重置按键时间
}

void Game::runBattle() {
    std::string winner = "";
    nodelay(stdscr, TRUE); // Set getch() to be non-blocking
    
    while (winner.empty()) {
        int key = getch();
        if (key != ERR) {
             controlSnakes(key); // 处理玩家输入
        }

        // 如果是 AI 对战模式，获取 AI 的下一步移动方向
        if (mCurrentBattleType == BattleType::PlayerVsAI) {
            Direction ai_dir = mPtrAI->findNextMove(*mPtrMap, *mPtrSnake, *mPtrSnake2, 
                                                   mFood, mSpecialFood, mPoison, mRandomItem,
                                                   mCurrentFoodType, mHasSpecialFood, mHasPoison, mHasRandomItem);
            mPtrSnake2->changeDirection(ai_dir);
        }

        // 更新作弊模式状态
        updateCheatMode();
        
        werase(mWindows[1]);
        box(mWindows[1], 0, 0);
        renderMap();

        // 渲染蛇和食物（在移动之前）
        renderSnakes();
        renderFood();
        renderPoison();
        renderSpecialFood();
        renderCorpseFoods();
        renderBattleStatus();

        // 同步食物信息并移动两条蛇
        mPtrSnake->senseFood(mFood);
        mPtrSnake2->senseFood(mFood);
        
        // 检查两条蛇是否都会到达食物位置
        bool p1_will_eat = mPtrSnake->touchFood();
        bool p2_will_eat = mPtrSnake2->touchFood();
        
        // 移动蛇
        bool p1_ate = mPtrSnake->moveFoward();
        bool p2_ate = mPtrSnake2->moveFoward();
        
        // 如果两条蛇都会到达食物位置，确保它们都增长
        if (p1_will_eat && p2_will_eat && !p1_ate && !p2_ate) {
            // 两条蛇都应该增长，但moveFoward可能没有正确处理
            // 手动让第二条蛇增长
            auto& snake2 = mPtrSnake2->getSnake();
            if (!snake2.empty()) {
                snake2.push_back(snake2.back()); // 复制尾部增加长度
            }
            p2_ate = true; // 标记为已吃食物
        }
        
        // 检测特殊食物、毒药、尸体食物和随机道具碰撞
        bool p1_ate_special = mPtrSnake->touchSpecialFood();
        bool p2_ate_special = mPtrSnake2->touchSpecialFood();
        bool p1_ate_poison = mPtrSnake->touchPoison();
        bool p2_ate_poison = mPtrSnake2->touchPoison();
        bool p1_ate_corpse = mPtrSnake->touchCorpseFood();
        bool p2_ate_corpse = mPtrSnake2->touchCorpseFood();
        bool p1_ate_random = mPtrSnake->touchRandomItem();
        bool p2_ate_random = mPtrSnake2->touchRandomItem();

        // 检查碰撞
        winner = checkBattleCollisions();
        if (!winner.empty()) {
            wrefresh(mWindows[1]);
            break; // 如果有胜负，跳出循环
        }
        
        // 处理碰撞后的重置（如果蛇还活着但发生了碰撞）
        if (mPtrSnake->checkCollision() || mPtrSnake2->isPartOfSnake(mPtrSnake->getSnake().front().getX(), mPtrSnake->getSnake().front().getY())) {
            if (mPtrSnake->isAlive()) {
                // 在蛇死亡时，将蛇的尸体转换为食物
                const std::vector<SnakeBody>& snakeBody = mPtrSnake->getSnake();
                this->createCorpseFoods(snakeBody);
                this->mPtrSnake->senseCorpseFoods(this->mCorpseFoods);
                this->mPtrSnake2->senseCorpseFoods(this->mCorpseFoods);
                
                // 重置玩家1蛇的位置
                mPtrSnake->initializeSnake(5, 5, InitialDirection::Right);
                mPtrSnake->setLives(mPtrSnake->getLives()); // 保持当前生命值
            }
        }
        
        if (mPtrSnake2->checkCollision() || mPtrSnake->isPartOfSnake(mPtrSnake2->getSnake().front().getX(), mPtrSnake2->getSnake().front().getY())) {
            if (mPtrSnake2->isAlive()) {
                // 在蛇死亡时，将蛇的尸体转换为食物
                const std::vector<SnakeBody>& snakeBody = mPtrSnake2->getSnake();
                this->createCorpseFoods(snakeBody);
                this->mPtrSnake->senseCorpseFoods(this->mCorpseFoods);
                this->mPtrSnake2->senseCorpseFoods(this->mCorpseFoods);
                
                // 重置玩家2/AI蛇的位置
                mPtrSnake2->initializeSnake(mGameBoardWidth - 10, mGameBoardHeight - 10, InitialDirection::Right);
                mPtrSnake2->setLives(mPtrSnake2->getLives()); // 保持当前生命值
            }
        }

        // 处理吃食物
        if (p1_ate || p2_ate) {
            if (p1_ate) { mPoints++; addCoins(1); }
            if (p2_ate) mPoints2++;
            

            
            createRamdonFood();
            
            // 重新生成特殊食物或毒药（100%概率生成）
            int specialRand = std::rand() % 100;
            if (specialRand < 70) {
                this->createSpecialFood();
                this->mPtrSnake->senseSpecialFood(this->mSpecialFood);
                this->mPtrSnake2->senseSpecialFood(this->mSpecialFood);
            } else {
                this->createPoison();
                this->mPtrSnake->sensePoison(this->mPoison);
                this->mPtrSnake2->sensePoison(this->mPoison);
            }
            
            // 同步尸体食物信息
            this->mPtrSnake->senseCorpseFoods(this->mCorpseFoods);
            this->mPtrSnake2->senseCorpseFoods(this->mCorpseFoods);
            
            // 重新生成随机道具（有10%概率）
            if (std::rand() % 100 < 10) {
                this->createRandomItem();
                this->mPtrSnake->senseRandomItem(this->mRandomItem);
                this->mPtrSnake2->senseRandomItem(this->mRandomItem);
            } else {
                mHasRandomItem = false;
            }
        }
        
        // 处理特殊食物效果（蛇长度变化后调整延迟）
        if (p1_ate_special && mHasSpecialFood) {
            int effect = getFoodEffect(mCurrentFoodType);
            if (effect > 0) {
                // 正效果：增加长度和点数
                for (int i = 0; i < effect; i++) {
                    auto& snake = this->mPtrSnake->getSnake();
                    if (!snake.empty()) {
                        snake.push_back(snake.back()); // 复制尾部增加长度
                    }
                }
                this->mPoints += effect;
                addCoins(effect); // 增加金币
            }
            mHasSpecialFood = false;
            // 蛇长度变化后调整延迟
            adjustBattleDelay();
        }
        if (p2_ate_special && mHasSpecialFood) {
            int effect = getFoodEffect(mCurrentFoodType);
            if (effect > 0) {
                // 正效果：增加长度和点数
                for (int i = 0; i < effect; i++) {
                    auto& snake = this->mPtrSnake2->getSnake();
                    if (!snake.empty()) {
                        snake.push_back(snake.back()); // 复制尾部增加长度
                    }
                }
                this->mPoints2 += effect;
            }
            mHasSpecialFood = false;
            // 蛇长度变化后调整延迟
            adjustBattleDelay();
        }
        
        // 处理毒药效果（蛇长度变化后调整延迟）
        if (p1_ate_poison && mHasPoison) {
            int effect = getFoodEffect(FoodType::Poison);
            if (effect < 0) {
                // 负效果：减少长度
                auto& snake = this->mPtrSnake->getSnake();
                for (int i = 0; i < -effect && snake.size() > 1; i++) {
                    snake.pop_back(); // 减少长度
                }
            }
            mHasPoison = false;
            // 蛇长度变化后调整延迟
            adjustBattleDelay();
        }
        if (p2_ate_poison && mHasPoison) {
            int effect = getFoodEffect(FoodType::Poison);
            if (effect < 0) {
                // 负效果：减少长度
                auto& snake = this->mPtrSnake2->getSnake();
                for (int i = 0; i < -effect && snake.size() > 1; i++) {
                    snake.pop_back(); // 减少长度
                }
            }
            mHasPoison = false;
            // 蛇长度变化后调整延迟
            adjustBattleDelay();
        }
        

        
        // 处理尸体食物效果
        if (p1_ate_corpse && !mCorpseFoods.empty()) {
            // 处理尸体食物效果
            int effect = getFoodEffect(FoodType::Normal); // 尸体食物按普通食物处理
            if (effect > 0) {
                // 正效果：增加长度和点数
                for (int i = 0; i < effect; i++) {
                    auto& snake = this->mPtrSnake->getSnake();
                    if (!snake.empty()) {
                        snake.push_back(snake.back()); // 复制尾部增加长度
                    }
                }
                this->mPoints += effect;
                addCoins(effect); // 增加金币
            }
            // 移除被吃掉的尸体食物
            SnakeBody eatenCorpse = this->mPtrSnake->getEatenCorpseFood();
            if (eatenCorpse.getX() != -1 && eatenCorpse.getY() != -1) {
                mCorpseFoods.erase(
                    std::remove_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                        [&eatenCorpse](const SnakeBody& corpse) {
                            return corpse.getX() == eatenCorpse.getX() && corpse.getY() == eatenCorpse.getY();
                        }),
                    mCorpseFoods.end()
                );
                // 更新蛇感知的尸体食物列表
                this->mPtrSnake->senseCorpseFoods(this->mCorpseFoods);
                this->mPtrSnake2->senseCorpseFoods(this->mCorpseFoods);
            }
            // 蛇长度变化后调整延迟
            adjustBattleDelay();
        }
        if (p2_ate_corpse && !mCorpseFoods.empty()) {
            // 处理尸体食物效果
            int effect = getFoodEffect(FoodType::Normal); // 尸体食物按普通食物处理
            if (effect > 0) {
                // 正效果：增加长度和点数
                for (int i = 0; i < effect; i++) {
                    auto& snake = this->mPtrSnake2->getSnake();
                    if (!snake.empty()) {
                        snake.push_back(snake.back()); // 复制尾部增加长度
                    }
                }
                this->mPoints2 += effect;
            }
            // 移除被吃掉的尸体食物
            SnakeBody eatenCorpse = this->mPtrSnake2->getEatenCorpseFood();
            if (eatenCorpse.getX() != -1 && eatenCorpse.getY() != -1) {
                mCorpseFoods.erase(
                    std::remove_if(mCorpseFoods.begin(), mCorpseFoods.end(),
                        [&eatenCorpse](const SnakeBody& corpse) {
                            return corpse.getX() == eatenCorpse.getX() && corpse.getY() == eatenCorpse.getY();
                        }),
                    mCorpseFoods.end()
                );
                // 更新蛇感知的尸体食物列表
                this->mPtrSnake->senseCorpseFoods(this->mCorpseFoods);
                this->mPtrSnake2->senseCorpseFoods(this->mCorpseFoods);
            }
            // 蛇长度变化后调整延迟
            adjustBattleDelay();
        }
        
        // 对战模式中不处理随机道具效果（禁用道具功能）
        if (p1_ate_random && mHasRandomItem) {
            // 对战模式中不添加到库存，直接消失
            mHasRandomItem = false;
        }
        if (p2_ate_random && mHasRandomItem) {
            // AI也不获得道具
            mHasRandomItem = false;
        }
        
        // 检查特殊食物/毒药/道具是否超时消失
        auto now = std::chrono::steady_clock::now();
        if (mHasSpecialFood && std::chrono::duration_cast<std::chrono::seconds>(now - mSpecialFoodSpawnTime).count() > mSpecialFoodDuration) {
            mHasSpecialFood = false;
        }
        if (mHasPoison && std::chrono::duration_cast<std::chrono::seconds>(now - mPoisonSpawnTime).count() > mPoisonDuration) {
            mHasPoison = false;
        }
        if (mHasRandomItem && std::chrono::duration_cast<std::chrono::seconds>(now - mRandomItemSpawnTime).count() > mRandomItemDuration) {
            mHasRandomItem = false;
        }

        // 实现加速逻辑
        long currentDelay = mBattleBaseDelay; // 使用对战模式专用延迟
        if (mAccelerating) {
            // 对战模式加速：基础延迟的40%
            currentDelay = static_cast<long>(mBattleBaseDelay * 0.4);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(currentDelay));
        wrefresh(mWindows[1]);
    }
    nodelay(stdscr, FALSE);
    renderWinnerText(winner); // 显示胜利者
}

void Game::controlSnakes(int key) {
    // Battle mode中禁用道具使用
    // handleItemUsage(key); // 注释掉道具使用

    // 处理长按加速
    handleAcceleration(key);

    if (mCurrentBattleType == BattleType::PlayerVsAI) {
        // 玩家1用方向键
        switch(key) {
            case KEY_UP:    mPtrSnake->changeDirection(Direction::Up); break;
            case KEY_DOWN:  mPtrSnake->changeDirection(Direction::Down); break;
            case KEY_LEFT:  mPtrSnake->changeDirection(Direction::Left); break;
            case KEY_RIGHT: mPtrSnake->changeDirection(Direction::Right); break;
        }
    } else if (mCurrentBattleType == BattleType::PlayerVsPlayer) {
        // 玩家1用WASD，玩家2用方向键
        switch(key) {
            case 'W': case 'w': mPtrSnake->changeDirection(Direction::Up); break;
            case 'S': case 's': mPtrSnake->changeDirection(Direction::Down); break;
            case 'A': case 'a': mPtrSnake->changeDirection(Direction::Left); break;
            case 'D': case 'd': mPtrSnake->changeDirection(Direction::Right); break;
        }
        switch(key) {
            case KEY_UP:    mPtrSnake2->changeDirection(Direction::Up); break;
            case KEY_DOWN:  mPtrSnake2->changeDirection(Direction::Down); break;
            case KEY_LEFT:  mPtrSnake2->changeDirection(Direction::Left); break;
            case KEY_RIGHT: mPtrSnake2->changeDirection(Direction::Right); break;
        }
    }
}

std::string Game::checkBattleCollisions() {
    const auto& head1 = mPtrSnake->getSnake().front();
    const auto& head2 = mPtrSnake2->getSnake().front();

    // 检查蛇1是否碰撞（撞墙、撞自己、撞蛇2身体）
    bool p1_collision = mPtrSnake->checkCollision() || mPtrSnake2->isPartOfSnake(head1.getX(), head1.getY());

    // 检查蛇2是否碰撞（撞墙、撞自己、撞蛇1身体）
    bool p2_collision = mPtrSnake2->checkCollision() || mPtrSnake->isPartOfSnake(head2.getX(), head2.getY());

    // 特殊情况：头对头碰撞
    if (head1 == head2) {
        p1_collision = true;
        p2_collision = true;
    }

    // 处理碰撞，减少生命值
    bool p1_died = false;
    bool p2_died = false;
    
    if (p1_collision && mPtrSnake->isAlive()) {
        if (!mPtrSnake->loseLife()) {
            p1_died = true; // 生命值归零，真正死亡
        }
    }

    if (p2_collision && mPtrSnake2->isAlive()) {
        if (!mPtrSnake2->loseLife()) {
            p2_died = true; // 生命值归零，真正死亡
        }
    }

    // 判断胜负
    if (p1_died && p2_died) {
        return "Draw!"; // 双方同时死亡
    } else if (p1_died) {
        return (mCurrentBattleType == BattleType::PlayerVsAI) ? "AI Wins!" : "Player 2 Wins!";
    } else if (p2_died) {
        return "Player 1 Wins!";
    }

    return ""; // 没有真正死亡
}

void Game::renderSnakes() const {
    if (mPtrSnake) {
        short color_pair = 1;
        switch (mCurrentSkin) {
            case SnakeSkin::Default: color_pair = 1; break;
            case SnakeSkin::Red:     color_pair = 4; break;
            case SnakeSkin::Blue:    color_pair = 5; break;
            case SnakeSkin::Green:   color_pair = 6; break;
            case SnakeSkin::Yellow:  color_pair = 2; break;
        }
        wattron(mWindows[1], COLOR_PAIR(color_pair));
        for (const auto& part : mPtrSnake->getSnake()) {
            mvwaddch(mWindows[1], part.getY(), part.getX(), mSnakeSymbol);
        }
        wattroff(mWindows[1], COLOR_PAIR(color_pair));
    }
    if (mPtrSnake2) {
        wattron(mWindows[1], COLOR_PAIR(2)); // 蛇2依然用黄色
        for (const auto& part : mPtrSnake2->getSnake()) {
            mvwaddch(mWindows[1], part.getY(), part.getX(), mSnakeSymbol2);
        }
        wattroff(mWindows[1], COLOR_PAIR(2));
    }
}

void Game::renderBattleStatus() const {
    for(int i = 1; i < 15; ++i) mvwprintw(mWindows[2], i, 1, "                     ");

    mvwprintw(mWindows[2], 1, 1, "Battle Mode");

    wattron(mWindows[2], COLOR_PAIR(1));
    mvwprintw(mWindows[2], 3, 1, "Player 1 (Arrows)");
    wattroff(mWindows[2], COLOR_PAIR(1));
    mvwprintw(mWindows[2], 4, 1, "Points: %d", mPoints);
    mvwprintw(mWindows[2], 5, 1, "Lives: %d", mPtrSnake ? mPtrSnake->getLives() : mPlayerLives);

    wattron(mWindows[2], COLOR_PAIR(2));
    if (mCurrentBattleType == BattleType::PlayerVsPlayer) {
        mvwprintw(mWindows[2], 7, 1, "Player 2 (WASD)");
    } else {
        mvwprintw(mWindows[2], 7, 1, "AI Player");
    }
    wattroff(mWindows[2], COLOR_PAIR(2));
    mvwprintw(mWindows[2], 8, 1, "Points: %d", mPoints2);
    mvwprintw(mWindows[2], 9, 1, "Lives: %d", mPtrSnake2 ? mPtrSnake2->getLives() : mPlayer2Lives);
    
    // Battle mode中禁用道具，不显示道具说明
    // mvwprintw(mWindows[2], 9, 1, "Items: C-Cheat P-Portal");
    // if (mCurrentMode == GameMode::Battle) {
    //     mvwprintw(mWindows[2], 10, 1, "X-Attack (Battle Only)");
    // }

    wrefresh(mWindows[2]);
}

void Game::renderWinnerText(const std::string& winner) const {
    WINDOW * menu;
    int width = this->mGameBoardWidth * 0.5;
    int height = 5;
    int startX = this->mGameBoardWidth * 0.25;
    int startY = this->mGameBoardHeight * 0.25 + this->mInformationHeight;

    menu = newwin(height, width, startY, startX);
    box(menu, 0, 0);

    mvwprintw(menu, 1, (width - 10) / 2, "Game Over!");
    mvwprintw(menu, 2, (width - winner.length()) / 2, winner.c_str());
    mvwprintw(menu, 3, (width - 26) / 2, "Press any key to continue...");
    wrefresh(menu);

    nodelay(stdscr, FALSE); // 切换为阻塞模式，等待按键
    getch();
    nodelay(stdscr, TRUE); // 恢复非阻塞模式
    delwin(menu);
}

// ====== 商店和皮肤持久化 ======
const std::string PLAYER_PROFILE_FILE = "player_profile.dat";

void Game::savePlayerProfile() const {
    std::ofstream ofs(PLAYER_PROFILE_FILE, std::ios::binary);
    if (!ofs) return;
    ofs.write(reinterpret_cast<const char*>(&mCoins), sizeof(mCoins));
    int skin = static_cast<int>(mCurrentSkin);
    ofs.write(reinterpret_cast<const char*>(&skin), sizeof(skin));
    int ownedCount = mOwnedSkins.size();
    ofs.write(reinterpret_cast<const char*>(&ownedCount), sizeof(ownedCount));
    for (auto s : mOwnedSkins) {
        int sval = static_cast<int>(s);
        ofs.write(reinterpret_cast<const char*>(&sval), sizeof(sval));
    }
    ofs.close();
}

void Game::loadPlayerProfile() {
    std::ifstream ifs(PLAYER_PROFILE_FILE, std::ios::binary);
    if (!ifs) return;
    mOwnedSkins.clear();
    ifs.read(reinterpret_cast<char*>(&mCoins), sizeof(mCoins));
    int skin = 0;
    ifs.read(reinterpret_cast<char*>(&skin), sizeof(skin));
    mCurrentSkin = static_cast<SnakeSkin>(skin);
    int ownedCount = 0;
    ifs.read(reinterpret_cast<char*>(&ownedCount), sizeof(ownedCount));
    for (int i = 0; i < ownedCount; ++i) {
        int sval = 0;
        ifs.read(reinterpret_cast<char*>(&sval), sizeof(sval));
        mOwnedSkins.insert(static_cast<SnakeSkin>(sval));
    }
    ifs.close();
    // 确保基础皮肤一定拥有
    mOwnedSkins.insert(SnakeSkin::Default);
}

void Game::setSnakeSkin(SnakeSkin skin) { mCurrentSkin = skin; }
SnakeSkin Game::getSnakeSkin() const { return mCurrentSkin; }
int Game::getCoins() const { return mCoins; }
void Game::addCoins(int amount) { mCoins += amount; }
bool Game::buySkin(SnakeSkin skin, int price) {
    if (mOwnedSkins.count(skin)) return false;
    if (mCoins < price) return false;
    mCoins -= price;
    mOwnedSkins.insert(skin);
    return true;
}
bool Game::hasSkin(SnakeSkin skin) const { return mOwnedSkins.count(skin) > 0; }

void Game::showShopMenu() {
    // 保存当前屏幕状态
    clear();
    refresh();
    
    while (true) {
        clear();
        refresh();
        
        // 商店主菜单
        std::vector<std::string> shopMenu = {"Skin Shop", "Item Shop", "Return"};
        int index = 0;
        
        // 居中显示
        int startY = (LINES - 8) / 2;
        int startX = (COLS - 20) / 2;
        
        // 标题
        mvprintw(startY, startX, "==== SNAKE SHOP ====");
        
        // 金币信息
        mvprintw(startY + 2, startX, "Coins: %d", mCoins);
        
        // 操作说明
        mvprintw(startY + 3, startX, "Up/Down: Select   Enter: Confirm");
        
        // 菜单选项
        for (size_t i = 0; i < shopMenu.size(); ++i) {
            if ((int)i == index) attron(A_REVERSE);
            mvprintw(startY + 5 + i, startX, "%s", shopMenu[i].c_str());
            if ((int)i == index) attroff(A_REVERSE);
        }
        
        refresh();
        
        int key;
        while (true) {
            key = getch();
            if (key == 'w' || key == 'W' || key == KEY_UP) {
                mvprintw(startY + 5 + index, startX, "%s", shopMenu[index].c_str());
                index = (index - 1 + shopMenu.size()) % shopMenu.size();
                attron(A_REVERSE);
                mvprintw(startY + 5 + index, startX, "%s", shopMenu[index].c_str());
                attroff(A_REVERSE);
                refresh();
            } else if (key == 's' || key == 'S' || key == KEY_DOWN) {
                mvprintw(startY + 5 + index, startX, "%s", shopMenu[index].c_str());
                index = (index + 1) % shopMenu.size();
                attron(A_REVERSE);
                mvprintw(startY + 5 + index, startX, "%s", shopMenu[index].c_str());
                attroff(A_REVERSE);
                refresh();
            }
            if (key == ' ' || key == 10) break;
        }
        
        if (index == 0) {
            // 皮肤商店
            showShopMenu_Skin();
        } else if (index == 1) {
            // 道具商店
            showShopMenu_Item();
        } else {
            // 返回
            break;
        }
    }
    
    // 清理屏幕并刷新
    clear();
    refresh();
}

// 拆分皮肤商店和道具商店子菜单
void Game::showShopMenu_Skin() {
    struct SkinInfo {
        SnakeSkin skin;
        std::string name;
        int price;
        short color_pair;
    };
    std::vector<SkinInfo> skins = {
        {SnakeSkin::Default, "Cyan (Default)", 0, 1},
        {SnakeSkin::Red,     "Red", 10, 4},
        {SnakeSkin::Blue,    "Blue", 10, 5},
        {SnakeSkin::Green,   "Green", 10, 6},
        {SnakeSkin::Yellow,  "Yellow", 10, 2}
    };
    
    int selected = 0;
    bool inShop = true;
    bool needRedraw = true;
    
    while (inShop) {
        if (needRedraw) {
            clear();
            refresh();
            
            // 居中显示
            int startY = (LINES - 12) / 2;
            int startX = (COLS - 50) / 2;
            
            // 标题
            mvprintw(startY, startX, "==== SKIN SHOP ====");
            
            // 金币信息
            mvprintw(startY + 2, startX, "Coins: %d", mCoins);
            
            // 操作说明
            mvprintw(startY + 3, startX, "Up/Down: Select   Enter: Buy/Use   Q: Exit");
            
            // 皮肤列表
            int row = startY + 5;
            for (size_t i = 0; i < skins.size(); ++i) {
                bool owned = hasSkin(skins[i].skin);
                bool current = (mCurrentSkin == skins[i].skin);
                
                if ((int)i == selected) attron(A_REVERSE);
                
                // 选择指示器和皮肤名称
                mvprintw(row, startX, "%s ", (current ? ">" : " "));
                attron(COLOR_PAIR(skins[i].color_pair));
                mvprintw(row, startX + 2, "%-15s", skins[i].name.c_str());
                attroff(COLOR_PAIR(skins[i].color_pair));
                
                // 价格
                mvprintw(row, startX + 18, "Price: %-3d", skins[i].price);
                
                // 状态信息
                if (owned) {
                    mvprintw(row, startX + 28, "[Owned]");
                } else if (skins[i].price == 0) {
                    mvprintw(row, startX + 28, "[Free] ");
                } else if (mCoins >= skins[i].price) {
                    mvprintw(row, startX + 28, "[Buy]  ");
                } else {
                    mvprintw(row, startX + 28, "[No$] ");
                }
                
                // 当前使用状态
                if (current) {
                    mvprintw(row, startX + 36, "[Using]");
                } else if (owned) {
                    mvprintw(row, startX + 36, "[Switch]");
                }
                
                if ((int)i == selected) attroff(A_REVERSE);
                row++;
            }
            
            refresh();
            needRedraw = false;
        }
        
        int ch = getch();
        if (ch == 'q' || ch == 'Q') { 
            inShop = false; 
            break; 
        }
        if (ch == KEY_UP) {
            selected = (selected - 1 + skins.size()) % skins.size();
            needRedraw = true;
        }
        if (ch == KEY_DOWN) {
            selected = (selected + 1) % skins.size();
            needRedraw = true;
        }
        if (ch == 10 || ch == ' ') {
            if (hasSkin(skins[selected].skin)) {
                setSnakeSkin(skins[selected].skin);
            } else if (mCoins >= skins[selected].price) {
                if (buySkin(skins[selected].skin, skins[selected].price)) {
                    setSnakeSkin(skins[selected].skin);
                }
            }
            needRedraw = true;
        }
    }
    
    clear();
    refresh();
}
void Game::showShopMenu_Item() {
    struct ItemInfo {
        ItemType type;
        std::string name;
        int price;
        std::string desc;
    };
    std::vector<ItemInfo> items = {
        {ItemType::Portal,     "Portal", 15, "Teleport to random location"},
        {ItemType::RandomBox,  "Random Box", 12, "Random effect"},
        {ItemType::Cheat,      "Cheat", 30, "Cheat for a round"},
        {ItemType::Attack,     "Attack", 20, "Attack opponent"},
        {ItemType::Shield,     "Shield/Helmet", 18, "Block one hit"}
    };
    
    int selected = 0;
    bool inShop = true;
    bool needRedraw = true;
    
    while (inShop) {
        if (needRedraw) {
            clear();
            refresh();
            
            // 居中显示
            int startY = (LINES - 10) / 2;
            int startX = (COLS - 60) / 2;
            
            // 标题
            mvprintw(startY, startX, "==== ITEM SHOP ====");
            
            // 金币信息
            mvprintw(startY + 2, startX, "Coins: %d", mCoins);
            
            // 操作说明
            mvprintw(startY + 3, startX, "Up/Down: Select   Enter: Buy   Q: Exit");
            
            // 表头
            mvprintw(startY + 5, startX, "%-15s %-8s %-8s %s", "Item Name", "Price", "Owned", "Description");
            
            // 道具列表
            int row = startY + 6;
            for (size_t i = 0; i < items.size(); ++i) {
                int count = getItemCount(items[i].type);
                
                if ((int)i == selected) attron(A_REVERSE);
                
                // 选择指示器
                mvprintw(row, startX, "%s ", (count > 0 ? ">" : " "));
                
                // 道具名称
                mvprintw(row, startX + 2, "%-15s", items[i].name.c_str());
                
                // 价格
                mvprintw(row, startX + 18, "$%-7d", items[i].price);
                
                // 拥有数量
                mvprintw(row, startX + 26, "%-8d", count);
                
                // 描述
                mvprintw(row, startX + 35, "%s", items[i].desc.c_str());
                
                if ((int)i == selected) attroff(A_REVERSE);
                row++;
            }
            
            refresh();
            needRedraw = false;
        }
        
        int ch = getch();
        if (ch == 'q' || ch == 'Q') { 
            inShop = false; 
            break; 
        }
        if (ch == KEY_UP) {
            selected = (selected - 1 + items.size()) % items.size();
            needRedraw = true;
        }
        if (ch == KEY_DOWN) {
            selected = (selected + 1) % items.size();
            needRedraw = true;
        }
        if (ch == 10 || ch == ' ') {
            if (mCoins >= items[selected].price) {
                buyItem(items[selected].type, items[selected].price);
            }
            needRedraw = true;
        }
    }
    
    clear();
    refresh();
}

// ====== 道具持久化 ======
const std::string ITEM_INVENTORY_FILE = "item_inventory.dat";

void Game::saveItemInventory() const {
    std::ofstream ofs(ITEM_INVENTORY_FILE, std::ios::binary);
    if (!ofs) return;
    int itemCount = mItemInventory.size();
    ofs.write(reinterpret_cast<const char*>(&itemCount), sizeof(itemCount));
    for (const auto& kv : mItemInventory) {
        int item = static_cast<int>(kv.first);
        int count = kv.second;
        ofs.write(reinterpret_cast<const char*>(&item), sizeof(item));
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));
    }
    ofs.close();
}

void Game::loadItemInventory() {
    std::ifstream ifs(ITEM_INVENTORY_FILE, std::ios::binary);
    if (!ifs) return;
    mItemInventory.clear();
    int itemCount = 0;
    ifs.read(reinterpret_cast<char*>(&itemCount), sizeof(itemCount));
    for (int i = 0; i < itemCount; ++i) {
        int item = 0, count = 0;
        ifs.read(reinterpret_cast<char*>(&item), sizeof(item));
        ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
        mItemInventory[static_cast<ItemType>(item)] = count;
    }
    ifs.close();
}

bool Game::buyItem(ItemType item, int price) {
    if (mCoins < price) return false;
    mCoins -= price;
    mItemInventory[item]++;
    return true;
}
int Game::getItemCount(ItemType item) const {
    auto it = mItemInventory.find(item);
    return (it != mItemInventory.end()) ? it->second : 0;
}
void Game::addItem(ItemType item, int count) {
    mItemInventory[item] += count;
}
bool Game::useItem(ItemType item) {
    if (getItemCount(item) > 0) {
        mItemInventory[item]--;
        return true;
    }
    return false;
}

// ====== 道具使用功能实现 ======

void Game::activateCheatMode() {
    if (useItem(ItemType::Cheat)) {
        mCheatMode = true;
        mCheatStartTime = std::chrono::steady_clock::now();
        if (mPtrSnake) {
            mPtrSnake->setInvincible(true);
        }
        if (mPtrSnake2) {
            mPtrSnake2->setInvincible(true);
        }
    }
}

void Game::deactivateCheatMode() {
    mCheatMode = false;
    if (mPtrSnake) {
        mPtrSnake->setInvincible(false);
    }
    if (mPtrSnake2) {
        mPtrSnake2->setInvincible(false);
    }
}

bool Game::isCheatModeActive() const {
    if (!mCheatMode) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - mCheatStartTime);
    float seconds = duration.count() / 1000.0f;
    
    if (seconds >= mCheatDuration) {
        const_cast<Game*>(this)->deactivateCheatMode();
        return false;
    }
    return true;
}

void Game::usePortal() {
    if (!useItem(ItemType::Portal) || !mPtrSnake) return;
    
    // 获取当前蛇头位置
    const auto& snake = mPtrSnake->getSnake();
    if (snake.empty()) return;
    
    // 寻找安全的新位置
    std::vector<SnakeBody> safePositions;
    for (int y = 1; y < mGameBoardHeight - 1; y++) {
        for (int x = 1; x < mGameBoardWidth - 1; x++) {
            // 检查是否是墙
            if (mPtrMap && mPtrMap->isWall(x, y)) continue;
            
            // 检查是否与蛇身重叠
            if (mPtrSnake->isPartOfSnake(x, y)) continue;
            
            // 检查是否与食物重叠
            if (x == mFood.getX() && y == mFood.getY()) continue;
            
            safePositions.push_back(SnakeBody(x, y));
        }
    }
    
    if (!safePositions.empty()) {
        // 随机选择一个安全位置
        int randomIndex = std::rand() % safePositions.size();
        SnakeBody newPos = safePositions[randomIndex];
        
        // 移动蛇头到新位置
        mPtrSnake->getSnake()[0] = newPos;
    }
}

void Game::useAttack() {
    if (!useItem(ItemType::Attack) || !mPtrSnake2) return;
    
    // 在对战模式中减少对手长度
    auto& snake2 = mPtrSnake2->getSnake();
    int reduceLength = 3; // 减少3格长度
    
    for (int i = 0; i < reduceLength && snake2.size() > 1; i++) {
        snake2.pop_back();
    }
}

void Game::handleItemUsage(int key) {
    // 简化道具按键：使用数字键1、2、3
    // 1键：作弊模式
    if (key == '1') {
        if (getItemCount(ItemType::Cheat) > 0) {
            activateCheatMode();
        }
    }
    // 2键：传送门
    else if (key == '2') {
        if (getItemCount(ItemType::Portal) > 0) {
            usePortal();
        }
    }
    // 3键：攻击道具
    else if (key == '3') {
        if (mCurrentMode == GameMode::Battle && getItemCount(ItemType::Attack) > 0) {
            useAttack();
        }
    }
    // 4键：护盾道具
    else if (key == '4') {
        if (getItemCount(ItemType::Shield) > 0 && !isShieldActive()) {
            activateShield();
        }
    }
}

void Game::updateCheatMode() {
    if (mCheatMode) {
        isCheatModeActive(); // 这会自动检查时间并停用作弊模式
    }
}

void Game::handleAcceleration(int key) {
    auto now = std::chrono::steady_clock::now();
    
    // 检查是否是方向键
    Direction currentDirection = Direction::Right;
    bool isDirectionKey = false;
    
    switch (key) {
        case 'w':
        case 'W':
        case KEY_UP:
            currentDirection = Direction::Up;
            isDirectionKey = true;
            break;
        case 's':
        case 'S':
        case KEY_DOWN:
            currentDirection = Direction::Down;
            isDirectionKey = true;
            break;
        case 'a':
        case 'A':
        case KEY_LEFT:
            currentDirection = Direction::Left;
            isDirectionKey = true;
            break;
        case 'd':
        case 'D':
        case KEY_RIGHT:
            currentDirection = Direction::Right;
            isDirectionKey = true;
            break;
    }
    
    if (isDirectionKey) {
        if (currentDirection == mLastKeyDirection) {
            // 相同方向键，检查是否长按
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastKeyPressTime);
            if (duration.count() > 150) { // 150ms后开始加速，平衡响应性和易用性
                mAccelerating = true;
            }
        } else {
            // 不同方向键，重置加速状态
            mAccelerating = false;
            mLastKeyDirection = currentDirection;
            mLastKeyPressTime = now; // 重置时间
        }
    } else {
        // 非方向键或没有按键，立即停止加速
        mAccelerating = false;
    }
}

bool Game::isKeyPressed(int key) {
    // 使用nodelay模式检查按键
    nodelay(stdscr, TRUE);
    int ch = getch();
    nodelay(stdscr, FALSE);
    return ch == key;
}

void Game::activateShield() {
    if (useItem(ItemType::Shield)) {
        mShieldActive = true;
    }
}

void Game::deactivateShield() {
    mShieldActive = false;
}

bool Game::isShieldActive() const {
    return mShieldActive;
}

int Game::getFoodEffect(FoodType foodType) const {
    switch (foodType) {
        case FoodType::Normal: return 1;    // 普通食物 +1
        case FoodType::Special1: return 2;  // 特殊食物1 +2
        case FoodType::Special2: return 3;  // 特殊食物2 +3
        case FoodType::Special3: return 5;  // 特殊食物3 +5
        case FoodType::Poison: return -1;   // 毒药 -1
        default: return 1;
    }
}

void Game::handleFoodEffect(FoodType foodType) {
    int effect = getFoodEffect(foodType);
    if (effect > 0) {
        // 正效果：增加长度
        for (int i = 0; i < effect; i++) {
            auto& snake = this->mPtrSnake->getSnake();
            if (!snake.empty()) {
                snake.push_back(snake.back()); // 复制尾部增加长度
            }
        }
        this->mPoints += effect;
        addCoins(effect); // 增加金币
    } else if (effect < 0) {
        // 负效果：减少长度
        auto& snake = this->mPtrSnake->getSnake();
        for (int i = 0; i < -effect && snake.size() > 1; i++) {
            snake.pop_back(); // 减少长度
        }
    }
}

// ====== 存档功能实现 ======

void Game::saveGame() const {
    std::ofstream ofs(mSaveFilePath, std::ios::binary);
    if (!ofs) {
        return;
    }
    
    // 保存游戏模式
    int currentMode = static_cast<int>(mCurrentMode);
    ofs.write(reinterpret_cast<const char*>(&currentMode), sizeof(currentMode));
    
    // 保存当前关卡
    ofs.write(reinterpret_cast<const char*>(&mCurrentLevel), sizeof(mCurrentLevel));
    
    // 保存分数
    ofs.write(reinterpret_cast<const char*>(&mPoints), sizeof(mPoints));
    ofs.write(reinterpret_cast<const char*>(&mPoints2), sizeof(mPoints2));
    
    // 保存生命值
    int lives1 = mPtrSnake ? mPtrSnake->getLives() : mPlayerLives;
    int lives2 = mPtrSnake2 ? mPtrSnake2->getLives() : mPlayer2Lives;
    ofs.write(reinterpret_cast<const char*>(&lives1), sizeof(lives1));
    ofs.write(reinterpret_cast<const char*>(&lives2), sizeof(lives2));
    
    // 保存蛇的状态
    if (mPtrSnake) {
        const auto& snake1 = mPtrSnake->getSnake();
        int size1 = snake1.size();
        ofs.write(reinterpret_cast<const char*>(&size1), sizeof(size1));
        for (const auto& body : snake1) {
            int x = body.getX();
            int y = body.getY();
            ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
            ofs.write(reinterpret_cast<const char*>(&y), sizeof(y));
        }
        int dir1 = static_cast<int>(mPtrSnake->getDirection());
        ofs.write(reinterpret_cast<const char*>(&dir1), sizeof(dir1));
    }
    
    if (mPtrSnake2) {
        const auto& snake2 = mPtrSnake2->getSnake();
        int size2 = snake2.size();
        ofs.write(reinterpret_cast<const char*>(&size2), sizeof(size2));
        for (const auto& body : snake2) {
            int x = body.getX();
            int y = body.getY();
            ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
            ofs.write(reinterpret_cast<const char*>(&y), sizeof(y));
        }
        int dir2 = static_cast<int>(mPtrSnake2->getDirection());
        ofs.write(reinterpret_cast<const char*>(&dir2), sizeof(dir2));
    }
    
    // 保存食物位置
    int foodX = mFood.getX();
    int foodY = mFood.getY();
    ofs.write(reinterpret_cast<const char*>(&foodX), sizeof(foodX));
    ofs.write(reinterpret_cast<const char*>(&foodY), sizeof(foodY));
    
    // 保存特殊食物状态
    ofs.write(reinterpret_cast<const char*>(&mHasSpecialFood), sizeof(mHasSpecialFood));
    if (mHasSpecialFood) {
        int specialX = mSpecialFood.getX();
        int specialY = mSpecialFood.getY();
        ofs.write(reinterpret_cast<const char*>(&specialX), sizeof(specialX));
        ofs.write(reinterpret_cast<const char*>(&specialY), sizeof(specialY));
        int foodType = static_cast<int>(mCurrentFoodType);
        ofs.write(reinterpret_cast<const char*>(&foodType), sizeof(foodType));
    }
    
    // 保存毒药状态
    ofs.write(reinterpret_cast<const char*>(&mHasPoison), sizeof(mHasPoison));
    if (mHasPoison) {
        int poisonX = mPoison.getX();
        int poisonY = mPoison.getY();
        ofs.write(reinterpret_cast<const char*>(&poisonX), sizeof(poisonX));
        ofs.write(reinterpret_cast<const char*>(&poisonY), sizeof(poisonY));
    }
    
    // 保存尸体食物
    int corpseCount = mCorpseFoods.size();
    ofs.write(reinterpret_cast<const char*>(&corpseCount), sizeof(corpseCount));
    for (const auto& corpse : mCorpseFoods) {
        int x = corpse.getX();
        int y = corpse.getY();
        ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
        ofs.write(reinterpret_cast<const char*>(&y), sizeof(y));
    }
    
    // 保存随机道具状态
    ofs.write(reinterpret_cast<const char*>(&mHasRandomItem), sizeof(mHasRandomItem));
    if (mHasRandomItem) {
        int itemX = mRandomItem.getX();
        int itemY = mRandomItem.getY();
        ofs.write(reinterpret_cast<const char*>(&itemX), sizeof(itemX));
        ofs.write(reinterpret_cast<const char*>(&itemY), sizeof(itemY));
        int itemType = static_cast<int>(mCurrentRandomItemType);
        ofs.write(reinterpret_cast<const char*>(&itemType), sizeof(itemType));
    }
    
    // 保存关卡状态
    int levelStatusSize = mLevelStatus.size();
    ofs.write(reinterpret_cast<const char*>(&levelStatusSize), sizeof(levelStatusSize));
    for (const auto& status : mLevelStatus) {
        int statusVal = static_cast<int>(status);
        ofs.write(reinterpret_cast<const char*>(&statusVal), sizeof(statusVal));
    }
    
    ofs.close();
}

bool Game::loadGame() {
    std::ifstream ifs(mSaveFilePath, std::ios::binary);
    if (!ifs) {
        return false;
    }
    
    try {
        // 加载游戏模式
        int currentMode;
        ifs.read(reinterpret_cast<char*>(&currentMode), sizeof(currentMode));
        mCurrentMode = static_cast<GameMode>(currentMode);
        
        // 加载当前关卡
        ifs.read(reinterpret_cast<char*>(&mCurrentLevel), sizeof(mCurrentLevel));
        
        // 验证关卡值的有效性
        if (mCurrentLevel < 1 || mCurrentLevel > mMaxLevel) {
            mCurrentLevel = 1; // 如果关卡值无效，重置为1
        }
        
        // 加载分数
        ifs.read(reinterpret_cast<char*>(&mPoints), sizeof(mPoints));
        ifs.read(reinterpret_cast<char*>(&mPoints2), sizeof(mPoints2));
        
        // 加载生命值
        int lives1, lives2;
        ifs.read(reinterpret_cast<char*>(&lives1), sizeof(lives1));
        ifs.read(reinterpret_cast<char*>(&lives2), sizeof(lives2));
        mPlayerLives = lives1;
        mPlayer2Lives = lives2;
        
        // 加载蛇1的状态
        if (mPtrSnake) {
            int size1;
            ifs.read(reinterpret_cast<char*>(&size1), sizeof(size1));
            std::vector<SnakeBody> snake1;
            for (int i = 0; i < size1; i++) {
                int x, y;
                ifs.read(reinterpret_cast<char*>(&x), sizeof(x));
                ifs.read(reinterpret_cast<char*>(&y), sizeof(y));
                snake1.push_back(SnakeBody(x, y));
            }
            mPtrSnake->getSnake() = snake1;
            
            int dir1;
            ifs.read(reinterpret_cast<char*>(&dir1), sizeof(dir1));
            mPtrSnake->changeDirection(static_cast<Direction>(dir1));
            mPtrSnake->setLives(lives1);
        }
        
        // 加载蛇2的状态
        if (mPtrSnake2) {
            int size2;
            ifs.read(reinterpret_cast<char*>(&size2), sizeof(size2));
            std::vector<SnakeBody> snake2;
            for (int i = 0; i < size2; i++) {
                int x, y;
                ifs.read(reinterpret_cast<char*>(&x), sizeof(x));
                ifs.read(reinterpret_cast<char*>(&y), sizeof(y));
                snake2.push_back(SnakeBody(x, y));
            }
            mPtrSnake2->getSnake() = snake2;
            
            int dir2;
            ifs.read(reinterpret_cast<char*>(&dir2), sizeof(dir2));
            mPtrSnake2->changeDirection(static_cast<Direction>(dir2));
            mPtrSnake2->setLives(lives2);
        }
        
        // 加载食物位置
        int foodX, foodY;
        ifs.read(reinterpret_cast<char*>(&foodX), sizeof(foodX));
        ifs.read(reinterpret_cast<char*>(&foodY), sizeof(foodY));
        mFood = SnakeBody(foodX, foodY);
        
        // 加载特殊食物状态
        ifs.read(reinterpret_cast<char*>(&mHasSpecialFood), sizeof(mHasSpecialFood));
        if (mHasSpecialFood) {
            int specialX, specialY;
            ifs.read(reinterpret_cast<char*>(&specialX), sizeof(specialX));
            ifs.read(reinterpret_cast<char*>(&specialY), sizeof(specialY));
            mSpecialFood = SnakeBody(specialX, specialY);
            
            int foodType;
            ifs.read(reinterpret_cast<char*>(&foodType), sizeof(foodType));
            mCurrentFoodType = static_cast<FoodType>(foodType);
        }
        
        // 加载毒药状态
        ifs.read(reinterpret_cast<char*>(&mHasPoison), sizeof(mHasPoison));
        if (mHasPoison) {
            int poisonX, poisonY;
            ifs.read(reinterpret_cast<char*>(&poisonX), sizeof(poisonX));
            ifs.read(reinterpret_cast<char*>(&poisonY), sizeof(poisonY));
            mPoison = SnakeBody(poisonX, poisonY);
        }
        
        // 加载尸体食物
        int corpseCount;
        ifs.read(reinterpret_cast<char*>(&corpseCount), sizeof(corpseCount));
        mCorpseFoods.clear();
        for (int i = 0; i < corpseCount; i++) {
            int x, y;
            ifs.read(reinterpret_cast<char*>(&x), sizeof(x));
            ifs.read(reinterpret_cast<char*>(&y), sizeof(y));
            mCorpseFoods.push_back(SnakeBody(x, y));
        }
        
        // 加载随机道具状态
        ifs.read(reinterpret_cast<char*>(&mHasRandomItem), sizeof(mHasRandomItem));
        if (mHasRandomItem) {
            int itemX, itemY;
            ifs.read(reinterpret_cast<char*>(&itemX), sizeof(itemX));
            ifs.read(reinterpret_cast<char*>(&itemY), sizeof(itemY));
            mRandomItem = SnakeBody(itemX, itemY);
            
            int itemType;
            ifs.read(reinterpret_cast<char*>(&itemType), sizeof(itemType));
            mCurrentRandomItemType = static_cast<ItemType>(itemType);
        }
        
        // 加载关卡状态
        int levelStatusSize;
        ifs.read(reinterpret_cast<char*>(&levelStatusSize), sizeof(levelStatusSize));
        mLevelStatus.clear();
        for (int i = 0; i < levelStatusSize; i++) {
            int statusVal;
            ifs.read(reinterpret_cast<char*>(&statusVal), sizeof(statusVal));
            mLevelStatus.push_back(static_cast<LevelStatus>(statusVal));
        }
        
        // 同步食物信息给蛇
        if (mPtrSnake) {
            mPtrSnake->senseFood(mFood);
            mPtrSnake->senseSpecialFood(mSpecialFood);
            mPtrSnake->sensePoison(mPoison);
            mPtrSnake->senseCorpseFoods(mCorpseFoods);
            mPtrSnake->senseRandomItem(mRandomItem);
        }
        if (mPtrSnake2) {
            mPtrSnake2->senseFood(mFood);
            mPtrSnake2->senseSpecialFood(mSpecialFood);
            mPtrSnake2->sensePoison(mPoison);
            mPtrSnake2->senseCorpseFoods(mCorpseFoods);
            mPtrSnake2->senseRandomItem(mRandomItem);
        }
        
        ifs.close();
        return true;
    } catch (...) {
        ifs.close();
        return false;
    }
}

bool Game::hasSaveFile() const {
    std::ifstream ifs(mSaveFilePath, std::ios::binary);
    return ifs.good();
}

void Game::deleteSaveFile() const {
    std::remove(mSaveFilePath.c_str());
}
