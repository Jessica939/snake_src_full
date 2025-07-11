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
    // 第四关也解锁，用于测试
    this->mLevelStatus[3] = LevelStatus::Unlocked;
    
    // 加载已保存的关卡进度
    this->loadLevelProgress();
    
    // Create maps directory if it doesn't exist
    std::filesystem::create_directory("maps");
    
    // 创建默认的关卡地图文件
    createDefaultLevelMaps();
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
                                if (x == 0 || y == 0 || x == width - 1 || y == height - 1 || 
                                    (x == width/4 && y < height*3/4) ||
                                    (x == width*3/4 && y > height/4) ||
                                    (y == height/4 && x > width/4 && x < width*3/4) ||
                                    (y == height*3/4 && x > width/4 && x < width*3/4)) {
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
    for (int i = 0; i < this->mWindows.size(); i ++)
    {
        delwin(this->mWindows[i]);
    }
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
    wrefresh(this->mWindows[0]);
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
    mvwprintw(this->mWindows[2], 1, 1, "Manual");

    mvwprintw(this->mWindows[2], 3, 1, "Up: W");
    mvwprintw(this->mWindows[2], 4, 1, "Down: S");
    mvwprintw(this->mWindows[2], 5, 1, "Left: A");
    mvwprintw(this->mWindows[2], 6, 1, "Right: D");

    mvwprintw(this->mWindows[2], 8, 1, "Difficulty");
    mvwprintw(this->mWindows[2], 11, 1, "Points");

    wrefresh(this->mWindows[2]);
}


void Game::renderLeaderBoard() const
{
    // If there is not too much space, skip rendering the leader board 
    if (this->mScreenHeight - this->mInformationHeight - 14 - 2 < 3 * 2)
    {
        return;
    }
    mvwprintw(this->mWindows[2], 14, 1, "Leader Board");
    std::string pointString;
    std::string rank;
    for (int i = 0; i < std::min(this->mNumLeaders, this->mScreenHeight - this->mInformationHeight - 14 - 2); i ++)
    {
        pointString = std::to_string(this->mLeaderBoard[i]);
        rank = "#" + std::to_string(i + 1) + ":";
        mvwprintw(this->mWindows[2], 14 + (i + 1), 1, rank.c_str());
        mvwprintw(this->mWindows[2], 14 + (i + 1), 5, pointString.c_str());
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
    mvwprintw(menu, 2, 1, pointString.c_str());
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
            case 'W':
            case 'w':
            case KEY_UP:
            {
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index --;
                index = (index < 0) ? menuItems.size() - 1 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            }
            case 'S':
            case 's':
            case KEY_DOWN:
            {
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index ++;
                index = (index > menuItems.size() - 1) ? 0 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
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
    mvwprintw(menu, 0 + offset, 1, menuItems[0].c_str());
    wattroff(menu, A_STANDOUT);
    
    for (int i = 1; i < menuItems.size(); i++) {
        mvwprintw(menu, i + offset, 1, menuItems[i].c_str());
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
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index --;
                index = (index < 0) ? menuItems.size() - 1 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            }
            case 'S':
            case 's':
            case KEY_DOWN:
            {
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index ++;
                index = (index > menuItems.size() - 1) ? 0 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
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
    mvwprintw(this->mWindows[2], 12, 1, pointString.c_str());
    wrefresh(this->mWindows[2]);
}

void Game::renderDifficulty() const
{
    std::string difficultyString = std::to_string(this->mDifficulty);
    mvwprintw(this->mWindows[2], 9, 1, difficultyString.c_str());
    wrefresh(this->mWindows[2]);
}

void Game::renderLevel() const
{
    mvwprintw(this->mWindows[2], 15, 1, "Level");
    std::string levelString = std::to_string(this->mCurrentLevel);
    mvwprintw(this->mWindows[2], 16, 1, levelString.c_str());
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
    
    // 创建食物
    this->createRamdonFood();
    this->mPtrSnake->senseFood(this->mFood);
    
    this->mDifficulty = 0;
    this->mPoints = 0;
    this->mDelay = this->mBaseDelay;
}

void Game::createRamdonFood()
{
    // 使用地图提供的空白位置生成食物
    std::vector<SnakeBody> availableGrids;
    if (this->mPtrMap) {
        availableGrids = this->mPtrMap->getEmptyPositions(this->mPtrSnake->getSnake());
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

void Game::renderFood() const
{
    mvwaddch(this->mWindows[1], this->mFood.getY(), this->mFood.getX(), this->mFoodSymbol);
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
    std::vector<SnakeBody>& snake = this->mPtrSnake->getSnake();
    for (int i = 0; i < snakeLength; i ++)
    {
        mvwaddch(this->mWindows[1], snake[i].getY(), snake[i].getX(), this->mSnakeSymbol);
    }
    wrefresh(this->mWindows[1]);
}

void Game::controlSnake() const
{
    int key;
    key = getch();
    
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
    for (int i = 0; i < this->mWindows.size(); i ++)
    {
        werase(this->mWindows[i]);
    }
    this->renderInformationBoard();
    this->renderGameBoard();
    this->renderInstructionBoard();
    for (int i = 0; i < this->mWindows.size(); i ++)
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
        this->mDelay = this->mBaseDelay * pow(0.75, this->mDifficulty);
    }
}

void Game::runGame()
{
    bool moveSuccess;
    int key;
    while (true)
    {
        this->controlSnake();
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 渲染地图
        this->renderMap();
        
        bool eatFood = this->mPtrSnake->moveFoward();
        bool collision = this->mPtrSnake->checkCollision();
        if (collision == true)
        {
            break;
        }
        this->renderSnake();
        if (eatFood == true)
        {
            this->mPoints += 1;
            this->createRamdonFood();
            this->mPtrSnake->senseFood(this->mFood);
            this->adjustDelay();
        }
        this->renderFood();
        this->renderDifficulty();
        this->renderPoints();
        // 即使在普通模式下，也显示当前为第1关
        this->renderLevel();

        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));

        refresh();
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
        switch(mCurrentMode) {
            case GameMode::Classic:
            case GameMode::Timed: {
                
                // 内层循环：负责处理“再来一局”
                bool playAgain = true;
                while (playAgain) {
                    
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
                    
                    playAgain = renderRestartMenu();
                }
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
                        mvwprintw(menu, 2, 1, pointString.c_str());
                        wattron(menu, A_STANDOUT);
                        mvwprintw(menu, 0 + offset, 1, menuItems[0].c_str());
                        wattroff(menu, A_STANDOUT);
                        mvwprintw(menu, 1 + offset, 1, menuItems[1].c_str());
                        mvwprintw(menu, 2 + offset, 1, menuItems[2].c_str());
                        mvwprintw(menu, 3 + offset, 1, menuItems[3].c_str());

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
                                    mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                                    index --;
                                    index = (index < 0) ? menuItems.size() - 1 : index;
                                    wattron(menu, A_STANDOUT);
                                    mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                                    wattroff(menu, A_STANDOUT);
                                    break;
                                }       
                                case 'S':
                                case 's':
                                case KEY_DOWN:
                                {
                                    mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                                    index ++;
                                    index = (index > menuItems.size() - 1) ? 0 : index;
                                    wattron(menu, A_STANDOUT);
                                    mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
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
    
    std::vector<std::string> menuItems = {
        "Classic Mode", 
        "Level Mode",
        "Timed Mode",
        "Exit Game"
    };

    int index = 0;
    int offset = 3;
    mvwprintw(menu, 1, 1, "Select Game Mode:");
    
    wattron(menu, A_STANDOUT);
    mvwprintw(menu, 0 + offset, 1, menuItems[0].c_str());
    wattroff(menu, A_STANDOUT);
    
    for (int i = 1; i < menuItems.size(); i++) {
        mvwprintw(menu, i + offset, 1, menuItems[i].c_str());
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
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index--;
                index = (index < 0) ? menuItems.size() - 1 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                wattroff(menu, A_STANDOUT);
                break;
            }
            case 'S':
            case 's':
            case KEY_DOWN:
            {
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                index++;
                index = (index > menuItems.size() - 1) ? 0 : index;
                wattron(menu, A_STANDOUT);
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
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
    
    // 如果选择退出
    if (index == 3) {
        return false;
    }
    
    // 设置游戏模式
    mCurrentMode = static_cast<GameMode>(index);
    mReturnToModeSelect = false; // 重置返回标志
    
    return true;
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
            break;
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
    
    // 创建食物
    this->createRamdonFood();
    this->mPtrSnake->senseFood(this->mFood);
    
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
    bool moveSuccess;
    int key;
    
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
    
    // 其他关卡的运行逻辑
    while (true)
    {
        this->controlSnake();
        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);
        
        // 渲染地图
        this->renderMap();
        
        bool eatFood = this->mPtrSnake->moveFoward();
        bool collision = this->mPtrSnake->checkCollision();
        
        if (collision == true)
        {
            // 如果碰撞，关卡失败
            break;
        }
        
        this->renderSnake();
        if (eatFood == true)
        {
            this->mPoints += 1;
            this->createRamdonFood();
            this->mPtrSnake->senseFood(this->mFood);
            this->adjustDelay();
            
            // 检查是否完成关卡目标
            if (this->isLevelCompleted())
            {
                // 如果达到目标分数，关卡通过
                this->renderFood();
                this->renderDifficulty();
                this->renderPoints();
                this->renderLevel();
                refresh();
                break;
            }
        }
        
        this->renderFood();
        this->renderDifficulty();
        this->renderPoints();
        this->renderLevel();
        
        // 根据关卡类型调整游戏逻辑
        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        
        refresh();
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
            
            // 如果关卡已完成，添加完成标记
            if (mLevelStatus[i] == LevelStatus::Completed) {
                itemText += " (Completed)";
            }
            
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
    for (int i = 0; i < menuItems.size(); i++) {
        if (i == 0) {
            wattron(menu, A_STANDOUT);
        }
        
        // 如果是已锁定的关卡，显示为灰色
        if (i < mMaxLevel && mLevelStatus[i] == LevelStatus::Locked) {
            wattron(menu, A_DIM);
        }
        
        mvwprintw(menu, i + offset, 1, menuItems[i].c_str());
        
        if (i < mMaxLevel && mLevelStatus[i] == LevelStatus::Locked) {
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
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattroff(menu, A_DIM);
                }
                
                // 选择前一个选项
                index--;
                index = (index < 0) ? menuItems.size() - 1 : index;
                
                // 高亮新选项
                wattron(menu, A_STANDOUT);
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
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
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattroff(menu, A_DIM);
                }
                
                // 选择下一个选项
                index++;
                index = (index > menuItems.size() - 1) ? 0 : index;
                
                // 高亮新选项
                wattron(menu, A_STANDOUT);
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
                    wattron(menu, A_DIM);
                }
                mvwprintw(menu, index + offset, 1, menuItems[index].c_str());
                if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
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
    if (index == menuItems.size() - 2) {
        mReturnToModeSelect = true;
        return false;
    }
    // 如果选择了退出游戏
    else if (index == menuItems.size() - 1) {
        return false;
    }
    // 如果选择了一个锁定的关卡
    else if (index < mMaxLevel && mLevelStatus[index] == LevelStatus::Locked) {
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
    mvwprintw(this->mWindows[2], 18, 1, "Time Left:");
    std::string timeString = std::to_string(mTimeRemaining) + " s";
    // 清除旧的计时显示
    mvwprintw(this->mWindows[2], 19, 1, "          "); 
    mvwprintw(this->mWindows[2], 19, 1, timeString.c_str());
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
        }

        this->renderFood();
        this->renderDifficulty();
        this->renderPoints();
        this->renderTimer(); // 在每一帧都渲染计时器

        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        refresh();
    }
}



