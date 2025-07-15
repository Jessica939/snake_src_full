#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <QObject>
#include <QApplication>
#include <memory>

class ModeSelectWindow;
class StoryLevelWindow;
class StoryDisplayWindow;

class GUIManager : public QObject
{
    Q_OBJECT

public:
    explicit GUIManager(QObject *parent = nullptr);
    ~GUIManager();
    
    void start();                       // 启动GUI
    bool isClassicModeSelected() const; // 是否选择了经典模式
    bool isExitRequested() const;       // 是否请求退出
    int getSelectedLevel() const;       // 获取选择的关卡
    
    // 新增：同步关卡进度
    void syncLevelProgress();           // 同步关卡进度到GUI
    
    // 新增：关卡胜利后显示漫画
    void showCartoonAfterLevelVictory(int level); // 关卡胜利后显示漫画


private slots:
    void onStoryModeSelected();         // 剧情模式被选择
    void onClassicModeSelected();       // 经典模式被选择
    void onExitRequested();             // 退出请求
    void onLevelSelected(int level);    // 关卡被选择
    void onBackToModeSelect();          // 返回模式选择
    void onStoryFinished();             // 剧情播放完成
    void onSkipToGame();                // 跳过剧情直接开始游戏
    void onLevelStoryFinished();        // 关卡剧情播放完成
    void onSkipLevelStoryToGame();      // 跳过关卡剧情直接开始游戏
    void onVictoryCartoonFinished();    // 胜利漫画播放完成
    void onPrologueCartoonFinished();   // 序章漫画播放完成
    void onLevel5CartoonFinished();     // Level5漫画播放完成

private:
    void showModeSelectWindow();        // 显示模式选择窗口
    void showStoryLevelWindow();        // 显示剧情关卡选择窗口
    void showStoryDisplayWindow();      // 显示剧情播放窗口
    void loadLevelProgress();           // 加载关卡进度
    
    std::unique_ptr<ModeSelectWindow> mModeSelectWindow;
    std::unique_ptr<StoryLevelWindow> mStoryLevelWindow;
    std::unique_ptr<StoryDisplayWindow> mStoryDisplayWindow;
    
    bool mClassicModeSelected;          // 是否选择经典模式
    bool mExitRequested;                // 是否请求退出
    int mSelectedLevel;                 // 选择的关卡
    std::vector<int> mUnlockedLevels;   // 已解锁的关卡
};

#endif // GUI_MANAGER_H 