#ifndef MODE_SELECT_WINDOW_H
#define MODE_SELECT_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QWidget>

class ModeSelectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ModeSelectWindow(QWidget *parent = nullptr);
    ~ModeSelectWindow();

signals:
    void storyModeSelected();        // 选择剧情模式信号
    void classicModeSelected();      // 选择经典模式信号
    void exitGameRequested();       // 退出游戏信号

private slots:
    void onDirectionClicked();       // Direction按钮点击槽
    void onClassicClicked();         // Classic按钮点击槽
    void onExitClicked();            // Exit按钮点击槽

private:
    void setupUI();                  // 设置界面
    void setupBackgroundImage();     // 设置背景图片
    
    QWidget *centralWidget;
    QLabel *backgroundLabel;
    QPushButton *directionButton;    // Direction按钮（进入剧情模式）
    QPushButton *classicButton;      // Classic按钮（经典模式）
    QPushButton *exitButton;         // Exit按钮
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
};

#endif // MODE_SELECT_WINDOW_H 