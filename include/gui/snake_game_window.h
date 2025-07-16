#ifndef SNAKE_GAME_WINDOW_H
#define SNAKE_GAME_WINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>
#include <QMap>
#include "game.h"

class SnakeGameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SnakeGameWindow(QWidget *parent = nullptr);
    ~SnakeGameWindow();
    
    void startGame(GameMode mode, int level = 0);
    bool isGameRunning() const { return m_bGameRunning; }
    bool isLevelCompleted() const;
    void showVictoryScreen(int level);

signals:
    void gameExited();
    void levelCompleted(int level);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateGame();

private:
    // 游戏逻辑
    Game m_aGame;
    QTimer* m_aGameTimer;
    bool m_bGameRunning;
    GameMode m_eCurrentMode;
    int m_nCurrentLevel;
    int m_nGameBoardWidth;
    int m_nGameBoardHeight;
    
    // 界面元素
    QWidget* m_aCentralWidget;
    QGridLayout* m_aGameLayout;
    QWidget* m_aGameBoardWidget;
    QGridLayout* m_aGameBoard;
    QVector<QVector<QLabel*>> m_aCells;
    QLabel* m_aScoreLabel;
    QLabel* m_aLevelLabel;
    QLabel* m_aLivesLabel;
    QLabel* m_aStatusLabel;
    
    // 游戏元素颜色样式
    QString m_sSnakeStyle;
    QString m_sFoodStyle;
    QString m_sWallStyle;
    QString m_sPoisonStyle;
    QString m_sSpecialFoodStyle;
    QString m_sRandomItemStyle;
    QString m_sEmptyStyle;
    QMap<int, QString> m_aColorMap; // 用于存储不同颜色对应的样式表
    
    // 辅助函数
    void initializeUI();
    void renderGameBoard();
    void updateGameInfo();
    void processKey(int key);
    void setupColorStyles();
};

#endif // SNAKE_GAME_WINDOW_H 