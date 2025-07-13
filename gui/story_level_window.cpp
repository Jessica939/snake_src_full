#include "gui/story_level_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QShowEvent>
#include <QHideEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <algorithm>
#include <QCoreApplication> // Added for QCoreApplication::applicationDirPath()

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
    , hoverSound(nullptr)
    , clickSound(nullptr)
    , backgroundMusic(nullptr)
{
    setupUI();
    setupBackgroundImage();
    setupLevelButtons();
    setupAudioEffects();
    setupBackgroundMusic();
    
    // 默认只解锁第一关
    mUnlockedLevels = {1};
    updateLevelStatus(mUnlockedLevels);
}

StoryLevelWindow::~StoryLevelWindow()
{
    stopBackgroundMusic();
    // Qt的父子关系会自动清理内存
}

void StoryLevelWindow::setupUI()
{
    // 设置窗口属性
    setWindowTitle("Snake Game - Story Mode");
    setFixedSize(1536, 1000);
    
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
    backgroundLabel->setGeometry(0, 0, 1536, 1000);
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
    
    // 设置按钮样式基础模板（透明样式）
    QString transparentStyle = 
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    color: transparent;"
        "    font-size: 1px;"
        "}"
        "QPushButton:hover {"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QPushButton:pressed {"
        "    background-color: transparent;"
        "    border: none;"
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
    
    // 设置关卡按钮为透明样式
    level1Button->setStyleSheet(transparentStyle);
    level2Button->setStyleSheet(transparentStyle);
    level3Button->setStyleSheet(transparentStyle);
    level4Button->setStyleSheet(transparentStyle);
    level5Button->setStyleSheet(transparentStyle);
    
    // 设置关卡按钮位置（根据worldmap.png的布局分布在地图上）
    level1Button->setGeometry(45, 355, 120, 96);    // 左下角开始
    level2Button->setGeometry(365, 500, 120, 96);    // 向右上移动
    level3Button->setGeometry(693, 736, 120, 96);    // 继续向右上
    level4Button->setGeometry(1211, 751, 120, 96);    // 右上区域
    level5Button->setGeometry(1220, 190, 110, 105);    // 最右上角
    
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
    
    // 为按钮安装事件过滤器以捕获鼠标事件
    level1Button->installEventFilter(this);
    level2Button->installEventFilter(this);
    level3Button->installEventFilter(this);
    level4Button->installEventFilter(this);
    level5Button->installEventFilter(this);
    backButton->installEventFilter(this);
}

void StoryLevelWindow::setupAudioEffects()
{
    // 获取程序所在目录的绝对路径
    QString appDir = QCoreApplication::applicationDirPath();
    
    // 初始化悬停音效
    hoverSound = new QSoundEffect(this);
    QString hoverSoundPath = appDir + "/assets/music/mode_select_hover.wav";
    hoverSound->setSource(QUrl::fromLocalFile(hoverSoundPath));
    hoverSound->setVolume(0.6);  // 设置音量为60%
    
    // 初始化点击音效
    clickSound = new QSoundEffect(this);
    QString clickSoundPath = appDir + "/assets/music/mode_select_click.wav";
    clickSound->setSource(QUrl::fromLocalFile(clickSoundPath));
    clickSound->setVolume(0.7);  // 设置音量为70%
    
    // 检查音效是否加载成功
    if (hoverSound->status() != QSoundEffect::Ready) {
        qDebug() << "剧情模式悬停音效加载失败:" << hoverSoundPath;
    }
    if (clickSound->status() != QSoundEffect::Ready) {
        qDebug() << "剧情模式点击音效加载失败:" << clickSoundPath;
    }
}

void StoryLevelWindow::setupBackgroundMusic()
{
    // 获取程序所在目录的绝对路径
    QString appDir = QCoreApplication::applicationDirPath();
    QString musicPath = appDir + "/assets/music/story_background.mp3";
    
    // 初始化背景音乐
    backgroundMusic = new QMediaPlayer(this);
    backgroundMusic->setMedia(QUrl::fromLocalFile(musicPath));
    backgroundMusic->setVolume(70);  // 设置音量为70%，比剧情窗口稍低
    
    // 设置循环播放
    connect(backgroundMusic, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            backgroundMusic->play();  // 播放结束时重新开始
        }
    });
    
    // 检查音乐是否加载成功
    if (backgroundMusic->error() != QMediaPlayer::NoError) {
        qDebug() << "关卡选择背景音乐加载失败:" << backgroundMusic->errorString() << "路径:" << musicPath;
    } else {
        qDebug() << "关卡选择背景音乐加载成功:" << musicPath;
    }
}

void StoryLevelWindow::startBackgroundMusic()
{
    if (backgroundMusic && backgroundMusic->error() == QMediaPlayer::NoError) {
        // 重新连接循环播放的信号
        connect(backgroundMusic, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
            if (state == QMediaPlayer::StoppedState) {
                backgroundMusic->play();  // 播放结束时重新开始
            }
        });
        backgroundMusic->play();
        qDebug() << "开始播放关卡选择背景音乐";
    }
}

void StoryLevelWindow::stopBackgroundMusic()
{
    if (backgroundMusic && backgroundMusic->state() == QMediaPlayer::PlayingState) {
        // 先断开循环播放的连接，防止stop()后立即重新播放
        backgroundMusic->disconnect();
        backgroundMusic->stop();
        qDebug() << "停止播放关卡选择背景音乐";
    }
}

void StoryLevelWindow::playHoverSound()
{
    if (hoverSound && hoverSound->status() == QSoundEffect::Ready) {
        hoverSound->play();
    }
}

void StoryLevelWindow::playClickSound()
{
    if (clickSound && clickSound->status() == QSoundEffect::Ready) {
        clickSound->play();
    }
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
    QString transparentStyle = 
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    color: transparent;"
        "    font-size: 1px;"
        "}"
        "QPushButton:hover {"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QPushButton:pressed {"
        "    background-color: transparent;"
        "    border: none;"
        "}";
    
    if (unlocked) {
        button->setStyleSheet(transparentStyle);
        button->setEnabled(true);
    } else {
        button->setStyleSheet(transparentStyle);
        button->setEnabled(false);
    }
}

void StoryLevelWindow::onLevel1Clicked()
{
    playClickSound();
    emit levelSelected(1);
}

void StoryLevelWindow::onLevel2Clicked()
{
    playClickSound();
    emit levelSelected(2);
}

void StoryLevelWindow::onLevel3Clicked()
{
    playClickSound();
    emit levelSelected(3);
}

void StoryLevelWindow::onLevel4Clicked()
{
    playClickSound();
    emit levelSelected(4);
}

void StoryLevelWindow::onLevel5Clicked()
{
    playClickSound();
    emit levelSelected(5);
}

void StoryLevelWindow::onBackClicked()
{
    playClickSound();
    emit backToModeSelect();
}

bool StoryLevelWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 检查是否是按钮对象
    QPushButton *button = qobject_cast<QPushButton*>(obj);
    if (button && (button == level1Button || button == level2Button || button == level3Button || 
                   button == level4Button || button == level5Button || button == backButton)) {
        
        if (event->type() == QEvent::Enter) {
            // 鼠标进入按钮区域
            playHoverSound();
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            // 鼠标按下（点击）
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                playClickSound();
            }
        }
    }
    
    // 调用父类的事件过滤器
    return QMainWindow::eventFilter(obj, event);
}

void StoryLevelWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    startBackgroundMusic();
}

void StoryLevelWindow::hideEvent(QHideEvent *event)
{
    QMainWindow::hideEvent(event);
    stopBackgroundMusic();
} 