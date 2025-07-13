#include "gui/mode_select_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QSizePolicy>
#include <QEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QDebug>
#include <QCoreApplication> // Added for QCoreApplication::applicationDirPath()

ModeSelectWindow::ModeSelectWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , backgroundLabel(nullptr)
    , directionButton(nullptr)
    , classicButton(nullptr)
    , shopButton(nullptr)
    , exitButton(nullptr)
    , mainLayout(nullptr)
    , buttonLayout(nullptr)
    , hoverSound(nullptr)
    , clickSound(nullptr)
    , backgroundMusic(nullptr)
{
    setupUI();
    setupBackgroundImage();
    setupAudioEffects();
    setupBackgroundMusic();
}

ModeSelectWindow::~ModeSelectWindow()
{
    stopBackgroundMusic();
    // Qt的父子关系会自动清理内存
}

void ModeSelectWindow::setupUI()
{
    // 设置窗口属性
    setWindowTitle("Snake Game - Mode Select");
    setFixedSize(1536, 1175);
    
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
    backgroundLabel->setGeometry(0, 0, 1536, 1175);
    backgroundLabel->setScaledContents(true);
    
    // 创建按钮
    directionButton = new QPushButton("Direction", centralWidget);
    classicButton = new QPushButton("Classic", centralWidget);
    shopButton = new QPushButton("Shop", centralWidget);
    exitButton = new QPushButton("Exit", centralWidget);
    
    // 设置按钮样式（完全透明）
    QString buttonStyle = 
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
    
    directionButton->setStyleSheet(buttonStyle);
    classicButton->setStyleSheet(buttonStyle);
    shopButton->setStyleSheet(buttonStyle);
    exitButton->setStyleSheet(buttonStyle);
    
    // 设置按钮位置（根据modeselect.png的布局调整）
    // 图片尺寸：1536x11756
    directionButton->setGeometry(136, 400, 225, 190);    // Direction按钮
    classicButton->setGeometry(135, 641, 230, 190);       // Classic按钮  
    shopButton->setGeometry(1200, 320, 180, 175);         // Shop按钮
    exitButton->setGeometry(1194, 600, 171, 190);         // Exit按钮
    
    // 连接信号和槽
    connect(directionButton, &QPushButton::clicked, this, &ModeSelectWindow::onDirectionClicked);
    connect(classicButton, &QPushButton::clicked, this, &ModeSelectWindow::onClassicClicked);
    connect(shopButton, &QPushButton::clicked, this, &ModeSelectWindow::onShopClicked);
    connect(exitButton, &QPushButton::clicked, this, &ModeSelectWindow::onExitClicked);
    
    // 为按钮安装事件过滤器以捕获鼠标事件
    directionButton->installEventFilter(this);
    classicButton->installEventFilter(this);
    shopButton->installEventFilter(this);
    exitButton->installEventFilter(this);
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
    playClickSound();
    emit storyModeSelected();
}

void ModeSelectWindow::onClassicClicked()
{
    playClickSound();
    emit classicModeSelected();
}

void ModeSelectWindow::onShopClicked()
{
    playClickSound();
    emit shopRequested();
}

void ModeSelectWindow::onExitClicked()
{
    playClickSound();
    emit exitGameRequested();
}

void ModeSelectWindow::setupAudioEffects()
{
    // 获取程序所在目录的绝对路径
    QString appDir = QCoreApplication::applicationDirPath();
    
    // 初始化悬停音效
    hoverSound = new QSoundEffect(this);
    QString hoverSoundPath = appDir + "/assets/music/mode_select_hover.wav";
    hoverSound->setSource(QUrl::fromLocalFile(hoverSoundPath));
    hoverSound->setVolume(0.7);  // 设置音量为70%
    
    // 初始化点击音效
    clickSound = new QSoundEffect(this);
    QString clickSoundPath = appDir + "/assets/music/mode_select_click.wav";
    clickSound->setSource(QUrl::fromLocalFile(clickSoundPath));
    clickSound->setVolume(0.8);  // 设置音量为80%
    
    // 检查音效是否加载成功
    if (hoverSound->status() != QSoundEffect::Ready) {
        qDebug() << "悬停音效加载失败:" << hoverSoundPath;
    }
    if (clickSound->status() != QSoundEffect::Ready) {
        qDebug() << "点击音效加载失败:" << clickSoundPath;
    }
}

void ModeSelectWindow::setupBackgroundMusic()
{
    // 获取程序所在目录的绝对路径
    QString appDir = QCoreApplication::applicationDirPath();
    QString musicPath = appDir + "/assets/music/mode_background.mp3";
    
    // 初始化背景音乐
    backgroundMusic = new QMediaPlayer(this);
    backgroundMusic->setMedia(QUrl::fromLocalFile(musicPath));
    backgroundMusic->setVolume(80);  // 设置音量为80%
    
    // 设置循环播放
    connect(backgroundMusic, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            backgroundMusic->play();  // 播放结束时重新开始
        }
    });
    
    // 检查音乐是否加载成功
    if (backgroundMusic->error() != QMediaPlayer::NoError) {
        qDebug() << "背景音乐加载失败:" << backgroundMusic->errorString() << "路径:" << musicPath;
    } else {
        qDebug() << "模式选择背景音乐加载成功:" << musicPath;
    }
}

void ModeSelectWindow::startBackgroundMusic()
{
    if (backgroundMusic && backgroundMusic->error() == QMediaPlayer::NoError) {
        backgroundMusic->play();
        qDebug() << "开始播放模式选择背景音乐";
    }
}

void ModeSelectWindow::stopBackgroundMusic()
{
    if (backgroundMusic && backgroundMusic->state() == QMediaPlayer::PlayingState) {
        backgroundMusic->stop();
        qDebug() << "停止播放模式选择背景音乐";
    }
}

void ModeSelectWindow::playHoverSound()
{
    if (hoverSound && hoverSound->status() == QSoundEffect::Ready) {
        hoverSound->play();
    }
}

void ModeSelectWindow::playClickSound()
{
    if (clickSound && clickSound->status() == QSoundEffect::Ready) {
        clickSound->play();
    }
}

bool ModeSelectWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 检查是否是按钮对象
    QPushButton *button = qobject_cast<QPushButton*>(obj);
    if (button && (button == directionButton || button == classicButton || button == shopButton || button == exitButton)) {
        
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

void ModeSelectWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    startBackgroundMusic();
}

void ModeSelectWindow::hideEvent(QHideEvent *event)
{
    QMainWindow::hideEvent(event);
    stopBackgroundMusic();
} 