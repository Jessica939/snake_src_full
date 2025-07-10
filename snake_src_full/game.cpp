#include <string>
#include <iostream>
#include <cmath> 

// For terminal delay
#include <chrono>
#include <thread>

#include <fstream>
#include <algorithm> 

#include "game.h"



Game::Game()
{
    // Separate the screen to three windows
    this->mWindows.resize(3);
    initscr();
    // If there wasn't any key pressed don't wait for keypress
    nodelay(stdscr, true);
    // Turn on keypad control
    keypad(stdscr, true);
    // No echo for the key pressed
    noecho();
    // No cursor show
    curs_set(0);
    // Get screen and board parameters
    getmaxyx(stdscr, this->mScreenHeight, this->mScreenWidth);
    this->mGameBoardWidth = this->mScreenWidth - this->mInstructionWidth;
    this->mGameBoardHeight = this->mScreenHeight - this->mInformationHeight;

    this->createInformationBoard();
    this->createGameBoard();
    this->createInstructionBoard();

    // Initialize the leader board to be all zeros
    this->mLeaderBoard.assign(this->mNumLeaders, 0);
}

Game::~Game()
{
    for (int i = 0; i < this->mWindows.size(); i ++)
    {
        delwin(this->mWindows[i]);
    }
    endwin();
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

// game.cpp

void Game::createGameBoard()
{
    int startY = this->mInformationHeight;
    int startX = 0;
    if (mShrunkBoundary) {
        mEffectiveHeight = this->mGameBoardHeight - 4;
        mEffectiveWidth = this->mGameBoardWidth - 8;
    } else {
        mEffectiveHeight = this->mGameBoardHeight;
        mEffectiveWidth = this->mGameBoardWidth;
    }

    if (this->mWindows[1]) {
        delwin(this->mWindows[1]); 
    }

    this->mWindows[1] = newwin(mEffectiveHeight, mEffectiveWidth, startY, startX);

    if (this->mPtrSnake) {
        // 只有当蛇对象存在时，才更新它的边界
        this->mPtrSnake->updateBoundary(mEffectiveWidth, mEffectiveHeight);
    }

    box(this->mWindows[1], 0, 0);
    wrefresh(this->mWindows[1]);
}

void Game::renderGameBoard() const
{
    box(this->mWindows[1], 0, 0);
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

void Game::initializeGame()
{
    this->mPtrSnake.reset(new Snake(this->mGameBoardWidth, this->mGameBoardHeight, this->mInitialSnakeLength));
    
    this->mPtrSnake->updateBoundary(this->mEffectiveWidth, this->mEffectiveHeight);
    
    this->createRamdonFood();
    this->spawnObstacles(5);//障碍物的数量可以
    this->mPtrSnake->senseFood(this->mFood);
    this->mDifficulty = 0;
    this->mPoints = 0;
    this->mDelay = this->mBaseDelay;
}

void Game::createRamdonFood()
{
    std::vector<SnakeBody> availableGrids;
    for (int i = 1; i < this->mEffectiveHeight - 1; i ++)
    {
        for (int j = 1; j < this->mEffectiveWidth - 1; j ++)
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

    if (availableGrids.empty()) return;

    // Randomly select a grid that is not occupied by the snake
    //生成普通食物
    int random_idx = std::rand() % availableGrids.size();
    this->mFood = availableGrids[random_idx];

    
    int extraType = std::rand() % 2; // 0: special, 1: poison

    // 从剩余 availableGrids 中选出一个没有普通食物的位置
    SnakeBody extraPos;
    do {
        extraPos = availableGrids[std::rand() % availableGrids.size()];
    } while (extraPos == this->mFood);

    if (extraType == 0) { // 生成特殊食物
        this->mSpecialFood.active = true;
        this->mSpecialFood.lifetime = 50; // 设置持续时间（帧数）
        this->mSpecialFood.type = (std::rand() % 2 == 0) ? FoodType::Special2 : FoodType::Special4;
        this->mSpecialFood.pos = extraPos;

        // 关闭毒药
        this->mPoisonFood.active = false;
        this->mHasPoison = false;

    } else if (extraType == 1 && !mHasPoison) { // 生成毒药（前提是当前没有）
        this->mPoisonFood.active = true;
        this->mPoisonFood.type = FoodType::Poison;
        this->mPoisonFood.pos = extraPos;
        this->mHasPoison = true;

        // 关闭特殊食物
        this->mSpecialFood.active = false;
    } else {
        // 都不生成（防止毒药重复生成）
        this->mSpecialFood.active = false;
        this->mPoisonFood.active = false;
        this->mHasPoison = false;
    }

}

void Game::renderFood() const
{
    mvwaddch(this->mWindows[1], this->mFood.getY(), this->mFood.getX(), this->mFoodSymbol);
    if (mSpecialFood.active) {
        char specialChar = (mSpecialFood.type == FoodType::Special2) ? mSpecial2Symbol : mSpecial4Symbol;
        mvwaddch(this->mWindows[1], mSpecialFood.pos.getY(), mSpecialFood.pos.getX(), specialChar);
    }

    // 渲染毒药
    if (mPoisonFood.active) {
        mvwaddch(this->mWindows[1], mPoisonFood.pos.getY(), mPoisonFood.pos.getX(), mPoisonSymbol);
    }

    wrefresh(this->mWindows[1]);
}

void Game::renderSnake() const
{
    auto isInside = [this](int x, int y) {
        return x > 0 && x < mEffectiveWidth - 1 && y > 0 && y < mEffectiveHeight - 1;
    };

    int snakeLength = this->mPtrSnake->getLength();
    std::vector<SnakeBody>& snake = this->mPtrSnake->getSnake();
    for (int i = 0; i < snakeLength; i ++)
    {
        int y = snake[i].getY();
        int x = snake[i].getX();
        if (isInside(x, y)){
            mvwaddch(this->mWindows[1], y, x, this->mSnakeSymbol);
        }
        
    }
    wrefresh(this->mWindows[1]);
}

void Game::controlSnake()
{
    int key;
    key = getch();
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
        case 'Q':
        case 'q': 
        {
            auto it_grow = mInventory.find(ItemType::GROW);
            if (it_grow != mInventory.end() && it_grow->second > 0) {
                it_grow->second--; // 使用道具
                mPtrSnake->grow(3);
                break; 
            }

            auto it_shrink = mInventory.find(ItemType::SHRINK);
            if (it_shrink != mInventory.end() && it_shrink->second > 0) {
                it_shrink->second--;
                mPtrSnake->shrink();
                mPtrSnake->shrink();
                break;
            }

            auto it_slow = mInventory.find(ItemType::SLOW_DOWN);
            if (it_slow != mInventory.end() && it_slow->second > 0) {
                it_slow->second--;
                mDelay = mDelay * 1.5; // 减速
                break;
            }

            auto it_speed = mInventory.find(ItemType::SPEED_UP);
            if (it_speed != mInventory.end() && it_speed->second > 0) {
                it_speed->second--;
                mDelay = mDelay * 0.5; // 加速
                break;
            }
            break;
        }
        case 'e':
        case 'E': //传送门
        {
            auto it_portal = mInventory.find(ItemType::PORTAL);
            if (it_portal != mInventory.end() && it_portal->second > 0) {
                it_portal->second--; 
                
                // 传送门逻辑：把蛇头传到一个随机位置
                SnakeBody newPos;
                bool position_ok = false;
                while(!position_ok) {
                    int x = (rand() % (mEffectiveWidth - 2)) + 1;
                    int y = (rand() % (mEffectiveHeight - 2)) + 1;
                    if(!mPtrSnake->isPartOfSnake(x, y)) {
                        newPos = SnakeBody(x,y);
                        position_ok = true;
                    }
                }
            
                mPtrSnake->getSnake().insert(mPtrSnake->getSnake().begin(), newPos);
                mPtrSnake->getSnake().erase(mPtrSnake->getSnake().begin() + 1);
            }
            break;
        }
        case 'r': 
        case 'R':
            this->useRandomBox();
            break;

        case 'f':
        case 'F':
            this->mIsPhasing = true; // 按下F键，标记本帧要尝试穿墙
            break;
        default:
        {
            break;
        }
    }
}

void Game::renderBoards() const
{
    werase(this->mWindows[0]);
    werase(this->mWindows[1]);
    this->renderInformationBoard();
    this->renderGameBoard();
    box(this->mWindows[0], 0, 0);
    wrefresh(this->mWindows[0]);

    this->renderSidePanel(); 

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
    while (true)
    {
        this->controlSnake();

        werase(this->mWindows[1]);
        box(this->mWindows[1], 0, 0);

        // 随机触发“边界缩小”
        if (!mShrunkBoundary && (rand() % 100 < 2)) { 
            mShrunkBoundary = true;
            mShrinkDuration = 150;
            this->createGameBoard();  
        }

        // 更新边界缩小状态
        if (mShrunkBoundary) {
            mShrinkDuration--;
            if (mShrinkDuration <= 0) {
                mShrunkBoundary = false;
                this->createGameBoard();  
            }
        }

        // 尝试生成随机道具
        this->spawnRandomItem();

        bool eatFood = false;

        // 根据玩家输入（是否按了穿墙键）执行不同的移动逻辑
        if (mIsPhasing) {
            SnakeBody nextPos = this->mPtrSnake->createNewHead();
            // 检查下一个位置是否是障碍物
            if (checkObstacleCollision(nextPos)) {
                mPtrSnake->getSnake().insert(mPtrSnake->getSnake().begin(), nextPos); // 移动到障碍物位置
                mPtrSnake->getSnake().pop_back(); 
                mPtrSnake->shrink(); // 长度-1
            } else {
                eatFood = this->mPtrSnake->moveFoward();
            }
            mIsPhasing = false; //重置穿墙意图
        } else {
            eatFood = this->mPtrSnake->moveFoward();
        }

        if (this->mPtrSnake->getLength() <= 0){
            // 如果蛇的长度小于等于0，也算死亡
            this->mPtrSnake->loseLife();
            if (this->mPtrSnake->isDead()) {
                break; 
            } else {
                this->mPtrSnake->resetSnake(); // 重置蛇
                continue; 
            }
        }

        bool selfCollision = this->mPtrSnake->checkCollision(); 
        bool obstacleCollision = checkObstacleCollision(this->mPtrSnake->getHead());

        // 如果撞了墙/自己，或者在没有头盔的情况下撞了障碍物
        if (selfCollision || (obstacleCollision && !mInventory.count(ItemType::HELMET))) {
            this->mPtrSnake->loseLife();
            if (this->mPtrSnake->isDead()) { break; }
            else {
                this->mPtrSnake->resetSnake();
                continue;
            }
        }
        
        // 如果有头盔时撞了障碍物，则消耗一个头盔（不会死亡）
        if (obstacleCollision && mInventory.count(ItemType::HELMET) && mInventory[ItemType::HELMET] > 0) {
            mInventory[ItemType::HELMET]--;
        }


        this->renderSnake();

        if (eatFood) {
            this->mPoints += 1;
            this->mMoney += 2;
            this->createRamdonFood();
            this->mPtrSnake->senseFood(this->mFood);
            this->adjustDelay();
        }

        if (mSpecialFood.active && mPtrSnake->getHead() == mSpecialFood.pos) {
            if (mSpecialFood.type == FoodType::Special2) {
                mPtrSnake->grow(2); mPoints += 2; mMoney += 4;
            } else {
                mPtrSnake->grow(4); mPoints += 4; mMoney += 8;
            }
            mSpecialFood.active = false;
            mPoisonFood.active = false;
            mHasPoison = false;
        }

        if (mPoisonFood.active && mPtrSnake->getHead() == mPoisonFood.pos) {
            mPtrSnake->shrink();
            mPoisonFood.active = false;
            mHasPoison = false;
        }

        if (mActiveItem.active && mPtrSnake->getHead() == mActiveItem.pos) {
            mInventory[mActiveItem.type]++;
            mActiveItem.active = false;
        }

        if (mSpecialFood.active) {
            mSpecialFood.lifetime--;
            if (mSpecialFood.lifetime <= 0) { mSpecialFood.active = false; }
        }

        if (mActiveItem.active) {
            mActiveItem.lifetime--;
            if (mActiveItem.lifetime <= 0) { mActiveItem.active = false; }
        }
  
        this->renderObstacles(); 
        this->renderFood();
        this->renderSidePanel();

       
        std::this_thread::sleep_for(std::chrono::milliseconds(this->mDelay));
        refresh();
    }
}

void Game::startGame()
{
    refresh();
    bool choice;
    while (true)
    {
        this->readLeaderBoard();
        this->renderBoards();
        this->initializeGame();
        this->runGame();
        this->updateLeaderBoard();
        this->writeLeaderBoard();
        choice = this->renderRestartMenu();
        if (choice == false)
        {
            break;
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


void Game::updateExtraFood() {
    // 每帧调用：处理特殊食物时间与毒药状态
    if (mSpecialFood.active) {
        mSpecialFood.lifetime--;
        if (mSpecialFood.lifetime <= 0) {
            mSpecialFood.active = false;
        }
    }
    // 毒药不受时间限制，只在新一轮食物生成时被清除
}


void Game::showShop() {
    WINDOW* shopWindow;
    int height = 15;
    int width = 40;
    int startY = (mScreenHeight - height) / 2;
    int startX = (mScreenWidth - width) / 2;

    shopWindow = newwin(height, width, startY, startX);
    box(shopWindow, 0, 0);
    
    mvwprintw(shopWindow, 1, 1, "Welcome to the Item Shop!");
    mvwprintw(shopWindow, 2, 1, "Your Money: %d", mMoney);
    mvwprintw(shopWindow, 4, 1, "Select an item to buy (W/S):");
    mvwprintw(shopWindow, 13, 1, "Press SPACE to continue to game...");

    std::vector<std::pair<ItemType, std::string>> itemsForSale;
    itemsForSale.push_back({ItemType::HELMET,    "1. Helmet (Price: 15, Press '1')"});
    itemsForSale.push_back({ItemType::SLOW_DOWN, "2. Slow Down (Price: 10, Press '2')"});
    itemsForSale.push_back({ItemType::SPEED_UP,  "3. Spped Up (Price: 10, Press '3')"});
    itemsForSale.push_back({ItemType::PORTAL,    "4. Portal (Price: 25, Press '4')"});
    itemsForSale.push_back({ItemType::RANDOM_BOX,"5. Random Box (Price: 12, Press '5')"});
    itemsForSale.push_back({ItemType::GROW,      "6. Grow (Price: 15, Press '6')"});
    itemsForSale.push_back({ItemType::SHRINK,    "7. Shrink (Price: 10, Press '7')"});


    int yOffset = 6;
    for(size_t i = 0; i < itemsForSale.size(); ++i) {
        mvwprintw(shopWindow, yOffset + i, 2, itemsForSale[i].second.c_str());
    }

    wrefresh(shopWindow);

    int key;
    while((key = getch()) != ' ' && key != 10) { // 等待直到按下空格或回车
        int price = 0;
        ItemType selectedItem;
        bool purchased = false;

        switch(key) {
            case '1':
                price = 15;
                selectedItem = ItemType::HELMET;
                if (mMoney >= price) {
                    mMoney -= price;
                    mInventory[selectedItem]++;
                    purchased = true;
                }
                break;
            case '2':
                price = 10;
                selectedItem = ItemType::SLOW_DOWN;
                if (mMoney >= price) {
                    mMoney -= price;
                    mInventory[selectedItem]++;
                    purchased = true;
                }
                break;
            case '3':
                price = 10;
                selectedItem = ItemType::SPEED_UP;
                if (mMoney >= price) {
                    mMoney -= price;
                    mInventory[selectedItem]++;
                    purchased = true;
                }
                break;
            case '4':
                price = 25;
                selectedItem = ItemType::PORTAL;
                if (mMoney >= price) {
                    mMoney -= price;
                    mInventory[selectedItem]++;
                    purchased = true;
                }
                break;
            case '5':
                price = 12;
                selectedItem = ItemType::RANDOM_BOX;
                if (mMoney >= price) {
                    mMoney -= price;
                    mInventory[selectedItem]++;
                    purchased = true;
                }
                break;
            case '6':
                price = 15;
                selectedItem = ItemType::GROW;
                if (mMoney >= price) {
                    mMoney -= price;
                    mInventory[selectedItem]++;
                    purchased = true;
                }
                break;
            case '7':
                price = 10;
                selectedItem = ItemType::SHRINK;
                if (mMoney >= price) {
                    mMoney -= price;
                    mInventory[selectedItem]++;
                    purchased = true;
                }
                break;
            default:
                continue; // 如果按键不在选项内，跳过
        }

        if (purchased) {
             mvwprintw(shopWindow, 2, 1, "Your Money: %d   ", mMoney); // 刷新金钱显示
             wrefresh(shopWindow);
        }
    }
    delwin(shopWindow);
}


void Game::spawnRandomItem() {
    // 如果当前已有活动道具，则不生成新的
    if (mActiveItem.active) return;

    // 10% 的概率生成一个道具
    if (rand() % 100 < 10) {
        // 确保生成位置不是蛇的身体或食物
        SnakeBody itemPos;
        bool position_ok = false;
        while (!position_ok) {
            int x = (rand() % (mEffectiveWidth - 2)) + 1;
            int y = (rand() % (mEffectiveHeight - 2)) + 1;
            itemPos = SnakeBody(x, y);

            if (!mPtrSnake->isPartOfSnake(x, y) && !(mFood == itemPos)) {
                position_ok = true;
            }
        }
        
        mActiveItem.active = true;
        mActiveItem.pos = itemPos;
        mActiveItem.lifetime = 150; // 道具存在150帧

        // 随机选择一种道具
        int item_id = rand() % 4;
        switch(item_id) {
            case 0: mActiveItem.type = ItemType::GROW; mActiveItem.symbol = '+'; break;
            case 1: mActiveItem.type = ItemType::SHRINK; mActiveItem.symbol = '-'; break;
            case 2: mActiveItem.type = ItemType::SPEED_UP; mActiveItem.symbol = 'F'; break; 
            case 3: mActiveItem.type = ItemType::RANDOM_BOX; mActiveItem.symbol = '?'; break;
        }
    }
}


void Game::useRandomBox() {
    // 查找库存中是否有随机盒子
    auto it = mInventory.find(ItemType::RANDOM_BOX);
    if (it != mInventory.end() && it->second > 0) {
        it->second--; // 消耗一个随机盒子

        // 随机生成一个 0 到 4 之间的数字，对应 5 种可能的道具
        int randomEffect = std::rand() % 5; 
        
        switch (randomEffect) {
            case 0:
                mInventory[ItemType::HELMET]++;
                // 可以在这里加一个文字提示，告诉玩家获得了什么
                break;
            case 1:
                mInventory[ItemType::GROW]++;
                break;
            case 2:
                mInventory[ItemType::SHRINK]++;
                break;
            case 3:
                mInventory[ItemType::SPEED_UP]++;
                break;
            case 4:
                mInventory[ItemType::SLOW_DOWN]++;
                break;
        }
    }
}

void Game::renderFood() const
{
    // 渲染普通食物
    mvwaddch(this->mWindows[1], this->mFood.getY(), this->mFood.getX(), this->mFoodSymbol);

    // 渲染特殊食物
    if (mSpecialFood.active) {
        char specialChar = (mSpecialFood.type == FoodType::Special2) ? mSpecial2Symbol : mSpecial4Symbol;
        mvwaddch(this->mWindows[1], mSpecialFood.pos.getY(), mSpecialFood.pos.getX(), specialChar);
    }
    
    // 渲染毒药
    if (mPoisonFood.active) {
        mvwaddch(this->mWindows[1], mPoisonFood.pos.getY(), mPoisonFood.pos.getX(), mPoisonSymbol);
    }

    // 渲染地图上的随机道具
    if (mActiveItem.active) {
        mvwaddch(this->mWindows[1], mActiveItem.pos.getY(), mActiveItem.pos.getX(), mActiveItem.symbol);
    }

    // 刷新
    wrefresh(this->mWindows[1]);
}


void Game::renderLives() const{
    std::string hearts = "Lives: ";
    for (int i = 0; i < this->mPtrSnake->getLives(); ++i) {
        hearts += "\u2665 "; // 打印心型
    }
    mvwprintw(this->mWindows[2], 2, 1, "%s", hearts.c_str());
    wrefresh(this->mWindows[2]);
}

void Game::renderMoneyAndInventory(int& currentY) const
{
    // 绘制【玩家资产】部分
    currentY++; // 增加垂直间距
    mvwprintw(this->mWindows[2], currentY++, 1, "Money: %d", mMoney);
    
    // 检查并绘制库存
    bool hasItems = false;
    for (auto it = mInventory.begin(); it != mInventory.end(); ++it) {
        if (it->second > 0) {
            hasItems = true;
            break;
        }
    }

    if (hasItems) {
        mvwprintw(this->mWindows[2], currentY++, 1, "Inventory:");
        for (auto const& [itemType, count] : mInventory) {
            if (count > 0) {
                std::string itemName;
                switch(itemType) {
                    case ItemType::HELMET:    itemName = "Helmet"; break;
                    case ItemType::GROW:      itemName = "Grow"; break;
                    case ItemType::SHRINK:    itemName = "Shrink"; break;
                    case ItemType::SLOW_DOWN: itemName = "Slow Down"; break;
                    case ItemType::SPEED_UP:  itemName = "Speed Up"; break;
                    case ItemType::PORTAL:    itemName = "Portal"; break;
                    case ItemType::RANDOM_BOX:itemName = "Random Box"; break;
                    default:                  itemName = "Unknown Item"; break;
                }
                if (mScreenHeight - mInformationHeight > currentY) {
                    mvwprintw(this->mWindows[2], currentY++, 2, "- %s: %d", itemName.c_str(), count);
                }
            }
        }
    }
}

//此函数负责统一绘制所有信息，包括游戏说明、实时分数、生命值、玩家资产（金钱/库存）以及排行榜。
void Game::renderSidePanel() const {
    werase(this->mWindows[2]);
    box(this->mWindows[2], 0, 0);
    int currentY = 1;

    mvwprintw(this->mWindows[2], currentY++, 1, "Manual");
    mvwprintw(this->mWindows[2], currentY++, 1, "Up:    W");
    mvwprintw(this->mWindows[2], currentY++, 1, "Down:  S");
    mvwprintw(this->mWindows[2], currentY++, 1, "Left:  A");
    mvwprintw(this->mWindows[2], currentY++, 1, "Right: D");
    mvwprintw(this->mWindows[2], currentY++, 1, "Items: Q/E");

    currentY++; 
    mvwprintw(this->mWindows[2], currentY++, 1, "Difficulty: %d", this->mDifficulty);
    mvwprintw(this->mWindows[2], currentY++, 1, "Points: %d", this->mPoints);
    
    std::string hearts = "Lives: ";
    if (this->mPtrSnake) { 
        for (int i = 0; i < this->mPtrSnake->getLives(); ++i) {
            hearts += "\u2665 "; 
        }
    }
    mvwprintw(this->mWindows[2], currentY++, 1, "%s", hearts.c_str());

    // 绘制【玩家资产】部分
    this->renderMoneyAndInventory(currentY);

    // 绘制【排行榜】部分
    currentY++; 
    if (mScreenHeight - mInformationHeight > currentY + 3) {
        mvwprintw(this->mWindows[2], currentY++, 1, "Leader Board");
        std::string pointString;
        std::string rank;
        for (int i = 0; i < mNumLeaders; i++) {
            if (mScreenHeight - mInformationHeight <= currentY) break; 
            
            pointString = std::to_string(this->mLeaderBoard[i]);
            rank = "#" + std::to_string(i + 1) + ":";
            mvwprintw(this->mWindows[2], currentY++, 1, "%s %s", rank.c_str(), pointString.c_str());
        }
    }

    //显示所有内容
    wrefresh(this->mWindows[2]);
}

void Game::spawnObstacles(int count) {
    mObstacles.clear(); // 每轮游戏开始前清空旧的障碍物
    for (int i = 0; i < count; ++i) {
        SnakeBody obstaclePos;
        bool position_ok = false;
        while (!position_ok) {
            int x = (rand() % (mEffectiveWidth - 2)) + 1;
            int y = (rand() % (mEffectiveHeight - 2)) + 1;
            
            // 确保障碍物位置不会与蛇的初始位置重叠
            if (!mPtrSnake->isPartOfSnake(x, y)) {
                obstaclePos = SnakeBody(x, y);
                position_ok = true;
            }
        }
        mObstacles.push_back(obstaclePos);
    }
}

void Game::renderObstacles() const {
    for (const auto& obs : mObstacles) {
        mvwaddch(this->mWindows[1], obs.getY(), obs.getX(), mObstacleSymbol);
    }
}

bool Game::checkObstacleCollision(const SnakeBody& pos) const {
    for (const auto& obs : mObstacles) {
        if (obs == pos) {
            return true;
        }
    }
    return false;
}


