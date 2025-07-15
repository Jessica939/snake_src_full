#ifndef CARTOON_DISPLAY_WINDOW_H
#define CARTOON_DISPLAY_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSoundEffect>
#include <QTimer>
#include <vector>
#include <string>

class CartoonDisplayWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CartoonDisplayWindow(QWidget *parent = nullptr);
    ~CartoonDisplayWindow();
    
    void showCartoon(const QString& cartoonPath);           // 显示单张漫画
    void showCartoons(const QStringList& cartoonPaths);     // 显示多张漫画
    void showCartoonsForLevel(int level, const QString& trigger = ""); // 根据关卡和触发条件显示漫画

signals:
    void cartoonFinished();               // 漫画播放完成信号
    void skipCartoon();                   // 跳过漫画信号

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onNextCartoon();                 // 显示下一张漫画
    void onPreviousCartoon();             // 显示上一张漫画
    void onSkipCartoons();                // 跳过所有漫画

private:
    void setupUI();                       // 设置界面
    void setupAudioEffects();             // 设置音效
    void loadCartoon(const QString& path); // 加载单张漫画
    void updateNavigationButtons();       // 更新导航按钮状态
    void showCurrentCartoon();            // 显示当前漫画
    QString getCartoonPath(const QString& filename); // 获取漫画完整路径
    
    // UI组件
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
    QLabel *m_cartoonLabel;
    QScrollArea *m_scrollArea;
    QPushButton *m_previousButton;
    QPushButton *m_nextButton;
    QPushButton *m_skipButton;
    QLabel *m_hintLabel;
    
    // 音效
    QSoundEffect *m_pageSound;
    QSoundEffect *m_clickSound;
    
    // 漫画数据
    QStringList m_cartoonPaths;           // 当前漫画列表
    int m_currentIndex;                   // 当前漫画索引
    QPixmap m_currentPixmap;              // 当前显示的漫画
    
    // 定时器
    QTimer *m_hintTimer;                  // 提示显示定时器
    
    // 常量
    static const int HINT_DELAY = 2000;   // 提示延迟（毫秒）
};

#endif // CARTOON_DISPLAY_WINDOW_H 