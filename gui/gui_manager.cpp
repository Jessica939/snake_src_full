#include "gui/gui_manager.h"
#include "gui/mode_select_window.h"
#include "gui/story_level_window.h"
#include "gui/story_display_window.h"
#include <fstream>
#include <QEventLoop>

GUIManager::GUIManager(QObject *parent)
    : QObject(parent)
    , mModeSelectWindow(nullptr)
    , mStoryLevelWindow(nullptr)
    , mStoryDisplayWindow(nullptr)
    , mClassicModeSelected(false)
    , mExitRequested(false)
    , mSelectedLevel(0)
{
    loadLevelProgress();
}

GUIManager::~GUIManager()
{
    // unique_ptr会自动清理
}

void GUIManager::start()
{
    showModeSelectWindow();
}

bool GUIManager::isClassicModeSelected() const
{
    return mClassicModeSelected;
}

bool GUIManager::isExitRequested() const
{
    return mExitRequested;
}

int GUIManager::getSelectedLevel() const
{
    return mSelectedLevel;
}

void GUIManager::showModeSelectWindow()
{
    // 隐藏剧情模式窗口
    if (mStoryLevelWindow) {
        mStoryLevelWindow->hide();
    }
    
    // 创建或显示模式选择窗口
    if (!mModeSelectWindow) {
        mModeSelectWindow = std::make_unique<ModeSelectWindow>();
        
        // 连接信号
        connect(mModeSelectWindow.get(), &ModeSelectWindow::storyModeSelected,
                this, &GUIManager::onStoryModeSelected);
        connect(mModeSelectWindow.get(), &ModeSelectWindow::classicModeSelected,
                this, &GUIManager::onClassicModeSelected);
        connect(mModeSelectWindow.get(), &ModeSelectWindow::exitGameRequested,
                this, &GUIManager::onExitRequested);
    }
    
    mModeSelectWindow->show();
    mModeSelectWindow->raise();
    mModeSelectWindow->activateWindow();
}

void GUIManager::showStoryLevelWindow()
{
    // 隐藏其他窗口
    if (mModeSelectWindow) {
        mModeSelectWindow->hide();
    }
    if (mStoryDisplayWindow) {
        mStoryDisplayWindow->hide();  // 确保隐藏剧情显示窗口，停止剧情音乐
    }
    
    // 创建或显示剧情关卡选择窗口
    if (!mStoryLevelWindow) {
        mStoryLevelWindow = std::make_unique<StoryLevelWindow>();
        
        // 连接信号
        connect(mStoryLevelWindow.get(), &StoryLevelWindow::levelSelected,
                this, &GUIManager::onLevelSelected);
        connect(mStoryLevelWindow.get(), &StoryLevelWindow::backToModeSelect,
                this, &GUIManager::onBackToModeSelect);
    }
    
    // 更新关卡状态
    mStoryLevelWindow->updateLevelStatus(mUnlockedLevels);
    
    mStoryLevelWindow->show();
    mStoryLevelWindow->raise();
    mStoryLevelWindow->activateWindow();
}

void GUIManager::loadLevelProgress()
{
    // 加载关卡进度，默认只解锁第一关
    mUnlockedLevels = {1};
    
    // 尝试从文件读取关卡进度
    std::ifstream file("level_progress.dat");
    if (file.is_open()) {
        int level;
        mUnlockedLevels.clear();
        while (file >> level) {
            if (level >= 1 && level <= 5) {
                mUnlockedLevels.push_back(level);
            }
        }
        file.close();
        
        // 如果文件为空或没有有效关卡，至少解锁第一关
        if (mUnlockedLevels.empty()) {
            mUnlockedLevels.push_back(1);
        }
    }
}

void GUIManager::onStoryModeSelected()
{
    showStoryDisplayWindow();
}

void GUIManager::onClassicModeSelected()
{
    mClassicModeSelected = true;
    
    // 隐藏所有窗口
    if (mModeSelectWindow) {
        mModeSelectWindow->hide();
    }
    if (mStoryLevelWindow) {
        mStoryLevelWindow->hide();
    }
    
    // 退出Qt事件循环，让main函数继续执行ncurses游戏
    QApplication::quit();
}

void GUIManager::onExitRequested()
{
    mExitRequested = true;
    
    // 隐藏所有窗口
    if (mModeSelectWindow) {
        mModeSelectWindow->hide();
    }
    if (mStoryLevelWindow) {
        mStoryLevelWindow->hide();
    }
    
    // 退出Qt事件循环
    QApplication::quit();
}

void GUIManager::onLevelSelected(int level)
{
    mSelectedLevel = level;
    
    // 隐藏关卡选择窗口
    if (mStoryLevelWindow) {
        mStoryLevelWindow->hide();
    }
    
    // 创建并显示剧情窗口
    if (!mStoryDisplayWindow) {
        mStoryDisplayWindow = std::make_unique<StoryDisplayWindow>();
        
        // 连接信号
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished,
                this, &GUIManager::onLevelStoryFinished);
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame,
                this, &GUIManager::onSkipLevelStoryToGame);
    } else {
        // 重新连接信号，确保指向正确的处理函数
        disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished,
                   this, &GUIManager::onStoryFinished);
        disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame,
                   this, &GUIManager::onSkipToGame);
        
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished,
                this, &GUIManager::onLevelStoryFinished);
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame,
                this, &GUIManager::onSkipLevelStoryToGame);
    }
    
    // 显示关卡剧情
    mStoryDisplayWindow->loadStoryForLevel(level);
    mStoryDisplayWindow->show();
}

void GUIManager::onBackToModeSelect()
{
    showModeSelectWindow();
}

void GUIManager::onStoryFinished()
{
    // 剧情播放完成后，显示关卡选择窗口
    showStoryLevelWindow();
}

void GUIManager::onSkipToGame()
{
    // 跳过剧情，直接显示关卡选择窗口
    showStoryLevelWindow();
}

void GUIManager::onLevelStoryFinished()
{
    // 关卡剧情播放完成后，隐藏剧情窗口并退出Qt事件循环开始游戏
    if (mStoryDisplayWindow) {
        mStoryDisplayWindow->hide();
    }
    
    // 退出Qt事件循环，让main函数继续执行指定关卡的游戏
    QApplication::quit();
}

void GUIManager::onSkipLevelStoryToGame()
{
    // 跳过关卡剧情，直接开始游戏
    if (mStoryDisplayWindow) {
        mStoryDisplayWindow->hide();
    }
    
    // 退出Qt事件循环，让main函数继续执行指定关卡的游戏
    QApplication::quit();
}

void GUIManager::showStoryDisplayWindow()
{
    // 隐藏其他窗口
    if (mModeSelectWindow) {
        mModeSelectWindow->hide();
    }
    if (mStoryLevelWindow) {
        mStoryLevelWindow->hide();
    }
    
    // 创建并显示剧情窗口
    if (!mStoryDisplayWindow) {
        mStoryDisplayWindow = std::make_unique<StoryDisplayWindow>();
    }
    
    // 确保信号连接到序章剧情处理函数
    disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished, nullptr, nullptr);
    disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame, nullptr, nullptr);
    
    connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished,
            this, &GUIManager::onStoryFinished);
    connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame,
            this, &GUIManager::onSkipToGame);
    
    // 显示序章剧情
    mStoryDisplayWindow->showPrologue();
    mStoryDisplayWindow->show();
} 