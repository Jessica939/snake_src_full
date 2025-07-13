#ifndef MODE_SELECT_WINDOW_H
#define MODE_SELECT_WINDOW_H

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

class ModeSelectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ModeSelectWindow(QWidget *parent = nullptr);
    ~ModeSelectWindow();

signals:
    void storyModeSelected();        // 选择剧情模式信号
    void classicModeSelected();      // 选择经典模式信号
    void shopRequested();            // 商店信号
    void exitGameRequested();       // 退出游戏信号

private slots:
    void onDirectionClicked();       // Direction按钮点击槽
    void onClassicClicked();         // Classic按钮点击槽
    void onShopClicked();            // Shop按钮点击槽
    void onExitClicked();            // Exit按钮点击槽

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // 事件过滤器
    void showEvent(QShowEvent *event) override;              // 窗口显示事件
    void hideEvent(QHideEvent *event) override;              // 窗口隐藏事件

private:
    void setupUI();                  // 设置界面
    void setupBackgroundImage();     // 设置背景图片
    void setupAudioEffects();        // 设置音效
    void setupBackgroundMusic();     // 设置背景音乐
    void playHoverSound();           // 播放悬停音效
    void playClickSound();           // 播放点击音效
    void startBackgroundMusic();     // 开始播放背景音乐
    void stopBackgroundMusic();      // 停止播放背景音乐
    
    QWidget *centralWidget;
    QLabel *backgroundLabel;
    QPushButton *directionButton;    // Direction按钮（进入剧情模式）
    QPushButton *classicButton;      // Classic按钮（经典模式）
    QPushButton *shopButton;         // Shop按钮（商店）
    QPushButton *exitButton;         // Exit按钮
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    
    // 音效相关
    QSoundEffect *hoverSound;        // 悬停音效
    QSoundEffect *clickSound;        // 点击音效
    
    // 背景音乐相关
    QMediaPlayer *backgroundMusic;   // 背景音乐播放器
};

#endif // MODE_SELECT_WINDOW_H 