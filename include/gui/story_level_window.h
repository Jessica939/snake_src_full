#ifndef STORY_LEVEL_WINDOW_H
#define STORY_LEVEL_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QWidget>
#include <vector>

class StoryLevelWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StoryLevelWindow(QWidget *parent = nullptr);
    ~StoryLevelWindow();
    
    void updateLevelStatus(const std::vector<int>& unlockedLevels);  // 更新关卡解锁状态

signals:
    void levelSelected(int level);       // 选择关卡信号
    void backToModeSelect();            // 返回模式选择信号

private slots:
    void onLevel1Clicked();             // 关卡1点击槽
    void onLevel2Clicked();             // 关卡2点击槽
    void onLevel3Clicked();             // 关卡3点击槽
    void onLevel4Clicked();             // 关卡4点击槽
    void onLevel5Clicked();             // 关卡5点击槽
    void onBackClicked();               // 返回按钮点击槽

private:
    void setupUI();                     // 设置界面
    void setupBackgroundImage();        // 设置背景图片
    void setupLevelButtons();           // 设置关卡按钮
    void updateButtonState(QPushButton* button, bool unlocked, bool completed);  // 更新按钮状态
    
    QWidget *centralWidget;
    QLabel *backgroundLabel;
    QPushButton *level1Button;          // 关卡1按钮
    QPushButton *level2Button;          // 关卡2按钮
    QPushButton *level3Button;          // 关卡3按钮
    QPushButton *level4Button;          // 关卡4按钮
    QPushButton *level5Button;          // 关卡5按钮
    QPushButton *backButton;            // 返回按钮
    
    std::vector<int> mUnlockedLevels;   // 已解锁的关卡列表
};

#endif // STORY_LEVEL_WINDOW_H 