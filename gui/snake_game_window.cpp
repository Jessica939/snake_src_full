#include "gui/snake_game_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>
#include <QFont>

SnakeGameWindow::SnakeGameWindow(QWidget *parent)
    : QMainWindow(parent), m_bGameRunning(false)
{
    // 设置窗口属性
    setWindowTitle("贪吃蛇游戏");
    resize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    
    // 初始化定时器
    m_aGameTimer = new QTimer(this);
    connect(m_aGameTimer, &QTimer::timeout, this, &SnakeGameWindow::updateGame);
    
    // 设置默认游戏区域尺寸
    m_nGameBoardWidth = 30;
    m_nGameBoardHeight = 20;
    
    // 初始化UI
    initializeUI();
    
    // 初始化颜色样式
    setupColorStyles();
}

SnakeGameWindow::~SnakeGameWindow()
{
    if (m_aGameTimer->isActive()) {
        m_aGameTimer->stop();
    }
    
    // 清理cells
    for (int i = 0; i < m_aCells.size(); i++) {
        for (int j = 0; j < m_aCells[i].size(); j++) {
            delete m_aCells[i][j];
        }
    }
}

void SnakeGameWindow::initializeUI()
{
    // 创建中央部件
    m_aCentralWidget = new QWidget(this);
    setCentralWidget(m_aCentralWidget);
    
    // 创建主布局
    m_aGameLayout = new QGridLayout(m_aCentralWidget);
    
    // 创建游戏板Widget
    m_aGameBoardWidget = new QWidget(this);
    m_aGameBoard = new QGridLayout(m_aGameBoardWidget);
    m_aGameBoard->setSpacing(1);
    m_aGameBoard->setContentsMargins(0, 0, 0, 0);
    
    // 创建游戏信息区域
    QWidget* infoWidget = new QWidget(this);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);
    
    QFont labelFont("Arial", 12, QFont::Bold);
    
    m_aScoreLabel = new QLabel("得分: 0", this);
    m_aScoreLabel->setFont(labelFont);
    infoLayout->addWidget(m_aScoreLabel);
    
    m_aLevelLabel = new QLabel("关卡: 1", this);
    m_aLevelLabel->setFont(labelFont);
    infoLayout->addWidget(m_aLevelLabel);
    
    m_aLivesLabel = new QLabel("生命: 3", this);
    m_aLivesLabel->setFont(labelFont);
    infoLayout->addWidget(m_aLivesLabel);
    
    m_aStatusLabel = new QLabel("准备开始", this);
    m_aStatusLabel->setFont(labelFont);
    infoLayout->addWidget(m_aStatusLabel);
    
    // 添加弹簧使布局元素向上对齐
    infoLayout->addStretch();
    
    // 将组件添加到主布局
    m_aGameLayout->addWidget(m_aGameBoardWidget, 0, 0, 1, 1);
    m_aGameLayout->addWidget(infoWidget, 0, 1, 1, 1);
    
    // 设置列伸缩因子，使游戏区域占更多空间
    m_aGameLayout->setColumnStretch(0, 4);
    m_aGameLayout->setColumnStretch(1, 1);
    
    // 初始化游戏板网格
    for (int i = 0; i < m_nGameBoardHeight; i++) {
        QVector<QLabel*> row;
        for (int j = 0; j < m_nGameBoardWidth; j++) {
            QLabel* cell = new QLabel(this);
            cell->setFixedSize(20, 20);
            cell->setStyleSheet(m_sEmptyStyle);
            m_aGameBoard->addWidget(cell, i, j);
            row.append(cell);
        }
        m_aCells.append(row);
    }
}

void SnakeGameWindow::setupColorStyles()
{
    m_sEmptyStyle = "QLabel { background-color: white; border: 1px solid lightgray; }";
    m_sSnakeStyle = "QLabel { background-color: #00AAFF; border: 1px solid #0088CC; }";
    m_sFoodStyle = "QLabel { background-color: #FF5555; border: 1px solid #CC3333; }";
    m_sWallStyle = "QLabel { background-color: #888888; border: 1px solid #666666; }";
    m_sPoisonStyle = "QLabel { background-color: #AAFF00; border: 1px solid #88CC00; }";
    m_sSpecialFoodStyle = "QLabel { background-color: #FF00FF; border: 1px solid #CC00CC; }";
    m_sRandomItemStyle = "QLabel { background-color: #FFFF00; border: 1px solid #CCCC00; }";
    
    // 设置蛇的颜色映射
    m_aColorMap[1] = "QLabel { background-color: #00AAFF; border: 1px solid #0088CC; }"; // 默认青色
    m_aColorMap[2] = "QLabel { background-color: #FF5555; border: 1px solid #CC3333; }"; // 红色
    m_aColorMap[3] = "QLabel { background-color: #5555FF; border: 1px solid #3333CC; }"; // 蓝色
    m_aColorMap[4] = "QLabel { background-color: #55FF55; border: 1px solid #33CC33; }"; // 绿色
    m_aColorMap[5] = "QLabel { background-color: #FFFF55; border: 1px solid #CCCC33; }"; // 黄色
}

void SnakeGameWindow::startGame(GameMode mode, int level)
{
    m_eCurrentMode = mode;
    m_nCurrentLevel = level;
    m_bGameRunning = true;
    
    // 初始化游戏数据
    m_aGame = Game(); // 重置游戏
    
    // 根据模式和关卡设置游戏
    if (mode == GameMode::Classic) {
        // 使用现有方法初始化经典模式
        m_aGame.setGameMode(GameMode::Classic);
        m_aStatusLabel->setText("经典模式");
    } else if (mode == GameMode::Level) {
        // 初始化关卡模式
        m_aGame.setGameMode(GameMode::Level);
        m_aGame.initializeLevel(level);
        m_aStatusLabel->setText("关卡 " + QString::number(level+1));
    }
    
    // 设置定时器间隔为100毫秒（可以根据游戏速度调整）
    m_aGameTimer->start(100);
    
    // 渲染初始游戏板
    renderGameBoard();
    updateGameInfo();
}

void SnakeGameWindow::keyPressEvent(QKeyEvent *event)
{
    if (!m_bGameRunning) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    
    // 处理键盘输入
    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_W:
            processKey(KEY_UP);
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            processKey(KEY_DOWN);
            break;
        case Qt::Key_Left:
        case Qt::Key_A:
            processKey(KEY_LEFT);
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            processKey(KEY_RIGHT);
            break;
        case Qt::Key_Space:
            processKey(' '); // 空格键
            break;
        case Qt::Key_Escape:
            // 暂停游戏或退出
            if (m_aGameTimer->isActive()) {
                m_aGameTimer->stop();
                m_aStatusLabel->setText("已暂停");
            } else {
                m_aGameTimer->start(100);
                m_aStatusLabel->setText("进行中");
            }
            break;
        default:
            QMainWindow::keyPressEvent(event);
    }
}

void SnakeGameWindow::closeEvent(QCloseEvent *event)
{
    m_aGameTimer->stop();
    m_bGameRunning = false;
    emit gameExited();
    QMainWindow::closeEvent(event);
}

void SnakeGameWindow::updateGame()
{
    // TODO: 这里需要修改Game类，增加一个帧更新函数，不依赖于ncurses
    // 现在暂时使用占位实现
    
    // 更新游戏逻辑
    // m_aGame.update();
    
    // 检查游戏是否结束
    // if (m_aGame.isGameOver()) {
    //     m_aGameTimer->stop();
    //     m_bGameRunning = false;
    //     m_aStatusLabel->setText("游戏结束");
    //     QMessageBox::information(this, "游戏结束", "你的得分: " + QString::number(m_aGame.getScore()));
    //     return;
    // }
    
    // 检查关卡是否完成
    // if (m_eCurrentMode == GameMode::Level && m_aGame.isLevelCompleted()) {
    //     m_aGameTimer->stop();
    //     m_bGameRunning = false;
    //     emit levelCompleted(m_nCurrentLevel);
    //     return;
    // }
    
    // 更新UI
    renderGameBoard();
    updateGameInfo();
}

void SnakeGameWindow::renderGameBoard()
{
    // TODO: 这里需要从Game类获取当前状态并渲染到UI
    // 现在暂时使用占位实现
    
    // 例如:
    // - 墙使用m_sWallStyle
    // - 蛇使用m_sSnakeStyle
    // - 食物使用m_sFoodStyle
    // - 毒药使用m_sPoisonStyle
    // - 特殊食物使用m_sSpecialFoodStyle
    // - 随机道具使用m_sRandomItemStyle
    
    // 清空所有单元格
    for (int i = 0; i < m_aCells.size(); i++) {
        for (int j = 0; j < m_aCells[i].size(); j++) {
            m_aCells[i][j]->setStyleSheet(m_sEmptyStyle);
        }
    }
    
    // 这里需要Game类提供获取游戏元素位置的接口
    // 例如：
    // auto walls = m_aGame.getWalls();
    // for (const auto& wall : walls) {
    //     m_aCells[wall.y][wall.x]->setStyleSheet(m_sWallStyle);
    // }
    
    // auto snakeBody = m_aGame.getSnakeBody();
    // for (const auto& segment : snakeBody) {
    //     m_aCells[segment.y][segment.x]->setStyleSheet(m_sSnakeStyle);
    // }
    
    // auto food = m_aGame.getFood();
    // m_aCells[food.y][food.x]->setStyleSheet(m_sFoodStyle);
}

void SnakeGameWindow::updateGameInfo()
{
    // TODO: 更新游戏信息
    // 例如:
    // m_aScoreLabel->setText("得分: " + QString::number(m_aGame.getScore()));
    // m_aLivesLabel->setText("生命: " + QString::number(m_aGame.getLives()));
}

void SnakeGameWindow::processKey(int key)
{
    // TODO: 处理按键输入，更新游戏状态
    // m_aGame.handleKeyInput(key);
}

bool SnakeGameWindow::isLevelCompleted() const
{
    // TODO: 从Game类获取关卡是否完成
    // return m_aGame.isLevelCompleted();
    return false; // 临时返回值
}

void SnakeGameWindow::showVictoryScreen(int level)
{
    m_aStatusLabel->setText("关卡 " + QString::number(level+1) + " 胜利！");
    // 根据需要显示胜利画面
} 