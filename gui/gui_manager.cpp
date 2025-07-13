#include "gui/gui_manager.h"
#include "gui/mode_select_window.h"
#include "gui/story_level_window.h"
#include <fstream>
#include <QEventLoop>

GUIManager::GUIManager(QObject *parent)
    : QObject(parent)
    , mModeSelectWindow(nullptr)
    , mStoryLevelWindow(nullptr)
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
    // 隐藏模式选择窗口
    if (mModeSelectWindow) {
        mModeSelectWindow->hide();
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
    showStoryLevelWindow();
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
    
    // 隐藏所有窗口
    if (mModeSelectWindow) {
        mModeSelectWindow->hide();
    }
    if (mStoryLevelWindow) {
        mStoryLevelWindow->hide();
    }
    
    // 退出Qt事件循环，让main函数继续执行指定关卡的游戏
    QApplication::quit();
}

void GUIManager::onBackToModeSelect()
{
    showModeSelectWindow();
} 