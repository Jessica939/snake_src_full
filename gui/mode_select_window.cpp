#include "gui/mode_select_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QSizePolicy>

ModeSelectWindow::ModeSelectWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , backgroundLabel(nullptr)
    , directionButton(nullptr)
    , classicButton(nullptr)
    , exitButton(nullptr)
    , mainLayout(nullptr)
    , buttonLayout(nullptr)
{
    setupUI();
    setupBackgroundImage();
}

ModeSelectWindow::~ModeSelectWindow()
{
    // Qt的父子关系会自动清理内存
}

void ModeSelectWindow::setupUI()
{
    // 设置窗口属性
    setWindowTitle("Snake Game - Mode Select");
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
    
    // 创建按钮
    directionButton = new QPushButton("Direction", centralWidget);
    classicButton = new QPushButton("Classic", centralWidget);
    exitButton = new QPushButton("Exit", centralWidget);
    
    // 设置按钮样式
    QString buttonStyle = 
        "QPushButton {"
        "    background-color: rgba(70, 130, 180, 200);"
        "    border: 2px solid white;"
        "    border-radius: 15px;"
        "    color: white;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    min-width: 120px;"
        "    min-height: 40px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(100, 150, 200, 220);"
        "    border: 3px solid yellow;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(50, 100, 150, 200);"
        "}";
    
    directionButton->setStyleSheet(buttonStyle);
    classicButton->setStyleSheet(buttonStyle);
    exitButton->setStyleSheet(buttonStyle);
    
    // 设置按钮位置（根据modeselect.png的布局调整）
    directionButton->setGeometry(350, 300, 140, 50);
    classicButton->setGeometry(350, 370, 140, 50);
    exitButton->setGeometry(350, 440, 140, 50);
    
    // 连接信号和槽
    connect(directionButton, &QPushButton::clicked, this, &ModeSelectWindow::onDirectionClicked);
    connect(classicButton, &QPushButton::clicked, this, &ModeSelectWindow::onClassicClicked);
    connect(exitButton, &QPushButton::clicked, this, &ModeSelectWindow::onExitClicked);
}

void ModeSelectWindow::setupBackgroundImage()
{
    // 加载背景图片
    QPixmap background("assets/images/modeselect.png");
    if (background.isNull()) {
        // 如果图片加载失败，设置默认背景色
        backgroundLabel->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2C3E50, stop:1 #34495E);");
    } else {
        backgroundLabel->setPixmap(background);
    }
}

void ModeSelectWindow::onDirectionClicked()
{
    emit storyModeSelected();
}

void ModeSelectWindow::onClassicClicked()
{
    emit classicModeSelected();
}

void ModeSelectWindow::onExitClicked()
{
    emit exitGameRequested();
} 