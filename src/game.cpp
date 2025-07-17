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
    mvwprintw(this->mWindows[0], 2, 1, "Author: Go Go Snakeg");
    mvwprintw(this->mWindows[0], 3, 1, "Website: https://github.com/Jessica939/snake_src_full");
    mvwprintw(this->mWindows[0], 4, 1, "Implemented using C++.");
    
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
    //lives
    if (mCurrentMode == GameMode::Classic && mPtrSnake != nullptr) {
            mvwprintw(this->mWindows[2], row++, 1, "Lives");
            mvwprintw(this->mWindows[2], row++, 2, "%d", mPtrSnake->getLives());
    }
    
    // Level - 仅在关卡模式下显示
    if (mCurrentMode == GameMode::Level) {
        mvwprintw(this->mWindows[2], row++, 1, "Level");
        mvwprintw(this->mWindows[2], row++, 2, "%d", mCurrentLevel);
    }
    
    // --- 剩余时间 (仅限时模式) ---
    if (mCurrentMode == GameMode::Timed) {
        mvwprintw(this->mWindows[2], 14, 1, "Time Left:");
        mvwprintw(this->mWindows[2], 15, 2, "%d s", mTimeRemaining);
    }
    
    // Points
    mvwprintw(this->mWindows[2], row++, 1, "Points");
    mvwprintw(this->mWindows[2], row++, 2, "%d", mPoints);
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
        mvwprintw(menu, 2, 1, "%s", pointString.c_str());
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

// 新增：关卡失败菜单
int Game::renderLevelFailureMenu() const
{
    WINDOW * menu;
    int width = this->mGameBoardWidth * 0.6;
    int height = 8;
    int startX = this->mGameBoardWidth * 0.2;
    int startY = this->mGameBoardHeight * 0.3 + this->mInformationHeight;

    menu = newwin(height, width, startY, startX);
    box(menu, 0, 0);

    // 三个选项
    std::vector<std::string> menuItems = {
        "Retry Current Level",
        "Return to Level Select (GUI)",
        "Quit Game"
    };

    int index = 0;
    int offset = 3;

    mvwprintw(menu, 1, 1, "Level %d Failed!", mCurrentLevel);
    mvwprintw(menu, 2, 1, "Your Score: %d", mPoints);

    // 显示初始菜单
    for (int i = 0; i < menuItems.size(); i++) {
        if (i == index) {
            wattron(menu, A_STANDOUT);
            mvwprintw(menu, i + offset, 1, menuItems[i].c_str());
            wattroff(menu, A_STANDOUT);
        } else {
            mvwprintw(menu, i + offset, 1, menuItems[i].c_str());
        }
    }

    wrefresh(menu);

    int key;
    while (true) {
        key = getch();
        switch(key) {
            case 'W': case 'w': case KEY_UP:
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index = (index == 0) ? menuItems.size() - 1 : index - 1;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            case 'S': case 's': case KEY_DOWN:
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index = (index == menuItems.size() - 1) ? 0 : index + 1;
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
    return index;
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
    // 显示当前得分
    std::string pointString = std::to_string(this->mPoints);
    mvwprintw(this->mWindows[2], 15, 2, "%s", pointString.c_str());
    
    // 显示目标分数（仅在关卡模式中）
    if (mCurrentMode == GameMode::Level && mCurrentLevel <= mMaxLevel) {
        std::string targetString = std::to_string(mLevelTargetPoints);
        mvwprintw(this->mWindows[2], 16, 8, "%s", targetString.c_str());
    } else {
        mvwprintw(this->mWindows[2], 16, 8, "  ");
    }
    
    wrefresh(this->mWindows[2]);
}

void Game::renderLevel() const
{
    // 只在关卡模式下显示关卡信息
    if (mCurrentMode != GameMode::Level) {
        return;
    }
    
    // 显示当前关卡
    if (mCurrentLevel <= mMaxLevel) {
        std::string levelString = std::to_string(this->mCurrentLevel);
        mvwprintw(this->mWindows[2], 12, 2, "%s", levelString.c_str());
        
        // 显示关卡类型
        std::string typeString;
        switch (mCurrentLevelType) {
            case LevelType::Normal:
                typeString = "Normal";
                break;
            case LevelType::Speed:
                typeString = "Speed";
                break;
            case LevelType::Maze:
                typeString = "Maze";
                break;
            case LevelType::Custom1:
                typeString = "Special";
                break;
            case LevelType::Custom2:
                typeString = "Boss";
                break;
            default:
                typeString = "Unknown";
        }
        mvwprintw(this->mWindows[2], 12, 10, "(%s)", typeString.c_str());
    }
    
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
    
    this->mPoints = 0;
    this->mDelay = this->mBaseDelay;
    
    // 在经典模式中设置蛇的生命数
    if (mCurrentMode == GameMode::Classic && mPtrSnake) {
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
            // 计算实际地图坐标（考虑视窗偏移）
            int mapX = x;
            int mapY = y;
            
            // 如果启用了视窗跟随，则应用偏移
            if (mUseViewport) {
                mapX += mViewOffsetX;
                mapY += mViewOffsetY;
            }
            
            // 检查坐标是否在地图范围内
            if (mapX >= 0 && mapX < this->mPtrMap->getWidth() &&
                mapY >= 0 && mapY < this->mPtrMap->getHeight())
            {
                if (this->mPtrMap->isWall(mapX, mapY)) {
                    mvwaddch(this->mWindows[1], y, x, this->mWallSymbol);
                }
            }
            else {
                // 如果超出地图范围，显示边界墙
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
        int x = snake[i].getX();
        int y = snake[i].getY();
        
        // 如果启用视窗跟随，将坐标转换为窗口相对坐标
        if (mUseViewport)
        {
            x -= mViewOffsetX;
            y -= mViewOffsetY;
        }
        
        // 只渲染在窗口范围内的蛇身部分
        if (x >= 0 && x < this->mGameBoardWidth && y >= 0 && y < this->mGameBoardHeight)
        {
            mvwaddch(this->mWindows[1], y, x, this->mSnakeSymbol);
        }
    }
    wattroff(this->mWindows[1], COLOR_PAIR(color_pair));
    wrefresh(this->mWindows[1]);
}

void Game::controlSnake() const
{
    // 设置为非阻塞模式
    nodelay(stdscr, TRUE);
    
    int key;
    key = getch();
    
    // 如果没有按键输入（-1），则不改变方向
    if(key == -1) {
        return;
    }
    
    // 如果是ESC键，不执行任何操作（防止ESC键导致游戏暂停）
    if(key == 27) {  // 27是ESC键的ASCII值
        return;
    }
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
    // 设置为非阻塞模式
    nodelay(stdscr, TRUE);
    
    // 添加一个准备阶段，让玩家有时间反应
    {
        // 渲染当前状态，让玩家看到蛇的初始位置
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        this->renderMap();
        this->renderSnake();
        this->renderFood();
        
        // 创建一个倒计时窗口
        WINDOW* countdownWin;
        int width = 24;
        int height = 5;
        int startX = (this->mGameBoardWidth - width) / 2;
        int startY = (this->mGameBoardHeight - height) / 2 + this->mInformationHeight;
        
        countdownWin = newwin(height, width, startY, startX);
        box(countdownWin, 0, 0);
        mvwprintw(countdownWin, 0, 8, "GET READY");
        
        // 倒计时3秒
        for (int i = 3; i > 0; i--) {
            mvwprintw(countdownWin, 2, 9, "READY: %d", i);
            wrefresh(countdownWin);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        mvwprintw(countdownWin, 2, 10, "GO!   ");
        wrefresh(countdownWin);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 删除倒计时窗口
        delwin(countdownWin);
    }
    
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
    // 初始化时设置非阻塞模式
    nodelay(stdscr, TRUE);
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
                
                        // 显示通关后的文字叙述
                        this->displayLevelCompletion(mCurrentLevel);
                
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
                
                        // 根据是否是最后一关或第一关显示不同的提示
                        if (mCurrentLevel == 1) {
                            // 第一关通过自动进入第二关
                            mvwprintw(levelCompleteWin, 3, 1, "Level %d Unlocked!", mCurrentLevel + 1);
                            mvwprintw(levelCompleteWin, 4, 1, "Entering Level 2 automatically...");
                            wrefresh(levelCompleteWin);
                            
                            // 短暂延迟后自动继续
                            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        } else if (mCurrentLevel < mMaxLevel) {
                            mvwprintw(levelCompleteWin, 3, 1, "Level %d Unlocked!", mCurrentLevel + 1);
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
                        } else {
                            mvwprintw(levelCompleteWin, 3, 1, "You completed all levels!");
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
                            int key;
                            while (true) {
                                key = getch();
                                if (key == ' ' || key == 10)
                                    break;
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                    
                            delwin(gameCompleteWin);
                        }
                
                        // 如果是第一关通过，自动进入第二关
                        if (mCurrentLevel == 1) {
                            // 自动进入第二关
                            mCurrentLevel = 2;
                            mIsLevelRetry = false; // 重置重试标志
                            continue;
                        } else {
                            // 其他关卡返回关卡选择界面
                            if (!this->selectLevelInLevelMode()) {
                                // 用户选择退出
                                break;
                            }
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
                            // 重新尝试当前关卡，设置重试标志
                            mIsLevelRetry = true;
                            continue;
                        } else if (index == 1) {
                            // 返回关卡选择界面，重置重试标志
                            mIsLevelRetry = false;
                            if (!this->selectLevelInLevelMode()) {
                                // 用户选择退出
                                break;
                            }
                        } else if (index == 2) {
                            // 返回到模式选择，重置重试标志
                            mIsLevelRetry = false;
                            mReturnToModeSelect = true;
                            break; // 先退出当前循环
                        } else {
                            // 退出游戏，重置重试标志
                            mIsLevelRetry = false;
                            break;
                        }
                    }
                }
                
                // 检查是否需要返回到模式选择界面
                if (mReturnToModeSelect) {
                    playAgain = false; // 结束当前游戏模式循环
                    break; // 退出关卡模式循环
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
        int offset = 2; // 减小偏移，使菜单更紧凑
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

void Game::displayLevelIntroduction(int level)
{
    // 确保所有面板都被绘制
    this->renderBoards();
    
    // 清除游戏区域并刷新，以准备显示介绍文字
    werase(this->mWindows[1]);
    box(this->mWindows[1], 0, 0);
    wrefresh(this->mWindows[1]);
    
    // 创建一个窗口用于显示开场文字
    WINDOW* introWin;
    int width = this->mGameBoardWidth * 0.85; // 略微增大宽度
    int height = this->mGameBoardHeight * 0.7; // 增大高度
    int startX = this->mGameBoardWidth * 0.075; // 调整X坐标保持居中
    int startY = this->mGameBoardHeight * 0.15 + this->mInformationHeight; // 向上移动20像素

    introWin = newwin(height, width, startY, startX);
    box(introWin, 0, 0);
    
    // 设置标题
    std::string title = "LEVEL " + std::to_string(level);
    mvwprintw(introWin, 1, (width - title.length()) / 2, "%s", title.c_str());
    
    // 根据关卡显示不同的开场文字
    std::vector<std::string> introText;
    
    switch (level) {
        case 1:
            introText = {
                " The Eternal Trial",
                "",
                "The world is falling apart before your eyes.",
                "The warmth of the nest, the gentle kiss of parents, the melody of a lullaby... everything becomes a distant and vague echo.",
                "You are thrown into the end of time, a cage called the Stasis Ruins.",
                "Sadness and fear wrap around you like ice, but in this dead silence, your parents' last words ignite a faint spark in your mind:",
                "My child, before you embark on your journey, you must learn to survive in eternal loneliness.",
                "This is their first teaching and your only guidepost.",
                "Stay alive.",
                "In order to feel the warmth again, in order to find out the truth, in order to... no longer be alone.",
                "This is your first adventure in this barren land.",
                "You must find enough food to survive.",
                "",
                "OBJECTIVE: Collect 100 pieces of food to learn to survive in eternal loneliness!"
            };
            break;
        case 2:
            introText = {
                "The Arrow of Time",
                "",
                "You have entered the \"realm of flowing light\".",
                "This is the time and space eroded by the remaining power of the \"Mechanical Master\", turning time into the most deadly weapon.",
                "Here, a moment of hesitation will be left behind by the torrent of time.",
                "The second lesson from parents sounded at the right time, like a warning bell:",
                "Remember, time is both your enemy and your weapon. You have to learn to harness it instead of being swallowed up by it.",
                "You take a deep breath and sink the power of \"Tough Heart\" into the depths of your consciousness.",
                "Now you have to be faster than time.",
                "",
                "OBJECTIVE: Collect 5 pieces of food within 30 seconds!"
            };
            break;
        case 3:
            introText = {
                "The Heart of Twin Shadows",
                "Your longing has awakened a spiritual imprint sleeping here.",
                "A gentle yet powerful voice, the combined voice of your parents, echoes from the water:",
                "\"We never left you, child. Feel our presence. You are never fighting alone.\"",
                "In the center of the pool, light gathers, forming a 'Soul Shadow' made of pure energy. It is both a trial and a guardian.",
                "Now, how will you face this memory?",
                ""
            };
            break;
        case 4:
            introText = {
                " The Melody of Life",
                "You have arrived at the Echoing Canyon.",
                "Everything here moves to an unseen rhythm. The power of the three shards allows you to \"hear\" this cosmic symphony.",
                "The Mechanos, on the other hand, represents absolute silence—the static that would erase all melody.",
                "Your parents' final lesson, the ultimate truth, now resonates within you:",
                "\"The world has its own heartbeat, a rhythm the machines can never understand. Listen to it, resonate with it. That is the most fundamental power of life.\"",
                "Forget mere movement. Let your body become a note in this symphony of life.",
                
            };
            break;
        case 5:
            introText = {
                " The Core Cleansing",
                "You have finally reached it.",
                "Hatred may have been your starting point, but a far greater power drives you now.",
                "Fortitude, Celerity, Bonds, Harmony.",
                "Your parents' legacy is now fully part of your soul. You are no longer an orphan seeking revenge, but the new guardian, embodying all the best qualities of life.",
                "The four shards form a halo of light above you. Their final words send you off as a blessing:",
                "\"Go, child. Use everything you have learned to end the old destiny and forge a new future.\"",
                "The final battle begins now.",
            };
            break;
        default:
            introText = {
                "UNKNOWN ADVENTURE",
                "",
                "A new challenge awaits you ahead...",
                "",
                "Are you ready?"
            };
    }
    
    // 实现每行文字单独显示，并自动换行的效果
    const int displayTime = 2000; // 每行显示时间（毫秒）
    const int maxDisplayWidth = width - 6; // 可显示的最大宽度（留边距）
    
    // 辅助函数：将长文本按单词分割成适合宽度的多行
    auto wrapText = [maxDisplayWidth](const std::string& text) -> std::vector<std::string> {
        std::vector<std::string> wrappedLines;
        if (text.empty()) {
            wrappedLines.push_back("");
            return wrappedLines;
        }
        
        std::istringstream wordStream(text);
        std::string word;
        std::string currentLine;
        
        while (wordStream >> word) {
            // 如果加上这个词会超出宽度，且当前行不为空，则另起一行
            if (currentLine.length() + word.length() + 1 > static_cast<size_t>(maxDisplayWidth) && !currentLine.empty()) {
                wrappedLines.push_back(currentLine);
                currentLine = word;
            }
            // 如果是第一个词或者可以加入当前行
            else {
                if (!currentLine.empty()) {
                    currentLine += " ";
                }
                currentLine += word;
            }
        }
        
        if (!currentLine.empty()) {
            wrappedLines.push_back(currentLine);
        }
        
        // 如果没有成功分行（可能是因为单词太长），强制按字符分割
        if (wrappedLines.empty()) {
            std::string line;
            for (char c : text) {
                if (line.length() >= static_cast<size_t>(maxDisplayWidth)) {
                    wrappedLines.push_back(line);
                    line.clear();
                }
                line += c;
            }
            if (!line.empty()) {
                wrappedLines.push_back(line);
            }
        }
        
        return wrappedLines;
    };
    
    // 设置getch为非阻塞模式，以便检测按键
    nodelay(stdscr, TRUE);
    
    // 在窗口底部显示跳过提示
    mvwprintw(introWin, height - 2, 2, "Press ESC to skip");
    wrefresh(introWin);
    
    bool skip = false;
    
    // 显示每一行原始文字（可能会自动换行）
    for (size_t i = 0; i < introText.size(); i++) {
        // 检查是否按下ESC键跳过
        int ch = getch();
        if (ch == 27) { // ESC键的ASCII码是27
            skip = true;
            break;
        }
        
        const std::string& originalLine = introText[i];
        
        // 如果是空行，只显示很短的时间
        if (originalLine.empty()) {
            // 清除显示区域
            for (int y = 3; y < height - 3; y++) {
                wmove(introWin, y, 2);
                for (int x = 2; x < width - 2; x++) {
                    waddch(introWin, ' ');
                }
            }
            wrefresh(introWin);
            
            // 在等待期间检查是否按下ESC键
            auto start = std::chrono::steady_clock::now();
            while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < 300) {
                ch = getch();
                if (ch == 27) {
                    skip = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (skip) break;
            continue;
        }
        
        // 将原始行按需换行
        std::vector<std::string> wrappedLines = wrapText(originalLine);
        
        // 清除显示区域
        for (int y = 3; y < height - 3; y++) {
            wmove(introWin, y, 2);
            for (int x = 2; x < width - 2; x++) {
                waddch(introWin, ' ');
            }
        }
        
        // 计算起始行，使文本垂直居中
        int startLine = (height - wrappedLines.size()) / 2;
        if (startLine < 3) startLine = 3;
        
        // 逐行显示包装后的文本
        for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
            const std::string& line = wrappedLines[lineIdx];
            
            // 计算当前行的水平居中位置
            int startX = (width - line.length()) / 2;
            if (startX < 2) startX = 2;
            
            // 逐个字符显示当前行
            for (size_t j = 0; j < line.length(); j++) {
                mvwaddch(introWin, startLine + static_cast<int>(lineIdx), startX + static_cast<int>(j), line[j]);
                wrefresh(introWin);
                
                // 每个字符显示后短暂暂停，同时检查是否按下ESC键
                ch = getch();
                if (ch == 27) {
                    skip = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            
            if (skip) break;
        }
        
        if (skip) break;
        
        // 显示完整行后等待一段时间，同时检查是否按下ESC键
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start).count() < displayTime) {
            ch = getch();
            if (ch == 27) {
                skip = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        if (skip) break;
    }
    
    // 如果跳过，立即显示所有文本的最后一行
    if (skip && !introText.empty()) {
        // 清除显示区域
        for (int y = 3; y < height - 3; y++) {
            wmove(introWin, y, 2);
            for (int x = 2; x < width - 2; x++) {
                waddch(introWin, ' ');
            }
        }
        
        // 找到最后一个非空行
        std::string lastLine;
        for (auto it = introText.rbegin(); it != introText.rend(); ++it) {
            if (!it->empty()) {
                lastLine = *it;
                break;
            }
        }
        
        if (!lastLine.empty()) {
            // 将最后一行按需换行
            std::vector<std::string> wrappedLines = wrapText(lastLine);
            
            // 计算起始行，使文本垂直居中
            int startLine = (height - wrappedLines.size()) / 2;
            if (startLine < 3) startLine = 3;
            
            // 显示最后一行
            for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
                const std::string& line = wrappedLines[lineIdx];
                
                // 计算当前行的水平居中位置
                int startX = (width - line.length()) / 2;
                if (startX < 2) startX = 2;
                
                mvwprintw(introWin, startLine + static_cast<int>(lineIdx), startX, "%s", line.c_str());
            }
            wrefresh(introWin);
        }
    }
    
    // 恢复getch为阻塞模式
    nodelay(stdscr, FALSE);
    
    // 如果没有跳过，清除所有文本
    if (!skip) {
        for (int y = 3; y < height - 3; y++) {
            wmove(introWin, y, 2);
            for (int x = 2; x < width - 2; x++) {
                waddch(introWin, ' ');
            }
        }
    }
    
    // 显示按键提示（替换ESC跳过提示）
    mvwprintw(introWin, height - 2, 2, "Press SPACE to continue...                ");
    
    wrefresh(introWin);
    
    // 等待用户按空格键继续
    int key;
    while (true) {
        key = getch();
        if (key == ' ' || key == 10) // 空格键或回车键
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // For level 3,弹出模式选择框
    if (level == 3) {
        // 清除introWin内容
        for (int y = 2; y < height - 2; y++) {
            wmove(introWin, y, 2);
            for (int x = 2; x < width - 2; x++) {
                waddch(introWin, ' ');
            }
        }
        box(introWin, 0, 0);
        // 选项内容
        std::vector<std::string> choices = {
            "*Face the trial alone*",
            "*Fight alongside an ally*"
        };
        std::vector<std::string> descs = {
            "You decide to face the test yourself, to listen to their wordless teachings.",
            "You choose to trust the companion, inviting them into this place of memory."
        };
        int choiceIdx = 0;
        int choiceY = height / 2 - 2;
        int descY = choiceY + 3;
        // 包装选项和描述文本
        auto wrapText = [width](const std::string& text) -> std::vector<std::string> {
            int maxDisplayWidth = width - 8;
            std::vector<std::string> wrappedLines;
            if (text.empty()) {
                wrappedLines.push_back("");
                return wrappedLines;
            }
            std::istringstream wordStream(text);
            std::string word;
            std::string currentLine;
            while (wordStream >> word) {
                if (currentLine.length() + word.length() + 1 > static_cast<size_t>(maxDisplayWidth) && !currentLine.empty()) {
                    wrappedLines.push_back(currentLine);
                    currentLine = word;
                } else {
                    if (!currentLine.empty()) currentLine += " ";
                    currentLine += word;
                }
            }
            if (!currentLine.empty()) wrappedLines.push_back(currentLine);
            if (wrappedLines.empty()) {
                std::string line;
                for (char c : text) {
                    if (line.length() >= static_cast<size_t>(maxDisplayWidth)) {
                        wrappedLines.push_back(line);
                        line.clear();
                    }
                    line += c;
                }
                if (!line.empty()) wrappedLines.push_back(line);
            }
            return wrappedLines;
        };
        // 渲染函数
        auto renderChoices = [&](int highlightIdx) {
            int y = choiceY;
            for (int i = 0; i < (int)choices.size(); ++i) {
                std::vector<std::string> lines = wrapText(choices[i]);
                for (size_t l = 0; l < lines.size(); ++l, ++y) {
                    int startX = (width - lines[l].length()) / 2;
                    if (startX < 4) startX = 4;
                    if (i == highlightIdx) wattron(introWin, A_STANDOUT);
                    mvwprintw(introWin, y, startX, "%s", lines[l].c_str());
                    if (i == highlightIdx) wattroff(introWin, A_STANDOUT);
                }
            }
        };
        auto renderDesc = [&](int idx) {
            std::vector<std::string> lines = wrapText(descs[idx]);
            int y = descY;
            for (size_t l = 0; l < lines.size(); ++l, ++y) {
                int startX = (width - lines[l].length()) / 2;
                if (startX < 4) startX = 4;
                mvwprintw(introWin, y, startX, "%s", lines[l].c_str());
            }
        };
        // 渲染初始选项和描述
        renderChoices(choiceIdx);
        renderDesc(choiceIdx);
        wrefresh(introWin);
        // 选项循环
        int key = 0;
        while (true) {
            key = getch();
            if (key == 'W' || key == 'w' || key == KEY_UP) {
                // 上移
                renderChoices(-1); // 先清除高亮
                choiceIdx = (choiceIdx - 1 + choices.size()) % choices.size();
                renderChoices(choiceIdx);
            } else if (key == 'S' || key == 's' || key == KEY_DOWN) {
                // 下移
                renderChoices(-1);
                choiceIdx = (choiceIdx + 1) % choices.size();
                renderChoices(choiceIdx);
            } else if (key == ' ' || key == 10) {
                // 确认
                break;
            }
            // 清空描述区
            for (int y = descY; y < descY + 3; ++y) {
                mvwprintw(introWin, y, 4, "%*s", width - 8, "");
            }
            renderDesc(choiceIdx);
            wrefresh(introWin);
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
        // 记录选择，后续可用成员变量保存
        mLevel3ModeChoice = choiceIdx; // 0: alone, 1: with ally

        // 如果选择了模式一，显示模式一的剧情前文本
        if (mLevel3ModeChoice == 0) {
            // 清空窗口内容
            for (int y = 2; y < height - 2; y++) {
                wmove(introWin, y, 2);
                for (int x = 2; x < width - 2; x++) {
                    waddch(introWin, ' ');
                }
            }
            box(introWin, 0, 0);
            // 标题
            std::string modeTitle = "The Mirror Dance";
            mvwprintw(introWin, 1, (width - modeTitle.length()) / 2, "%s", modeTitle.c_str());
            // 剧情文本
            std::vector<std::string> storyText = {
                "In that wordless dance, you came to understand your parents' hearts.",
                "Every move the shadow made was a consequence of your own. You learned empathy, foresight, and guardianship.",
                "You understood that 'coexistence' is not merely sharing a space, but a connection of souls. Even across worlds, love can move in perfect sync."
            };
            // 包装文本
            auto wrapText = [width](const std::string& text) -> std::vector<std::string> {
                int maxDisplayWidth = width - 8;
                std::vector<std::string> wrappedLines;
                if (text.empty()) {
                    wrappedLines.push_back("");
                    return wrappedLines;
                }
                std::istringstream wordStream(text);
                std::string word;
                std::string currentLine;
                while (wordStream >> word) {
                    if (currentLine.length() + word.length() + 1 > static_cast<size_t>(maxDisplayWidth) && !currentLine.empty()) {
                        wrappedLines.push_back(currentLine);
                        currentLine = word;
                    } else {
                        if (!currentLine.empty()) currentLine += " ";
                        currentLine += word;
                    }
                }
                if (!currentLine.empty()) wrappedLines.push_back(currentLine);
                if (wrappedLines.empty()) {
                    std::string line;
                    for (char c : text) {
                        if (line.length() >= static_cast<size_t>(maxDisplayWidth)) {
                            wrappedLines.push_back(line);
                            line.clear();
                        }
                        line += c;
                    }
                    if (!line.empty()) wrappedLines.push_back(line);
                }
                return wrappedLines;
            };

            // 设置getch为非阻塞模式，以便检测按键
            nodelay(stdscr, TRUE);
            
            // 在窗口底部显示跳过提示
            mvwprintw(introWin, height - 2, 2, "Press ESC to skip");
            wrefresh(introWin);
            
            bool skip = false;
            const int displayTime = 1500; // 每段显示时间（毫秒）
            
            // 逐行显示剧情文本
            for (size_t paraIdx = 0; paraIdx < storyText.size(); paraIdx++) {
                // 检查是否按下ESC键跳过
                int ch = getch();
                if (ch == 27) { // ESC键的ASCII码是27
                    skip = true;
                    break;
                }
                
                // 将段落按需换行
                std::vector<std::string> wrappedLines = wrapText(storyText[paraIdx]);
                
                // 清除显示区域
                for (int y = 3; y < height - 3; y++) {
                    wmove(introWin, y, 2);
                    for (int x = 2; x < width - 2; x++) {
                        waddch(introWin, ' ');
                    }
                }
                
                // 计算起始行，使文本垂直居中
                int startLine = (height - wrappedLines.size()) / 2;
                if (startLine < 3) startLine = 3;
                
                // 逐行显示包装后的文本
                for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
                    const std::string& line = wrappedLines[lineIdx];
                    
                    // 计算当前行的水平居中位置
                    int startX = (width - line.length()) / 2;
                    if (startX < 2) startX = 2;
                    
                    // 逐个字符显示当前行
                    for (size_t j = 0; j < line.length(); j++) {
                        mvwaddch(introWin, startLine + static_cast<int>(lineIdx), startX + static_cast<int>(j), line[j]);
                        wrefresh(introWin);
                        
                        // 每个字符显示后短暂暂停，同时检查是否按下ESC键
                        ch = getch();
                        if (ch == 27) {
                            skip = true;
                            break;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    }
                    
                    if (skip) break;
                }
                
                if (skip) break;
                
                // 显示完整段落后等待一段时间，同时检查是否按下ESC键
                auto start = std::chrono::steady_clock::now();
                while (std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - start).count() < displayTime) {
                    ch = getch();
                    if (ch == 27) {
                        skip = true;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                
                if (skip) break;
            }
            
            // 如果跳过，立即显示最后一段
            if (skip && !storyText.empty()) {
                // 清除显示区域
                for (int y = 3; y < height - 3; y++) {
                    wmove(introWin, y, 2);
                    for (int x = 2; x < width - 2; x++) {
                        waddch(introWin, ' ');
                    }
                }
                
                // 找到最后一段
                const std::string& lastPara = storyText.back();
                
                // 将最后一段按需换行
                std::vector<std::string> wrappedLines = wrapText(lastPara);
                
                // 计算起始行，使文本垂直居中
                int startLine = (height - wrappedLines.size()) / 2;
                if (startLine < 3) startLine = 3;
                
                // 显示最后一段
                for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
                    const std::string& line = wrappedLines[lineIdx];
                    
                    // 计算当前行的水平居中位置
                    int startX = (width - line.length()) / 2;
                    if (startX < 2) startX = 2;
                    
                    mvwprintw(introWin, startLine + static_cast<int>(lineIdx), startX, "%s", line.c_str());
                }
                wrefresh(introWin);
            }
            
            // 恢复getch为阻塞模式
            nodelay(stdscr, FALSE);
            
            // 显示按键提示（替换ESC跳过提示）
            mvwprintw(introWin, height - 2, 2, "Press SPACE to continue...                ");
            wrefresh(introWin);
            
            // 等待空格或回车
            int key;
            while (true) {
                key = getch();
                if (key == ' ' || key == 10)
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    // 删除窗口
    delwin(introWin);

    // 重新绘制游戏界面
    werase(this->mWindows[1]);
    box(this->mWindows[1], 0, 0);
    wrefresh(this->mWindows[1]);
}


void Game::initializeLevel(int level)
{
    mCurrentLevel = level;
    
    // 重新绘制界面
    this->renderBoards();
    
    // 根据关卡设置关卡类型
    switch (level) {
        case 1:
            mCurrentLevelType = LevelType::Normal;
            mLevelTargetPoints = 100;  // 第一关目标设置为100个食物
            break;
        case 2:
            mCurrentLevelType = LevelType::Speed;
            mLevelTargetPoints = 5;
            break;
        case 3:
            mCurrentLevelType = LevelType::Maze;
            mLevelTargetPoints = 8; // 只需要8个食物就能过关
            break;
        case 4:
            mCurrentLevelType = LevelType::Custom1;
            mLevelTargetPoints = 1;
            // 初始化第四关特殊设置
            this->initializeLevel4();
            // 显示开场介绍（除非是重试）
            if (!mIsLevelRetry) {
                this->displayLevelIntroduction(level);
            }
            return; // 第四关有特殊初始化，直接返回
        case 5:
            mCurrentLevelType = LevelType::Custom2;
            mLevelTargetPoints = 15;
            // 初始化第五关特殊设置
            this->initializeLevel5();
            // 显示开场介绍（除非是重试）
            if (!mIsLevelRetry) {
                this->displayLevelIntroduction(level);
            }
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
    
    // 尝试寻找合适的蛇初始位置，确保有足够的安全空间
    bool snakeInitialized = false;
    
    // 首先尝试找到一个有非常大的安全空间的位置 (10格)
    std::vector<std::pair<SnakeBody, InitialDirection>> validPositions =
        this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 10);
    
    if (!validPositions.empty()) {
        // 选择更靠近中央的位置
        int bestIdx = 0;
        int centerX = this->mGameBoardWidth / 2;
        int centerY = this->mGameBoardHeight / 2;
        int minDist = this->mGameBoardWidth + this->mGameBoardHeight; // 初始最大距离
        
        for (size_t i = 0; i < validPositions.size(); i++) {
            auto [pos, dir] = validPositions[i];
            int dx = pos.getX() - centerX;
            int dy = pos.getY() - centerY;
            int dist = std::abs(dx) + std::abs(dy); // 曼哈顿距离
            
            if (dist < minDist) {
                minDist = dist;
                bestIdx = i;
            }
        }
        
        auto [startPos, direction] = validPositions[bestIdx];
        this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
        snakeInitialized = true;
    }
    
    // 如果没有找到理想的位置，尝试降低空间要求
    if (!snakeInitialized) {
        validPositions = this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 6);
        
        if (!validPositions.empty()) {
            int idx = std::rand() % validPositions.size();
            auto [startPos, direction] = validPositions[idx];
            this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
            snakeInitialized = true;
        }
    }
    
    // 如果还是没有找到合适的位置，再降低要求
    if (!snakeInitialized) {
        validPositions = this->mPtrMap->getValidSnakePositions(this->mInitialSnakeLength, 3);
        
        if (!validPositions.empty()) {
            int idx = std::rand() % validPositions.size();
            auto [startPos, direction] = validPositions[idx];
            this->mPtrSnake->initializeSnake(startPos.getX(), startPos.getY(), direction);
            snakeInitialized = true;
        }
    }
    
    // 最后的备选方案，使用中心位置
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
            this->mDifficulty = 0;  // 速度关卡难度降低，让蛇移动更慢
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
    // 第二关速度调慢：延迟为基础延迟的2倍
    if (level == 2) {
        this->mDelay = this->mBaseDelay * 2.0;
    } else {
        this->mDelay = this->mBaseDelay * pow(0.75, this->mDifficulty);
    }
    
    // 显示开场介绍（除非是重试）
    if (!mIsLevelRetry) {
        this->displayLevelIntroduction(level);
    }
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
    
    // 第四关使用单键转向模式（智能判断方向）
    this->mPtrSnake->setTurnMode(TurnMode::SingleKey);
    
    // 设置蛇的初始位置（从迷宫的入口开始）
    this->mPtrSnake->initializeSnake(1, 1, InitialDirection::Right);
    
    // 设置终点位置（迷宫出口）
    //mEndpoint = SnakeBody(93, 40);

    mHasEndpoint = true;
    
    // 第四关不需要创建食物，直接将食物设置在不可能到达的位置
    mFood = SnakeBody(0, 0);
    this->mPtrSnake->senseFood(this->mFood);
    
    // 设置难度
    this->mDifficulty = 1;
    this->mDelay = this->mBaseDelay * pow(2.3, this->mDifficulty); // 将系数从2.0增加到3.0使蛇移动更慢
    this->mPoints = 0;
    
    // 启用视窗跟随功能（第四关专用）
    mUseViewport = true;
    mViewOffsetX = 0;
    mViewOffsetY = 0;
}

void Game::renderEndpoint() const
{
    if (!mHasEndpoint) return;
    
    // 计算终点在窗口中的坐标
    int x = mEndpoint.getX();
    int y = mEndpoint.getY();
    
    // 如果启用视窗跟随，将坐标转换为窗口相对坐标
    if (mUseViewport)
    {
        x -= mViewOffsetX;
        y -= mViewOffsetY;
    }
    
    // 只有在窗口范围内才绘制终点标记
    if (x >= 0 && x < this->mGameBoardWidth && y >= 0 && y < this->mGameBoardHeight)
    {
        mvwaddch(this->mWindows[1], y, x, mEndpointSymbol);
    }
}

// 第四关蛇的控制
void Game::controlSnakeLevel4() const
{
    int key;
    key = getch();
    
    if (key == mSingleKeyTurnSymbol || key == 't' || key == ' ')
    {
        // 手动触发智能转向
        this->mPtrSnake->singleKeyTurn();
    }
}

// 运行第四关特殊逻辑
void Game::runLevel4()
{
    // 播放背景音乐 (Linux系统)
    std::string musicPath = "assets/music/level4music.mp3";
    // 检查音乐文件是否存在
    std::ifstream checkMusic(musicPath);
    if (!checkMusic.good()) {
        // 如果assets目录下没有找到，尝试根目录
        musicPath = "level4music.mp3";
        checkMusic.close();
        checkMusic.open(musicPath);
        if (!checkMusic.good()) {
            mvwprintw(this->mWindows[0], 4, 1, "提示: 未找到音乐文件");
            wrefresh(this->mWindows[0]);
            musicPath = "";
        }
    }
    checkMusic.close();
    
    if (!musicPath.empty()) {
        // 尝试多种音乐播放器，按优先级排列
        std::vector<std::string> players = {
            "mpg123 -q -l 0",
            "ffplay -nodisp -loop 0",
            "mplayer -loop 0 -really-quiet",
            "cvlc --play-and-exit --loop",
            "aplay",  // 仅支持wav
            "paplay"  // 仅支持wav
        };
        
        bool musicStarted = false;
        for (const auto& player : players) {
            // 检查播放器是否安装
            std::string checkCmd = "which " + player.substr(0, player.find(' ')) + " 2>/dev/null";
            FILE* checkPlayer = popen(checkCmd.c_str(), "r");
            bool playerInstalled = false;
            
            if (checkPlayer != NULL) {
                char buffer[128];
                if (fgets(buffer, sizeof(buffer), checkPlayer) != NULL) {
                    playerInstalled = true;
                }
                pclose(checkPlayer);
            }
            
            if (playerInstalled) {
                std::string musicCmd = player + " \"" + musicPath + "\" >/dev/null 2>&1 &";
                try {
                    system(musicCmd.c_str());
                    mvwprintw(this->mWindows[0], 4, 1, "音乐已启动 (使用%s)", 
                              player.substr(0, player.find(' ')).c_str());
                    wrefresh(this->mWindows[0]);
                    musicStarted = true;
                    break;
                } catch (...) {
                    continue; // 尝试下一个播放器
                }
            }
        }
        
        if (!musicStarted) {
            mvwprintw(this->mWindows[0], 4, 1, "提示: 未找到可用的音频播放器");
            wrefresh(this->mWindows[0]);
        }
    }
    
    // 更新信息面板，显示关卡提示
    mvwprintw(this->mWindows[0], 1, 1, "Level 4: Smart Path Challenge");
    mvwprintw(this->mWindows[0], 2, 1, "Press 'T' or SPACE to turn automatically");
    mvwprintw(this->mWindows[0], 3, 1, "Reach the 'X' mark to win!");
    wrefresh(this->mWindows[0]);
    
    // 确保侧边栏正确显示
    this->renderInstructionBoard();
    this->renderPoints();
    this->renderLevel();
    
    while (true)
    {
        // 使用单键控制
        this->controlSnakeLevel4();
        
        // 更新视窗位置（让蛇居中）
        this->updateViewport();
        
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
    
    // 停止背景音乐 (Linux系统) - 尝试停止各种可能的音频播放器
    try {
        system("pkill -f mpg123 >/dev/null 2>&1");
        system("pkill -f ffplay >/dev/null 2>&1");
        system("pkill -f mplayer >/dev/null 2>&1");
        system("pkill -f vlc >/dev/null 2>&1");
        system("pkill -f aplay >/dev/null 2>&1");
        system("pkill -f paplay >/dev/null 2>&1");
    } catch (...) {
        // 忽略停止音乐时的错误
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
    // 第二关特殊逻辑：需要在时间限制内达到目标分数
    if (mCurrentLevel == 2) {
        return (mPoints >= mLevelTargetPoints);
    }
    
    // 其他关卡正常逻辑
    return (mPoints >= mLevelTargetPoints);
}

void Game::runLevel()
{
    // 确保初始化时侧边栏正确显示
    this->renderInstructionBoard();
    this->renderPoints();
    this->renderLevel();
    
    // 设置非阻塞模式，确保游戏不会在等待输入时卡住
    nodelay(stdscr, TRUE);
    
    // 如果是第三关，根据模式选择使用不同的运行逻辑
    if (mCurrentLevel == 3) {
        if (mLevel3ModeChoice == 0) {
            this->runLevel3Mode1(); // 模式一：镜像之舞
        } else {
            this->runLevel3Mode2(); // 模式二：协作模式
        }
        return;
    }
    
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
    
    // 添加一个准备阶段，让玩家有时间反应
    {
        // 渲染当前状态，让玩家看到蛇的初始位置
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        this->renderMap();
        this->renderSnake();
        this->renderFood();
        
        // 创建一个倒计时窗口
        WINDOW* countdownWin;
        int width = 24;
        int height = 5;
        int startX = (this->mGameBoardWidth - width) / 2;
        int startY = (this->mGameBoardHeight - height) / 2 + this->mInformationHeight;
        
        countdownWin = newwin(height, width, startY, startX);
        box(countdownWin, 0, 0);
        mvwprintw(countdownWin, 0, 8, "GET READY");
        
        // 倒计时3秒
        for (int i = 3; i > 0; i--) {
            mvwprintw(countdownWin, 2, 9, "READY: %d", i);
            wrefresh(countdownWin);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        mvwprintw(countdownWin, 2, 10, "GO!   ");
        wrefresh(countdownWin);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 删除倒计时窗口
        delwin(countdownWin);
    }
    
    // 为第二关设置时间计时
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    const int timeLimitSeconds = 30; // 第二关时间限制：30秒
    bool hasTimeLimit = (mCurrentLevel == 2);
    
    if (hasTimeLimit) {
        startTime = std::chrono::steady_clock::now();
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
        
        // 检查时间限制（第二关）
        if (hasTimeLimit) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                currentTime - startTime).count();
            int remainingTime = timeLimitSeconds - static_cast<int>(elapsedSeconds);
            
            // 在游戏面板上显示剩余时间
            mvwprintw(this->mWindows[1], 1, 1, "Time: %d s ", remainingTime);
            
            // 如果时间到，结束游戏
            if (remainingTime <= 0) {
                // 时间到但没有达到目标分数，关卡失败
                if (mPoints < mLevelTargetPoints) {
                    mvwprintw(this->mWindows[1], this->mGameBoardHeight / 2, this->mGameBoardWidth / 2 - 10, "TIME'S UP! FAILED!");
                    wrefresh(this->mWindows[1]);
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    break;
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
    int offset = 2; // 减小偏移，使菜单更紧凑
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
        mIsLevelRetry = false; // 首次选择关卡时，重置重试标志
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
        this->renderPoints();
        this->renderTimer(); // 在每一帧都渲染计时器
        
        // 在游戏界面上显示剩余时间
        mvwprintw(this->mWindows[1], 1, 1, "Time: %d s ", mTimeRemaining);

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
    int offset = 2; // 减小偏移，使菜单更紧凑
    mvwprintw(menu, 1, 1, "Select Battle Type:");
    wattron(menu, A_STANDOUT);
    mvwprintw(menu, 0 + offset, 1, "%s", menuItems[0].c_str());
    wattroff(menu, A_STANDOUT);
    for (size_t i = 1; i < menuItems.size(); i++) {
        mvwprintw(menu, static_cast<int>(i) + offset, 1, "%s", menuItems[i].c_str());
    }
    wrefresh(menu);

    int key;
    while (true) {
        key = getch();
        switch(key) {
            case 'W': case 'w': case KEY_UP:
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                index = (index - 1 + static_cast<int>(menuItems.size())) % static_cast<int>(menuItems.size());
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            case 'S': case 's': case KEY_DOWN:
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
                index = (index + 1) % static_cast<int>(menuItems.size());
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, "%s", menuItems[index].c_str());
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

// 显示关卡通关后的文字叙述
void Game::displayLevelCompletion(int level)
{
    // 清除屏幕并刷新，以确保界面正确显示
    werase(this->mWindows[1]);
    box(this->mWindows[1], 0, 0);
    wrefresh(this->mWindows[1]);
    
    // 创建一个窗口用于显示通关文字
    WINDOW* completeWin;
    int width = this->mGameBoardWidth * 0.85; // 略微增大宽度
    int height = this->mGameBoardHeight * 0.75; // 略微增大高度
    int startX = this->mGameBoardWidth * 0.075; // 调整X坐标保持居中
    int startY = this->mGameBoardHeight * 0.1 + this->mInformationHeight; // 向上移动20像素

    completeWin = newwin(height, width, startY, startX);
    box(completeWin, 0, 0);
    
    // 设置标题
    std::string title = "LEVEL " + std::to_string(level) + " COMPLETED";
    mvwprintw(completeWin, 1, (width - title.length()) / 2, "%s", title.c_str());
    
    // 根据关卡显示不同的通关文字
    std::vector<std::string> completionText;
    
    switch (level) {
        case 1:
            completionText = {
                "An eternity could have passed.",
                "In this endless solitude, you were not consumed.",
                "You forged your sorrow into sustenance and your confusion into strength. Every bite was a declaration of your will to live; every inch of growth was an answer to your parents' lesson.",
                "You are no longer the frightened hatchling cowering in the dark. Your spirit has been tempered, as hard and unbreakable as a diamond.",
                "",
                "[Item Acquired]: *Memory Shard - The Heart of Fortitude*",
                "",
                "This is the first gift your parents left you—the \"Heart of Fortitude.\"",
                "As it appears, the stagnant space around you begins to tremble. Like a compass, the shard points you toward a new direction, toward a rift engulfed in a chaotic temporal storm.",
                "You understand now. The trial... has only just begun.",
            };
            break;
        case 2:
            completionText = {
                "You succeeded.",
                "In this race against the clock, you learned to maintain focus and precision under extreme pressure.",
                "You no longer fear the fleeting moments; instead, you have come to understand their rhythm. You now grasp the cold, efficient terror of The Mechanos—and have found a way to navigate its laws.",
                "",
                "[Item Acquired]: *Memory Shard - The Heart of Celerity*",
                "",
                "The two shards circle you, their powers beginning to resonate.",
                "No longer pointing to a random rift, they now work in concert to open a new gateway—a path to the deepest corner of your heart, to the hallowed ground where memory and longing reside.",
            };
            break;
        case 3:
            completionText = {
                "[Item Acquired]: *Memory Shard - The Heart of Bonds*",
                "Fortitude, Celerity, Bonds.",
                "The three hearts orbit you. Their combined power allows you to perceive something greater, a law more fundamental and profound—a concept The Mechanos could never comprehend: the very heartbeat of life itself.",
                
            };
            break;
        case 4:
            completionText = {
                "You are no longer just following the beat; you have become a part of it.",
                "You have learned that the power of life lies not in conquest, but in harmonious resonance. This fluid, ever-changing, beautiful rhythm is the ultimate counter to the cold, rigid, and repetitive order of The Mechanos.",
                "Your very existence is now a hymn to life.",
                "[Animation]: The fourth and final shard is born from the resonance of the world's melody. It is iridescent, seeming to contain all the beauty and vitality of the universe.",
                "[Item Acquired]: *Memory Shard - The Heart of Harmony*",
                "The four shards are finally complete!",
                "They unite before you, sending a pillar of light into the sky. It tears through the veil of spacetime, forging a road that leads directly to the eye of the storm.",
                "At the end of that road lies the core of The Mechanos: the Zero Throne.",
            };
            break;
        case 5:
            completionText = {
                "It is over.",
                "The spinning lasers die down, the cold laws collapse.",
                "You have answered the silence of machines with the resonance of life. You have shattered eternal stillness with a harmonious melody.",
                "Now, it is time for this imprisoned world to be reborn.",
                "You watch the new world before you, where all things are reborn and thrive.",
                "You can almost see the proud smiles of your parents. They are not gone; they have become every blade of grass, every gust of wind that brushes against your scales.",
                "The hatred has long since faded, replaced by the weight of love and responsibility.",
                "You are no longer an echo of the past, but the prelude to the future.",
            };
            break;
        default:
            completionText = {
                "CHALLENGE COMPLETE",
                "",
                "You have overcome this trial, but the journey continues...",
                "",
                "What awaits in the next challenge?"
            };
    }
    
    // 实现文本自动换行的显示效果
    const int maxDisplayWidth = width - 6; // 可显示的最大宽度（留边距）
    
    // 辅助函数：将长文本按单词分割成适合宽度的多行
    auto wrapText = [maxDisplayWidth](const std::string& text) -> std::vector<std::string> {
        std::vector<std::string> wrappedLines;
        if (text.empty()) {
            wrappedLines.push_back("");
            return wrappedLines;
        }
        
        std::istringstream wordStream(text);
        std::string word;
        std::string currentLine;
        
        while (wordStream >> word) {
            // 如果加上这个词会超出宽度，且当前行不为空，则另起一行
            if (currentLine.length() + word.length() + 1 > static_cast<size_t>(maxDisplayWidth) && !currentLine.empty()) {
                wrappedLines.push_back(currentLine);
                currentLine = word;
            }
            // 如果是第一个词或者可以加入当前行
            else {
                if (!currentLine.empty()) {
                    currentLine += " ";
                }
                currentLine += word;
            }
        }
        
        if (!currentLine.empty()) {
            wrappedLines.push_back(currentLine);
        }
        
        return wrappedLines;
    };
    
    // 设置getch为非阻塞模式，以便检测按键
    nodelay(stdscr, TRUE);
    
    // 在窗口底部显示跳过提示
    mvwprintw(completeWin, height - 2, 2, "Press ESC to skip");
    wrefresh(completeWin);
    
    bool skip = false;
    
    // 显示每一行原始文字（可能会自动换行）
    for (size_t i = 0; i < completionText.size(); i++) {
        // 检查是否按下ESC键跳过
        int ch = getch();
        if (ch == 27) { // ESC键的ASCII码是27
            skip = true;
            break;
        }
        
        const std::string& originalLine = completionText[i];
        
        // 如果是空行，只显示很短的时间
        if (originalLine.empty()) {
            // 清除显示区域
            for (int y = 3; y < height - 3; y++) {
                wmove(completeWin, y, 2);
                for (int x = 2; x < width - 2; x++) {
                    waddch(completeWin, ' ');
                }
            }
            wrefresh(completeWin);
            
            // 在等待期间检查是否按下ESC键
            auto start = std::chrono::steady_clock::now();
            while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < 300) {
                ch = getch();
                if (ch == 27) {
                    skip = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (skip) break;
            continue;
        }
        
        // 将原始行按需换行
        std::vector<std::string> wrappedLines = wrapText(originalLine);
        
        // 清除显示区域
        for (int y = 3; y < height - 3; y++) {
            wmove(completeWin, y, 2);
            for (int x = 2; x < width - 2; x++) {
                waddch(completeWin, ' ');
            }
        }
        
        // 计算起始行，使文本垂直居中
        int startLine = (height - wrappedLines.size()) / 2;
        if (startLine < 3) startLine = 3;
        
        // 逐行显示包装后的文本
        for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
            const std::string& line = wrappedLines[lineIdx];
            
            // 计算当前行的水平居中位置
            int startPos = (width - line.length()) / 2;
            if (startPos < 2) startPos = 2;
            
            // 逐个字符显示当前行
            for (size_t j = 0; j < line.length(); j++) {
                mvwaddch(completeWin, startLine + static_cast<int>(lineIdx), startPos + static_cast<int>(j), line[j]);
                wrefresh(completeWin);
                
                // 每个字符显示后短暂暂停，同时检查是否按下ESC键
                ch = getch();
                if (ch == 27) {
                    skip = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            
            if (skip) break;
        }
        
        if (skip) break;
        
        // 显示完整行后等待一段时间，同时检查是否按下ESC键
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start).count() < 2000) {
            ch = getch();
            if (ch == 27) {
                skip = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        if (skip) break;
    }
    
    // 如果跳过，立即显示所有文本的最后一行
    if (skip && !completionText.empty()) {
        // 清除显示区域
        for (int y = 3; y < height - 3; y++) {
            wmove(completeWin, y, 2);
            for (int x = 2; x < width - 2; x++) {
                waddch(completeWin, ' ');
            }
        }
        
        // 找到最后一个非空行
        std::string lastLine;
        for (auto it = completionText.rbegin(); it != completionText.rend(); ++it) {
            if (!it->empty()) {
                lastLine = *it;
                break;
            }
        }
        
        if (!lastLine.empty()) {
            // 将最后一行按需换行
            std::vector<std::string> wrappedLines = wrapText(lastLine);
            
            // 计算起始行，使文本垂直居中
            int startLine = (height - wrappedLines.size()) / 2;
            if (startLine < 3) startLine = 3;
            
            // 显示最后一行
            for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
                const std::string& line = wrappedLines[lineIdx];
                
                // 计算当前行的水平居中位置
                int startPos = (width - line.length()) / 2;
                if (startPos < 2) startPos = 2;
                
                mvwprintw(completeWin, startLine + static_cast<int>(lineIdx), startPos, "%s", line.c_str());
            }
            wrefresh(completeWin);
        }
    }
    
    // 恢复getch为阻塞模式
    nodelay(stdscr, FALSE);
    
    // 如果没有跳过，清除所有文本
    if (!skip) {
        for (int y = 3; y < height - 3; y++) {
            wmove(completeWin, y, 2);
            for (int x = 2; x < width - 2; x++) {
                waddch(completeWin, ' ');
            }
        }
    }
    
    // 显示按键提示（替换ESC跳过提示）
    mvwprintw(completeWin, height - 2, 2, "Press SPACE to continue...                ");
    wrefresh(completeWin);
    
    // 等待用户按空格键继续
    int key;
    while (true) {
        key = getch();
        if (key == ' ' || key == 10) // 空格键或回车键
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 删除窗口
    delwin(completeWin);
    
    // 重新绘制游戏界面
    werase(this->mWindows[1]);
    box(this->mWindows[1], 0, 0);
    wrefresh(this->mWindows[1]);
}

// 实现第三关模式一：镜像之舞
void Game::runLevel3Mode1()
{
    // 确保初始化时侧边栏正确显示
    this->renderInstructionBoard();
    this->renderPoints();
    this->renderLevel();
    
    // 初始化第三关模式一的固定食物位置
    this->initializeLevel3Mode1Foods();
    // 设置第一个食物
    this->setNextLevel3Mode1Food();
    this->mPtrSnake->senseFood(this->mFood);
    
    // 降低蛇的移动速度 - 将延迟增加为原来的1.5倍
    this->mDelay = this->mBaseDelay * 1.5;
    
    // 设置非阻塞模式，确保游戏不会在等待输入时卡住
    nodelay(stdscr, TRUE);
    
    // 添加一个准备阶段，让玩家有时间反应
    {
        // 渲染当前状态，让玩家看到蛇的初始位置
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        this->renderMap();
        this->renderSnake();
        this->renderFood();
        
        // 创建一个倒计时窗口
        WINDOW* countdownWin;
        int width = 24;
        int height = 5;
        int startX = (this->mGameBoardWidth - width) / 2;
        int startY = (this->mGameBoardHeight - height) / 2 + this->mInformationHeight;
        
        countdownWin = newwin(height, width, startY, startX);
        box(countdownWin, 0, 0);
        mvwprintw(countdownWin, 0, 8, "GET READY");
        
        // 倒计时3秒
        for (int i = 3; i > 0; i--) {
            mvwprintw(countdownWin, 2, 9, "READY: %d", i);
            wrefresh(countdownWin);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        mvwprintw(countdownWin, 2, 10, "GO!   ");
        wrefresh(countdownWin);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 删除倒计时窗口
        delwin(countdownWin);
    }
    
    // 创建影子蛇 - 玩家的镜像
    std::unique_ptr<Snake> shadowSnake = std::make_unique<Snake>(this->mGameBoardWidth, this->mGameBoardHeight, this->mInitialSnakeLength);
    shadowSnake->setMap(this->mPtrMap.get());
    
    // 获取当前蛇的位置和方向
    int playerX = this->mPtrSnake->getSnake()[0].getX();
    int playerY = this->mPtrSnake->getSnake()[0].getY();
    Direction playerDir = this->mPtrSnake->getDirection();
    
    // 计算镜像位置 - 以游戏区域中央垂直轴为对称轴
    int centerX = this->mGameBoardWidth / 2;
    // 关于垂直中轴线的对称 (x坐标对称，y坐标不变)
    int shadowX = 2 * centerX - playerX;
    int shadowY = playerY;
    
    // 计算镜像方向
    Direction shadowDir;
    switch (playerDir) {
        case Direction::Up:
            shadowDir = Direction::Up; // 上下方向不变
            break;
        case Direction::Down:
            shadowDir = Direction::Down; // 上下方向不变
            break;
        case Direction::Left:
            shadowDir = Direction::Right; // 左右方向对称
            break;
        case Direction::Right:
            shadowDir = Direction::Left; // 左右方向对称
            break;
    }
    
    // 初始化影子蛇 - 需要转换为InitialDirection枚举
    InitialDirection shadowInitDir;
    switch (shadowDir) {
        case Direction::Up:
            shadowInitDir = InitialDirection::Up;
            break;
        case Direction::Down:
            shadowInitDir = InitialDirection::Down;
            break;
        case Direction::Left:
            shadowInitDir = InitialDirection::Left;
            break;
        case Direction::Right:
            shadowInitDir = InitialDirection::Right;
            break;
    }
    shadowSnake->initializeSnake(shadowX, shadowY, shadowInitDir);
    
    // 游戏主循环
    while (true)
    {
        // 处理玩家输入
        this->controlSnake();
        
        // 清除游戏区域
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 渲染地图
        this->renderMap();
        
        // 保存食物的当前位置
        int foodX = this->mFood.getX();
        int foodY = this->mFood.getY();
        
        // 移动玩家蛇
        bool playerEatFood = this->mPtrSnake->moveFoward();
        bool playerCollision = this->mPtrSnake->checkCollision();
        
        // 获取玩家蛇的新方向
        Direction playerNewDir = this->mPtrSnake->getDirection();
        
        // 计算影子蛇的镜像方向
        Direction shadowNewDir;
        switch (playerNewDir) {
            case Direction::Up:
                shadowNewDir = Direction::Up; // 上下方向不变
                break;
            case Direction::Down:
                shadowNewDir = Direction::Down; // 上下方向不变
                break;
            case Direction::Left:
                shadowNewDir = Direction::Right; // 左右方向对称
                break;
            case Direction::Right:
                shadowNewDir = Direction::Left; // 左右方向对称
                break;
        }
        
        // 设置影子蛇的方向
        shadowSnake->changeDirection(shadowNewDir);
        
        // 移动影子蛇
        shadowSnake->moveFoward();
        bool shadowCollision = shadowSnake->checkCollision();
        
        // 手动检查影子蛇是否吃到食物
        bool shadowEatFood = false;
        if (!playerEatFood) { // 如果玩家没有吃到食物，检查影子蛇是否吃到
            const SnakeBody& shadowHead = shadowSnake->getSnake()[0];
            if (shadowHead.getX() == foodX && shadowHead.getY() == foodY) {
                shadowEatFood = true;
            }
        }
        
        // 检查玩家或影子是否碰撞
        if (playerCollision || shadowCollision)
        {
            // 游戏结束
            break;
        }
        
        // 处理吃食物 - 玩家蛇或镜像蛇任一吃到食物都算
        if (playerEatFood || shadowEatFood)
        {
            this->mPoints += 1;
            // 使用固定食物列表而不是随机生成
            this->setNextLevel3Mode1Food();
            this->mPtrSnake->senseFood(this->mFood);
            shadowSnake->senseFood(this->mFood);
            
            // 检查是否完成关卡目标
            if (this->isLevelCompleted())
            {
                // 如果达到目标分数，关卡通过
                this->renderFood();
                this->renderPoints();
                this->renderLevel();
                refresh();
                break;
            }
        }
        
        // 渲染玩家蛇
        this->renderSnake();
        
        // 渲染影子蛇 - 使用不同的符号
        const std::vector<SnakeBody>& shadowBody = shadowSnake->getSnake();
        for (const auto& segment : shadowBody) {
            mvwaddch(this->mWindows[1], segment.getY(), segment.getX(), '%');
        }
        
        // 渲染食物和状态信息
        this->renderFood();
        this->renderPoints();
        this->renderLevel();
        
        // 添加提示文字
        mvwprintw(this->mWindows[1], 1, 1, "Mirror Dance: Watch your shadow!");
        
        // 游戏延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        
        refresh();
    }
}

// 初始化第三关模式一的10个固定食物位置
void Game::initializeLevel3Mode1Foods()
{
    // 清空旧的食物列表
    mLevel3Mode1Foods.clear();
    mLevel3FoodIndex = 0;
    

    mLevel3Mode1Foods.push_back(SnakeBody(7, 4));
    mLevel3Mode1Foods.push_back(SnakeBody(50, 4));
    mLevel3Mode1Foods.push_back(SnakeBody(3, 8));
    mLevel3Mode1Foods.push_back(SnakeBody(59, 8));
    mLevel3Mode1Foods.push_back(SnakeBody(10, 13));
    mLevel3Mode1Foods.push_back(SnakeBody(47, 10));
    mLevel3Mode1Foods.push_back(SnakeBody(15, 10));
    mLevel3Mode1Foods.push_back(SnakeBody(20, 6));

    


}

// 设置下一个固定食物
void Game::setNextLevel3Mode1Food()
{
    if (mLevel3Mode1Foods.empty())
    {
        // 如果食物列表为空，则创建随机食物作为备选
        createRamdonFood();
        return;
    }
    
    // 设置当前索引对应的食物
    mFood = mLevel3Mode1Foods[mLevel3FoodIndex];
    
    // 更新索引，循环使用食物列表
    mLevel3FoodIndex = (mLevel3FoodIndex + 1) % mLevel3Mode1Foods.size();
}

// 实现第三关模式二：协作模式
void Game::runLevel3Mode2()
{
    // 确保初始化时侧边栏正确显示
    this->renderInstructionBoard();
    this->renderPoints();
    this->renderLevel();
    
    // 显示模式二的开场叙述
    {
        // 清除游戏区域并刷新，以准备显示介绍文字
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        wrefresh(this->mWindows[1]);
        
        // 创建一个窗口用于显示开场文字
        WINDOW* introWin;
        int width = this->mGameBoardWidth * 0.85; // 略微增大宽度
        int height = this->mGameBoardHeight * 0.7; // 增大高度
        int startX = this->mGameBoardWidth * 0.075; // 调整X坐标保持居中
        int startY = this->mGameBoardHeight * 0.15 + this->mInformationHeight; // 向上移动20像素

        introWin = newwin(height, width, startY, startX);
        box(introWin, 0, 0);
        
        // 设置标题
        std::string title = "THE BOND OF TRUST";
        mvwprintw(introWin, 1, (width - title.length()) / 2, "%s", title.c_str());
        
        // 开场叙述文本
        std::vector<std::string> introText = {
            "In that seamless coordination, you felt the power of trust.",
            "Your parents' will acknowledged your companion, entrusting them with the duty of a guardian.",
            "You shared both risk and reward, protecting each other to claim victory.",
            "",
            "You understood that the bonds of family can extend into a wider trust.",
            "Entrusting your back to a friend is its own form of courage and growth."
        };
        
        // 实现文本自动换行的显示效果
        const int maxDisplayWidth = width - 6; // 可显示的最大宽度（留边距）
        
        // 辅助函数：将长文本按单词分割成适合宽度的多行
        auto wrapText = [maxDisplayWidth](const std::string& text) -> std::vector<std::string> {
            std::vector<std::string> wrappedLines;
            if (text.empty()) {
                wrappedLines.push_back("");
                return wrappedLines;
            }
            
            std::istringstream wordStream(text);
            std::string word;
            std::string currentLine;
            
            while (wordStream >> word) {
                // 如果加上这个词会超出宽度，且当前行不为空，则另起一行
                if (currentLine.length() + word.length() + 1 > static_cast<size_t>(maxDisplayWidth) && !currentLine.empty()) {
                    wrappedLines.push_back(currentLine);
                    currentLine = word;
                }
                // 如果是第一个词或者可以加入当前行
                else {
                    if (!currentLine.empty()) {
                        currentLine += " ";
                    }
                    currentLine += word;
                }
            }
            
            if (!currentLine.empty()) {
                wrappedLines.push_back(currentLine);
            }
            
            return wrappedLines;
        };
        
        // 设置getch为非阻塞模式，以便检测按键
        nodelay(stdscr, TRUE);
        
        // 在窗口底部显示跳过提示
        mvwprintw(introWin, height - 2, 2, "Press ESC to skip");
        wrefresh(introWin);
        
        bool skip = false;
        
        // 显示每一行原始文字（可能会自动换行）
        for (size_t i = 0; i < introText.size(); i++) {
            // 检查是否按下ESC键跳过
            int ch = getch();
            if (ch == 27) { // ESC键的ASCII码是27
                skip = true;
                break;
            }
            
            const std::string& originalLine = introText[i];
            
            // 如果是空行，只显示很短的时间
            if (originalLine.empty()) {
                // 清除显示区域
                for (int y = 3; y < height - 3; y++) {
                    wmove(introWin, y, 2);
                    for (int x = 2; x < width - 2; x++) {
                        waddch(introWin, ' ');
                    }
                }
                wrefresh(introWin);
                
                // 在等待期间检查是否按下ESC键
                auto start = std::chrono::steady_clock::now();
                while (std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - start).count() < 300) {
                    ch = getch();
                    if (ch == 27) {
                        skip = true;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                if (skip) break;
                continue;
            }
            
            // 将原始行按需换行
            std::vector<std::string> wrappedLines = wrapText(originalLine);
            
            // 清除显示区域
            for (int y = 3; y < height - 3; y++) {
                wmove(introWin, y, 2);
                for (int x = 2; x < width - 2; x++) {
                    waddch(introWin, ' ');
                }
            }
            
            // 计算起始行，使文本垂直居中
            int startLine = (height - wrappedLines.size()) / 2;
            if (startLine < 3) startLine = 3;
            
            // 逐行显示包装后的文本
            for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
                const std::string& line = wrappedLines[lineIdx];
                
                // 计算当前行的水平居中位置
                int startPos = (width - line.length()) / 2;
                if (startPos < 2) startPos = 2;
                
                // 逐个字符显示当前行
                for (size_t j = 0; j < line.length(); j++) {
                    mvwaddch(introWin, startLine + static_cast<int>(lineIdx), startPos + static_cast<int>(j), line[j]);
                    wrefresh(introWin);
                    
                    // 每个字符显示后短暂暂停，同时检查是否按下ESC键
                    ch = getch();
                    if (ch == 27) {
                        skip = true;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }
                
                if (skip) break;
            }
            
            if (skip) break;
            
            // 显示完整行后等待一段时间，同时检查是否按下ESC键
            auto start = std::chrono::steady_clock::now();
            while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < 2000) {
                ch = getch();
                if (ch == 27) {
                    skip = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            
            if (skip) break;
        }
        
        // 如果跳过，立即显示所有文本的最后一行
        if (skip && !introText.empty()) {
            // 清除显示区域
            for (int y = 3; y < height - 3; y++) {
                wmove(introWin, y, 2);
                for (int x = 2; x < width - 2; x++) {
                    waddch(introWin, ' ');
                }
            }
            
            // 找到最后一个非空行
            std::string lastLine;
            for (auto it = introText.rbegin(); it != introText.rend(); ++it) {
                if (!it->empty()) {
                    lastLine = *it;
                    break;
                }
            }
            
            if (!lastLine.empty()) {
                // 将最后一行按需换行
                std::vector<std::string> wrappedLines = wrapText(lastLine);
                
                // 计算起始行，使文本垂直居中
                int startLine = (height - wrappedLines.size()) / 2;
                if (startLine < 3) startLine = 3;
                
                // 显示最后一行
                for (size_t lineIdx = 0; lineIdx < wrappedLines.size(); lineIdx++) {
                    const std::string& line = wrappedLines[lineIdx];
                    
                    // 计算当前行的水平居中位置
                    int startPos = (width - line.length()) / 2;
                    if (startPos < 2) startPos = 2;
                    
                    mvwprintw(introWin, startLine + static_cast<int>(lineIdx), startPos, "%s", line.c_str());
                }
                wrefresh(introWin);
            }
        }
        
        // 恢复getch为阻塞模式
        nodelay(stdscr, FALSE);
        
        // 显示按键提示（替换ESC跳过提示）
        mvwprintw(introWin, height - 2, 2, "Press SPACE to continue...                ");
        wrefresh(introWin);
        
        // 等待用户按空格键继续
        int key;
        while (true) {
            key = getch();
            if (key == ' ' || key == 10) // 空格键或回车键
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 删除窗口
        delwin(introWin);
        
        // 重新绘制游戏界面
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        wrefresh(this->mWindows[1]);
    }
    
    // 创建地图
    mPtrMap = std::make_unique<Map>(mGameBoardWidth, mGameBoardHeight);
    mPtrMap->loadDefaultMap();
    
    // 创建两条蛇
    this->mPtrSnake.reset(new Snake(mGameBoardWidth, mGameBoardHeight, mInitialSnakeLength));
    this->mPtrSnake2.reset(new Snake(mGameBoardWidth, mGameBoardHeight, mInitialSnakeLength));
    
    // 设置蛇的起始位置（对角位置）
    this->mPtrSnake->setMap(mPtrMap.get());
    this->mPtrSnake->initializeSnake(5, 5, InitialDirection::Right);
    
    this->mPtrSnake2->setMap(mPtrMap.get());
    this->mPtrSnake2->initializeSnake(mGameBoardWidth - 10, mGameBoardHeight - 10, InitialDirection::Left);
    
    // 创建食物
    this->createRamdonFood();
    this->mPtrSnake->senseFood(this->mFood);
    this->mPtrSnake2->senseFood(this->mFood);
    
    // 重置分数
    this->mPoints = 0;  // 玩家1得分
    this->mPoints2 = 0; // 玩家2得分
    
    // 设置难度
    this->mDelay = this->mBaseDelay * 1.25; // 稍微慢一点，方便协作
    
    // 设置非阻塞模式，确保游戏不会在等待输入时卡住
    nodelay(stdscr, TRUE);
    
    // 添加一个准备阶段，让玩家有时间反应
    {
        // 渲染当前状态，让玩家看到蛇的初始位置
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        this->renderMap();
        
        // 渲染两条蛇
        wattron(this->mWindows[1], COLOR_PAIR(1));
        for (const auto& part : this->mPtrSnake->getSnake()) {
            mvwaddch(this->mWindows[1], part.getY(), part.getX(), mSnakeSymbol);
        }
        wattroff(this->mWindows[1], COLOR_PAIR(1));
        
        wattron(this->mWindows[1], COLOR_PAIR(2));
        for (const auto& part : this->mPtrSnake2->getSnake()) {
            mvwaddch(this->mWindows[1], part.getY(), part.getX(), mSnakeSymbol2);
        }
        wattroff(this->mWindows[1], COLOR_PAIR(2));
        
        this->renderFood();
        
        // 创建一个倒计时窗口
        WINDOW* countdownWin;
        int width = 30;
        int height = 7;
        int startX = (this->mGameBoardWidth - width) / 2;
        int startY = (this->mGameBoardHeight - height) / 2 + this->mInformationHeight;
        
        countdownWin = newwin(height, width, startY, startX);
        box(countdownWin, 0, 0);
        mvwprintw(countdownWin, 0, 9, "TEAM CHALLENGE");
        
        // 显示控制提示
        mvwprintw(countdownWin, 2, 2, "Player 1: WASD");
        mvwprintw(countdownWin, 3, 2, "Player 2: Arrow Keys");
        mvwprintw(countdownWin, 4, 2, "Goal: Collect 10 food together");
        
        wrefresh(countdownWin);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // 倒计时3秒
        for (int i = 3; i > 0; i--) {
            mvwprintw(countdownWin, 5, 13, "READY: %d", i);
            wrefresh(countdownWin);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        mvwprintw(countdownWin, 5, 14, "GO!   ");
        wrefresh(countdownWin);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 删除倒计时窗口
        delwin(countdownWin);
    }
    
    // 游戏主循环
    while (true)
    {
        // 获取用户输入
        int key = getch();
        
        // 控制两条蛇
        if (key != ERR) {
            switch(key) {
                // 玩家1控制 (WASD)
                case 'W': case 'w': mPtrSnake->changeDirection(Direction::Up); break;
                case 'S': case 's': mPtrSnake->changeDirection(Direction::Down); break;
                case 'A': case 'a': mPtrSnake->changeDirection(Direction::Left); break;
                case 'D': case 'd': mPtrSnake->changeDirection(Direction::Right); break;
                
                // 玩家2控制 (方向键)
                case KEY_UP:    mPtrSnake2->changeDirection(Direction::Up); break;
                case KEY_DOWN:  mPtrSnake2->changeDirection(Direction::Down); break;
                case KEY_LEFT:  mPtrSnake2->changeDirection(Direction::Left); break;
                case KEY_RIGHT: mPtrSnake2->changeDirection(Direction::Right); break;
            }
        }
        
        // 清除游戏区域
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 渲染地图
        this->renderMap();
        
        // 移动两条蛇
        bool p1_ate = mPtrSnake->moveFoward();
        bool p2_ate = mPtrSnake2->moveFoward();
        
        // 检查碰撞
        bool p1_collision = mPtrSnake->checkCollision();
        bool p2_collision = mPtrSnake2->checkCollision();
        
        // 检查蛇头之间的碰撞
        const SnakeBody& head1 = mPtrSnake->getSnake().front();
        const SnakeBody& head2 = mPtrSnake2->getSnake().front();
        bool head_collision = (head1.getX() == head2.getX() && head1.getY() == head2.getY());
        
        // 检查蛇是否互相碰撞
        if (!p1_collision) {
            p1_collision = mPtrSnake2->isPartOfSnake(head1.getX(), head1.getY());
        }
        
        if (!p2_collision) {
            p2_collision = mPtrSnake->isPartOfSnake(head2.getX(), head2.getY());
        }
        
        // 如果任一蛇碰撞或蛇头碰撞，结束游戏
        if (p1_collision || p2_collision || head_collision)
        {
            break;
        }
        
        // 处理吃食物
        if (p1_ate || p2_ate)
        {
            if (p1_ate) mPoints++;
            if (p2_ate) mPoints2++;
            
            // 创建新食物
            createRamdonFood();
            mPtrSnake->senseFood(mFood);
            mPtrSnake2->senseFood(mFood);
            
            // 检查总分是否达到目标
            if (mPoints + mPoints2 >= 10)
            {
                // 达到目标分数，关卡通过
                // 在mPoints中存储总分用于显示和判定
                mPoints = mPoints + mPoints2;
                mLevelTargetPoints = 10; // 确保isLevelCompleted正确判定
                
                // 更新显示
                this->renderFood();
                refresh();
                break;
            }
        }
        
        // 渲染两条蛇
        wattron(this->mWindows[1], COLOR_PAIR(1));
        for (const auto& part : this->mPtrSnake->getSnake()) {
            mvwaddch(this->mWindows[1], part.getY(), part.getX(), mSnakeSymbol);
        }
        wattroff(this->mWindows[1], COLOR_PAIR(1));
        
        wattron(this->mWindows[1], COLOR_PAIR(2));
        for (const auto& part : this->mPtrSnake2->getSnake()) {
            mvwaddch(this->mWindows[1], part.getY(), part.getX(), mSnakeSymbol2);
        }
        wattroff(this->mWindows[1], COLOR_PAIR(2));
        
        // 渲染食物
        this->renderFood();
        
        // 显示两个玩家的分数和合计分数
        mvwprintw(this->mWindows[1], 1, 1, "P1: %d | P2: %d | Total: %d/10",
                 mPoints, mPoints2, mPoints + mPoints2);
        
        // 游戏延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        
        refresh();
    }
}

// 添加视窗更新函数实现
void Game::updateViewport()
{
    if (!mUseViewport || mPtrSnake == nullptr || mPtrSnake->getSnake().empty()) return;
    
    // 获取蛇头位置
    const SnakeBody& head = mPtrSnake->getSnake()[0];
    int headX = head.getX();
    int headY = head.getY();
    
    // 计算理想的视窗中心位置（让蛇头居中）
    int idealOffsetX = headX - (mGameBoardWidth / 2);
    int idealOffsetY = headY - (mGameBoardHeight / 2);
    
    // 确保视窗不会超出地图边界
    int maxOffsetX = mPtrMap->getWidth() - mGameBoardWidth;
    int maxOffsetY = mPtrMap->getHeight() - mGameBoardHeight;
    
    // 特殊处理第四关：允许视窗在Y轴方向上超出正常的地图边界
    // 这样即使蛇头到了y=18及以上的位置也能正确显示
    if (mPtrSnake->getTurnMode() == TurnMode::SingleKey) {
        // 对于第四关，不限制Y轴方向的最大偏移
        mViewOffsetX = std::max(0, std::min(idealOffsetX, maxOffsetX));
        
        // Y轴只限制最小值，不限制最大值
        mViewOffsetY = std::max(0, idealOffsetY);
    } else {
        // 其他关卡正常限制视窗边界
        mViewOffsetX = std::max(0, std::min(idealOffsetX, maxOffsetX));
        mViewOffsetY = std::max(0, std::min(idealOffsetY, maxOffsetY));
    }
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
    // 1键：作弊模式
    if (key == '1') {
        if (useItem(ItemType::Cheat)) {
            activateCheatMode();
        }
    }
    // 2键：传送门
    else if (key == '2') {
        if (useItem(ItemType::Portal)) {
            usePortal();
        }
    }
    // 3键：攻击道具
    else if (key == '3') {
        if (mCurrentMode == GameMode::Battle && useItem(ItemType::Attack)) {
            useAttack();
        }
    }
    // 4键：护盾道具
    else if (key == '4') {
        if (useItem(ItemType::Shield) && !isShieldActive()) {
            activateShield();
        }
    }
    // 5键：随机箱
    else if (key == '5') {
        if (useItem(ItemType::RandomBox)) {
            useRandomBox();
        }
    }
    // 每次道具使用后刷新侧边栏
    renderInstructionBoard();
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

// 新增：设置游戏模式
void Game::setGameMode(GameMode mode) {
    mCurrentMode = mode;
}

// 新增：直接启动特定关卡，跳过关卡选择界面
void Game::startLevelDirectly(int level) {
    // 设置关卡模式
    mCurrentMode = GameMode::Level;
    mCurrentLevel = level;
    
    // 初始化ncurses环境
    nodelay(stdscr, TRUE);
    refresh();
    
    // 读取排行榜
    this->readLeaderBoard();
    this->renderBoards();
    
    // 直接运行选定的关卡，跳过selectLevelInLevelMode()
    while (true) {
        // 初始化并运行当前关卡
        this->initializeLevel(mCurrentLevel);
        this->runLevel();
        
        // 检查是否通过当前关卡
        if (this->isLevelCompleted()) {
            // 标记当前关卡为已完成
            this->mLevelStatus[mCurrentLevel - 1] = LevelStatus::Completed;
            
            // 显示通关后的文字叙述
            this->displayLevelCompletion(mCurrentLevel);
            
            // 如果不是最后一关，解锁下一关
            if (mCurrentLevel < mMaxLevel) {
                this->unlockLevel(mCurrentLevel + 1);
            }
            
            // 保存关卡进度
            this->saveLevelProgress();
            
            // 显示通关信息和选择菜单
            WINDOW* levelCompleteWin;
            int width = this->mGameBoardWidth * 0.5;
            int height = 8;
            int startX = this->mGameBoardWidth * 0.25;
            int startY = this->mGameBoardHeight * 0.3 + this->mInformationHeight;
            
            levelCompleteWin = newwin(height, width, startY, startX);
            box(levelCompleteWin, 0, 0);
            
            mvwprintw(levelCompleteWin, 1, 1, "Level %d Completed!", mCurrentLevel);
            if (mCurrentLevel < mMaxLevel) {
                mvwprintw(levelCompleteWin, 3, 1, "1. Continue to Level %d", mCurrentLevel + 1);
                mvwprintw(levelCompleteWin, 4, 1, "2. Return to Level Select (GUI)");
                mvwprintw(levelCompleteWin, 5, 1, "3. Return to Main Menu");
                mvwprintw(levelCompleteWin, 6, 1, "4. Quit Game");
            } else {
                mvwprintw(levelCompleteWin, 3, 1, "All levels completed!");
                mvwprintw(levelCompleteWin, 4, 1, "Congratulations! You've finished all levels!");
                mvwprintw(levelCompleteWin, 5, 1, "1. Return to Level Select (GUI)");
                mvwprintw(levelCompleteWin, 6, 1, "2. Return to Main Menu");
                mvwprintw(levelCompleteWin, 7, 1, "3. Quit Game");
            }
            
            wrefresh(levelCompleteWin);
            
            // 等待用户选择
            int choice = getch();
            delwin(levelCompleteWin);
            
            switch (choice) {
                case '1':
                    if (mCurrentLevel < mMaxLevel) {
                        mCurrentLevel++;
                        continue; // 继续下一关
                    } else {
                        mReturnToModeSelect = true;
                        return; // 返回关卡选择GUI
                    }
                case '2':
                    mReturnToModeSelect = true;
                    return; // 返回关卡选择GUI
                case '3':
                    if (mCurrentLevel < mMaxLevel) {
                        mReturnToModeSelect = true;
                        return; // 返回主菜单
                    } else {
                        mReturnToModeSelect = true;
                        return; // 返回主菜单
                    }
                case '4':
                    if (mCurrentLevel < mMaxLevel) {
                        return; // 退出游戏
                    } else {
                        return; // 退出游戏
                    }
                default:
                    mReturnToModeSelect = true;
                    return; // 默认返回GUI
            }
        } else {
            // 关卡失败，显示失败菜单
            int failureChoice = this->renderLevelFailureMenu();
            
            switch (failureChoice) {
                case 0: // 重试
                    break; // 继续循环，重试当前关卡
                case 1: // 返回关卡选择GUI
                    mReturnToModeSelect = true;
                    return;
                case 2: // 退出游戏
                    return;
                default:
                    mReturnToModeSelect = true;
                    return;
            }
        }
    }
}

void Game::useRandomBox() {
    if (!useItem(ItemType::RandomBox)) return;
    // 随机选择一个效果
    int effect = std::rand() % 6;
    std::string msg;
    switch (effect) {
        case 0: // 作弊模式
            activateCheatMode();
            msg = "RandomBox: Invincible!";
            break;
        case 1: // 传送门
            usePortal();
            msg = "RandomBox: Teleport!";
            break;
        case 2: // 护盾
            activateShield();
            msg = "RandomBox: Shield!";
            break;
        case 3: // 攻击（仅对战模式有效）
            if (mCurrentMode == GameMode::Battle) {
                useAttack();
                msg = "RandomBox: Attack!";
            } else {
                addCoins(5);
                msg = "RandomBox: +5 Coins!";
            }
            break;
        case 4: // 加分
            mPoints += 5;
            msg = "RandomBox: +5 Points!";
            break;
        case 5: // 获得一个随机道具
        {
            int t = std::rand() % 4;
            ItemType it = (ItemType)t; // 0-3: Portal, RandomBox, Cheat, Attack
            addItem(it, 1);
            msg = "RandomBox: Bonus Item!";
            break;
        }
    }
    // 弹窗显示效果
    WINDOW* win = newwin(3, 30, mGameBoardHeight/2 + mInformationHeight, mGameBoardWidth/2 - 15);
    box(win, 0, 0);
    mvwprintw(win, 1, 1, "%s", msg.c_str());
    wrefresh(win);
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    delwin(win);
}


