#include "gui/story_level_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <algorithm>

StoryLevelWindow::StoryLevelWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , backgroundLabel(nullptr)
    , level1Button(nullptr)
    , level2Button(nullptr)
    , level3Button(nullptr)
    , level4Button(nullptr)
    , level5Button(nullptr)
    , backButton(nullptr)
{
    setupUI();
    setupBackgroundImage();
    setupLevelButtons();
    
    // 默认只解锁第一关
    mUnlockedLevels = {1};
    updateLevelStatus(mUnlockedLevels);
}

StoryLevelWindow::~StoryLevelWindow()
{
    // Qt的父子关系会自动清理内存
}

void StoryLevelWindow::setupUI()
{
    // 设置窗口属性
    setWindowTitle("Snake Game - Story Mode");
    setFixedSize(800, 600);
    
    // 居中显示窗口
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // 创建中心窗口部件
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建背景标签
    backgroundLabel = new QLabel(centralWidget);
    backgroundLabel->setGeometry(0, 0, 800, 600);
    backgroundLabel->setScaledContents(true);
}

void StoryLevelWindow::setupBackgroundImage()
{
    // 加载背景图片
    QPixmap background("assets/images/worldmap.png");
    if (background.isNull()) {
        // 如果图片加载失败，设置默认背景色
        backgroundLabel->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1ABC9C, stop:1 #16A085);");
    } else {
        backgroundLabel->setPixmap(background);
    }
}

void StoryLevelWindow::setupLevelButtons()
{
    // 创建关卡按钮
    level1Button = new QPushButton("1", centralWidget);
    level2Button = new QPushButton("2", centralWidget);
    level3Button = new QPushButton("3", centralWidget);
    level4Button = new QPushButton("4", centralWidget);
    level5Button = new QPushButton("5", centralWidget);
    backButton = new QPushButton("Back", centralWidget);
    
    // 设置按钮样式基础模板
    QString unlockedStyle = 
        "QPushButton {"
        "    background-color: rgba(46, 204, 113, 200);"
        "    border: 3px solid white;"
        "    border-radius: 25px;"
        "    color: white;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    width: 50px;"
        "    height: 50px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(76, 234, 143, 220);"
        "    border: 4px solid yellow;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(26, 174, 83, 200);"
        "}";
    
    QString lockedStyle = 
        "QPushButton {"
        "    background-color: rgba(149, 165, 166, 200);"
        "    border: 3px solid gray;"
        "    border-radius: 25px;"
        "    color: gray;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    width: 50px;"
        "    height: 50px;"
        "}";
    
    QString backButtonStyle = 
        "QPushButton {"
        "    background-color: rgba(231, 76, 60, 200);"
        "    border: 2px solid white;"
        "    border-radius: 15px;"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 8px 16px;"
        "    min-width: 80px;"
        "    min-height: 35px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(241, 106, 90, 220);"
        "    border: 3px solid yellow;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(201, 46, 30, 200);"
        "}";
    
    // 设置关卡按钮位置（根据worldmap.png的布局分布在地图上）
    level1Button->setGeometry(150, 450, 50, 50);    // 左下角开始
    level2Button->setGeometry(300, 350, 50, 50);    // 向右上移动
    level3Button->setGeometry(450, 250, 50, 50);    // 继续向右上
    level4Button->setGeometry(600, 200, 50, 50);    // 右上区域
    level5Button->setGeometry(650, 100, 50, 50);    // 最右上角
    
    // 返回按钮位置
    backButton->setGeometry(50, 50, 100, 40);
    backButton->setStyleSheet(backButtonStyle);
    
    // 连接信号和槽
    connect(level1Button, &QPushButton::clicked, this, &StoryLevelWindow::onLevel1Clicked);
    connect(level2Button, &QPushButton::clicked, this, &StoryLevelWindow::onLevel2Clicked);
    connect(level3Button, &QPushButton::clicked, this, &StoryLevelWindow::onLevel3Clicked);
    connect(level4Button, &QPushButton::clicked, this, &StoryLevelWindow::onLevel4Clicked);
    connect(level5Button, &QPushButton::clicked, this, &StoryLevelWindow::onLevel5Clicked);
    connect(backButton, &QPushButton::clicked, this, &StoryLevelWindow::onBackClicked);
}

void StoryLevelWindow::updateLevelStatus(const std::vector<int>& unlockedLevels)
{
    mUnlockedLevels = unlockedLevels;
    
    // 更新每个按钮的状态
    updateButtonState(level1Button, std::find(unlockedLevels.begin(), unlockedLevels.end(), 1) != unlockedLevels.end(), false);
    updateButtonState(level2Button, std::find(unlockedLevels.begin(), unlockedLevels.end(), 2) != unlockedLevels.end(), false);
    updateButtonState(level3Button, std::find(unlockedLevels.begin(), unlockedLevels.end(), 3) != unlockedLevels.end(), false);
    updateButtonState(level4Button, std::find(unlockedLevels.begin(), unlockedLevels.end(), 4) != unlockedLevels.end(), false);
    updateButtonState(level5Button, std::find(unlockedLevels.begin(), unlockedLevels.end(), 5) != unlockedLevels.end(), false);
}

void StoryLevelWindow::updateButtonState(QPushButton* button, bool unlocked, bool completed)
{
    QString unlockedStyle = 
        "QPushButton {"
        "    background-color: rgba(46, 204, 113, 200);"
        "    border: 3px solid white;"
        "    border-radius: 25px;"
        "    color: white;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    width: 50px;"
        "    height: 50px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(76, 234, 143, 220);"
        "    border: 4px solid yellow;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(26, 174, 83, 200);"
        "}";
    
    QString lockedStyle = 
        "QPushButton {"
        "    background-color: rgba(149, 165, 166, 200);"
        "    border: 3px solid gray;"
        "    border-radius: 25px;"
        "    color: gray;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    width: 50px;"
        "    height: 50px;"
        "}";
    
    if (unlocked) {
        button->setStyleSheet(unlockedStyle);
        button->setEnabled(true);
    } else {
        button->setStyleSheet(lockedStyle);
        button->setEnabled(false);
    }
}

void StoryLevelWindow::onLevel1Clicked()
{
    emit levelSelected(1);
}

void StoryLevelWindow::onLevel2Clicked()
{
    emit levelSelected(2);
}

void StoryLevelWindow::onLevel3Clicked()
{
    emit levelSelected(3);
}

void StoryLevelWindow::onLevel4Clicked()
{
    emit levelSelected(4);
}

void StoryLevelWindow::onLevel5Clicked()
{
    emit levelSelected(5);
}

void StoryLevelWindow::onBackClicked()
{
    emit backToModeSelect();
} 