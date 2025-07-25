#ifndef STORY_LEVEL_WINDOW_H
#define STORY_LEVEL_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QWidget>
#include <QSoundEffect>
#include <QUrl>
#include <QMediaPlayer>
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

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // 事件过滤器
    void showEvent(QShowEvent *event) override;              // 窗口显示事件
    void hideEvent(QHideEvent *event) override;              // 窗口隐藏事件

private:
    void setupUI();                     // 设置界面
    void setupBackgroundImage();        // 设置背景图片
    void setupLevelButtons();           // 设置关卡按钮
    void setupAudioEffects();           // 设置音效
    void setupBackgroundMusic();        // 设置背景音乐
    void updateButtonState(QPushButton* button, bool unlocked, bool completed);  // 更新按钮状态
    void playHoverSound();              // 播放悬停音效
    void playClickSound();              // 播放点击音效
    void startBackgroundMusic();        // 开始播放背景音乐
    void stopBackgroundMusic();         // 停止播放背景音乐
    
    QWidget *centralWidget;
    QLabel *backgroundLabel;
    QPushButton *level1Button;          // 关卡1按钮
    QPushButton *level2Button;          // 关卡2按钮
    QPushButton *level3Button;          // 关卡3按钮
    QPushButton *level4Button;          // 关卡4按钮
    QPushButton *level5Button;          // 关卡5按钮
    QPushButton *backButton;            // 返回按钮
    
    std::vector<int> mUnlockedLevels;   // 已解锁的关卡列表
    
    // 音效相关
    QSoundEffect *hoverSound;           // 悬停音效
    QSoundEffect *clickSound;           // 点击音效
    
    // 背景音乐相关
    QMediaPlayer *backgroundMusic;      // 背景音乐播放器
};

#endif // STORY_LEVEL_WINDOW_H 