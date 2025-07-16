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
    , mShopRequested(false)
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

bool GUIManager::isShopRequested() const
{
    return mShopRequested;
}

int GUIManager::getSelectedLevel() const
{
    return mSelectedLevel;
}

void GUIManager::syncLevelProgress()
{
    // 重新加载关卡进度
    loadLevelProgress();
    
    // 如果关卡窗口已经存在，更新其状态
    if (mStoryLevelWindow) {
        mStoryLevelWindow->updateLevelStatus(mUnlockedLevels);
    }
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
        connect(mModeSelectWindow.get(), &ModeSelectWindow::shopRequested,
                this, &GUIManager::onShopRequested);
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
    
    // 尝试从二进制文件读取关卡进度（与Game类格式兼容）
    std::fstream file("level_progress.dat", std::ios::binary | std::ios::in);
    if (file.is_open()) {
        mUnlockedLevels.clear();
        
        // 读取关卡状态（二进制格式）
        int status;
        int level = 1;
        while (file.read(reinterpret_cast<char*>(&status), sizeof(status)) && level <= 5) {
            // 0=Locked, 1=Unlocked, 2=Completed
            if (status >= 1) { // Unlocked 或 Completed
                mUnlockedLevels.push_back(level);
            }
            level++;
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

void GUIManager::onShopRequested()
{
    mShopRequested = true;
    
    // 隐藏所有窗口
    if (mModeSelectWindow) {
        mModeSelectWindow->hide();
    }
    if (mStoryLevelWindow) {
        mStoryLevelWindow->hide();
    }
    
    // 退出Qt事件循环，让main函数继续执行ncurses商店界面
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
    // 序章剧情播放完成后，在同一窗口显示0_0.png漫画
    if (mStoryDisplayWindow) {
        // 重新连接信号，让漫画完成后跳转到关卡选择界面
        disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished, nullptr, nullptr);
        disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame, nullptr, nullptr);
        
        // 连接到序章漫画完成处理函数
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished,
                this, &GUIManager::onPrologueCartoonFinished);
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame,
                this, &GUIManager::onPrologueCartoonFinished);
        
        mStoryDisplayWindow->showCartoonForLevel(0, "prologue");
    } else {
        showStoryLevelWindow();
    }
}

void GUIManager::onSkipToGame()
{
    // 跳过剧情，直接显示关卡选择窗口
    showStoryLevelWindow();
}

void GUIManager::onLevelStoryFinished()
{
    // 根据关卡判断是否需要显示漫画
    if (mSelectedLevel == 5 && mStoryDisplayWindow) {
        // 重新连接信号，让level5漫画完成后能正确退出
        disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished, nullptr, nullptr);
        disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame, nullptr, nullptr);
        
        // 连接到level5漫画完成处理函数
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished,
                this, &GUIManager::onLevel5CartoonFinished);
        connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame,
                this, &GUIManager::onLevel5CartoonFinished);
        
        // level5关卡前的剧情放完后在同一窗口显示5_1'.png
        mStoryDisplayWindow->showCartoonForLevel(5, "pre_story");
    } else {
        // 其他关卡直接开始游戏
        if (mStoryDisplayWindow) {
            mStoryDisplayWindow->hide();
        }
        QApplication::quit();
    }
}

void GUIManager::onSkipLevelStoryToGame()
{
    // 跳过关卡剧情，直接开始游戏
    if (mStoryDisplayWindow) {
        mStoryDisplayWindow->hide();
    }
    QApplication::quit();
}

// 新增：关卡胜利后显示漫画
void GUIManager::showCartoonAfterLevelVictory(int level)
{
    // 创建并配置剧情窗口
    if (!mStoryDisplayWindow) {
        mStoryDisplayWindow = std::make_unique<StoryDisplayWindow>();
    }
    
    // 断开所有现有连接，重新连接到胜利漫画处理函数
    disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished, nullptr, nullptr);
    disconnect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame, nullptr, nullptr);
    
    // 连接到专门的胜利漫画完成处理函数
    connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::storyFinished,
            this, &GUIManager::onVictoryCartoonFinished);
    connect(mStoryDisplayWindow.get(), &StoryDisplayWindow::skipToGame,
            this, &GUIManager::onVictoryCartoonFinished);
    
    // 显示胜利漫画
    mStoryDisplayWindow->showCartoonForLevel(level, "victory");
    mStoryDisplayWindow->show();
}

void GUIManager::onVictoryCartoonFinished()
{
    // 胜利漫画播放完成，退出Qt应用返回游戏主循环
    if (mStoryDisplayWindow) {
        mStoryDisplayWindow->hide();
    }
    QApplication::quit();
}

void GUIManager::onPrologueCartoonFinished()
{
    // 序章漫画播放完成，显示关卡选择界面
    if (mStoryDisplayWindow) {
        mStoryDisplayWindow->hide();
    }
    showStoryLevelWindow();
}

void GUIManager::onLevel5CartoonFinished()
{
    // Level5漫画播放完成，退出Qt应用开始游戏
    if (mStoryDisplayWindow) {
        mStoryDisplayWindow->hide();
    }
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

 