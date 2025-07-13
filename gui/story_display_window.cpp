#include "gui/story_display_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QFont>
#include <QFontMetrics>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QRegularExpression>
#include <QScrollBar>
#include <QDebug>

StoryDisplayWindow::StoryDisplayWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_backgroundLabel(nullptr)
    , m_storyTextLabel(nullptr)
    , m_skipHintLabel(nullptr)
    , m_scrollArea(nullptr)
    , m_clickSound(nullptr)
    , m_typewriterSound(nullptr)
    , m_backgroundMusic(nullptr)
    , m_currentSegmentIndex(0)
    , m_currentLevel(0)
    , m_typewriterTimer(new QTimer(this))
    , m_currentCharIndex(0)
    , m_isTyping(false)
{
    setupUI();
    setupBackgroundImage();
    setupAudioEffects();
    setupBackgroundMusic();
    loadStoryText();
    
    // 连接打字机定时器
    connect(m_typewriterTimer, &QTimer::timeout, this, &StoryDisplayWindow::onTypewriterTimer);
    
    // 设置窗口属性
    setWindowTitle("剧情");
    setFixedSize(1536, 1024);
    
    // 居中显示
    QDesktopWidget *desktop = QApplication::desktop();
    int x = (desktop->width() - width()) / 2;
    int y = (desktop->height() - height()) / 2;
    move(x, y);
}

StoryDisplayWindow::~StoryDisplayWindow()
{
    if (m_backgroundMusic) {
        m_backgroundMusic->stop();
        delete m_backgroundMusic;
    }
    if (m_clickSound) {
        delete m_clickSound;
    }
    if (m_typewriterSound) {
        delete m_typewriterSound;
    }
}

void StoryDisplayWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 背景标签
    m_backgroundLabel = new QLabel(this);
    m_backgroundLabel->setFixedSize(1536, 1024);
    m_backgroundLabel->setScaledContents(true);
    
    // 创建滚动区域 - 调整为更适合storyboard的尺寸和位置
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFixedSize(800, 430);  // 📍 文字框大小调整位置
    m_scrollArea->setStyleSheet("QScrollArea { background: rgba(255, 255, 255, 0); border: none; border-radius: 20px; }");
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 剧情文本标签 - 使用Georgia字体和古朴黑色
    m_storyTextLabel = new QLabel();
    m_storyTextLabel->setWordWrap(true);
    m_storyTextLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_storyTextLabel->setStyleSheet("QLabel { color: #2F2F2F; font-size: 20px; font-family: 'Georgia', serif; padding: 45px; line-height: 2.8; font-weight: bold; background: rgba(255, 255, 255, 0); }");
    m_storyTextLabel->setMinimumSize(800, 430);  // 📍 文本区域大小调整位置    
    m_scrollArea->setWidget(m_storyTextLabel);
    
    // 跳过提示标签 - 调整样式
    m_skipHintLabel = new QLabel("点击任意位置继续 | 按ESC跳过", this);
    m_skipHintLabel->setStyleSheet("QLabel { color:rgb(184, 164, 52); font-size: 14px; font-family: '微软雅黑'; background: rgba(0, 0, 0, 150); padding: 12px; border-radius: 8px; border: 2px solid #FFD700; }");
    m_skipHintLabel->setAlignment(Qt::AlignCenter);
    m_skipHintLabel->setVisible(false);
    
    // 布局设置
    m_mainLayout->addWidget(m_backgroundLabel);
    
    // 将滚动区域定位到窗口底部（像剧情板一样）
    m_scrollArea->setParent(this);
    m_scrollArea->move(370, 280);  // 调整到底部中央
    
    // 跳过提示定位到中间偏下
    m_skipHintLabel->setParent(this);
    m_skipHintLabel->resize(320, 50);
    m_skipHintLabel->move(608, 800);  // 中间偏下位置 (1536-320)/2 = 608, Y=800
}

void StoryDisplayWindow::setupBackgroundImage()
{
    QString imagePath = QCoreApplication::applicationDirPath() + "/assets/images/storyboard.png";
    QPixmap backgroundPixmap(imagePath);
    
    if (backgroundPixmap.isNull()) {
        // 如果没有storyboard图片，使用默认背景
        imagePath = QCoreApplication::applicationDirPath() + "/assets/images/modeselect.png";
        backgroundPixmap = QPixmap(imagePath);
    }
    
    if (!backgroundPixmap.isNull()) {
        m_backgroundLabel->setPixmap(backgroundPixmap);
    } else {
        // 创建默认背景
        backgroundPixmap = QPixmap(1024, 768);
        backgroundPixmap.fill(QColor(20, 20, 40));
        m_backgroundLabel->setPixmap(backgroundPixmap);
    }
}

void StoryDisplayWindow::setupAudioEffects()
{
    // 点击音效
    m_clickSound = new QSoundEffect(this);
    QString clickSoundPath = QCoreApplication::applicationDirPath() + "/assets/music/mode_select_click.wav";
    m_clickSound->setSource(QUrl::fromLocalFile(clickSoundPath));
    m_clickSound->setVolume(0.3);
    
    // 打字机音效（使用悬停音效作为替代）
    m_typewriterSound = new QSoundEffect(this);
    QString typeSoundPath = QCoreApplication::applicationDirPath() + "/assets/music/mode_select_hover.wav";
    m_typewriterSound->setSource(QUrl::fromLocalFile(typeSoundPath));
    m_typewriterSound->setVolume(0.1);
}

void StoryDisplayWindow::setupBackgroundMusic()
{
    m_backgroundMusic = new QMediaPlayer(this);
    QString musicPath = QCoreApplication::applicationDirPath() + "/assets/music/story_background.mp3";
    m_backgroundMusic->setMedia(QUrl::fromLocalFile(musicPath));
    m_backgroundMusic->setVolume(60);
    
    // 设置循环播放
    connect(m_backgroundMusic, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            m_backgroundMusic->play();  // 播放结束时重新开始
        }
    });
    
    // 检查音乐是否加载成功
    if (m_backgroundMusic->error() != QMediaPlayer::NoError) {
        qDebug() << "剧情背景音乐加载失败:" << m_backgroundMusic->errorString() << "路径:" << musicPath;
    } else {
        qDebug() << "剧情背景音乐加载成功:" << musicPath;
    }
}

void StoryDisplayWindow::startBackgroundMusic()
{
    if (m_backgroundMusic && m_backgroundMusic->error() == QMediaPlayer::NoError) {
        // 重新连接循环播放的信号
        connect(m_backgroundMusic, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
            if (state == QMediaPlayer::StoppedState) {
                m_backgroundMusic->play();  // 播放结束时重新开始
            }
        });
        m_backgroundMusic->play();
        qDebug() << "开始播放剧情背景音乐";
    }
}

void StoryDisplayWindow::stopBackgroundMusic()
{
    if (m_backgroundMusic && m_backgroundMusic->state() == QMediaPlayer::PlayingState) {
        // 先断开可能的信号连接，防止stop()后立即重新播放
        m_backgroundMusic->disconnect();
        m_backgroundMusic->stop();
        qDebug() << "停止播放剧情背景音乐";
    }
}

void StoryDisplayWindow::loadStoryText()
{
    QString filePath = QCoreApplication::applicationDirPath() + "/assets/text/plot.txt";
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开剧情文件:" << filePath;
        return;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();
    
    // 解析文本内容
    parseStorySegments();
}

void StoryDisplayWindow::parseStorySegments()
{
    QString filePath = QCoreApplication::applicationDirPath() + "/assets/text/plot.txt";
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    m_storySegments.clear();
    m_levelSegmentRanges.clear();
    m_levelSegmentRanges.resize(6); // 0=序章, 1-5=各关卡
    
    QString allContent = in.readAll();
    file.close();
    
    QStringList lines = allContent.split('\n');
    
    // 存储各关卡的剧情内容
    QStringList levelContents;
    for (int i = 0; i < 6; ++i) {
        levelContents << QString();
    }
    
    QString currentLevelContent;
    int currentLevel = -1;
    bool inTextBlock = false;
    
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        
        // 跳过空行和标题行
        if (trimmedLine.isEmpty() || trimmedLine.startsWith("《") || 
            trimmedLine.startsWith("English") || trimmedLine.contains("Description")) {
            continue;
        }
        
        // 检测序章开始
        if (trimmedLine.contains("Prologue:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
            }
            currentLevel = 0;
            currentLevelContent = "";
            inTextBlock = false;
            continue;
        }
        
        // 检测各关卡开始
        if (trimmedLine.contains("Level 1:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
            }
            currentLevel = 1;
            currentLevelContent = "";
            inTextBlock = false;
            continue;
        }
        if (trimmedLine.contains("Level 2:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
            }
            currentLevel = 2;
            currentLevelContent = "";
            inTextBlock = false;
            continue;
        }
        if (trimmedLine.contains("Level 3:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
            }
            currentLevel = 3;
            currentLevelContent = "";
            inTextBlock = false;
            continue;
        }
        if (trimmedLine.contains("Level 4:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
            }
            currentLevel = 4;
            currentLevelContent = "";
            inTextBlock = false;
            continue;
        }
        if (trimmedLine.contains("Final Level:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
            }
            currentLevel = 5;
            currentLevelContent = "";
            inTextBlock = false;
            continue;
        }
        
        // 结束当前关卡（遇到Epilogue）
        if (trimmedLine.contains("Epilogue:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
            }
            currentLevel = -1;
            continue;
        }
        
        // 检测文本开始
        if (trimmedLine.startsWith("[Text]:") || trimmedLine.startsWith("[Scene]:")) {
            inTextBlock = true;
            continue;
        }
        
        // 检测其他标记
        if (trimmedLine.startsWith("[") && trimmedLine.endsWith("]")) {
            inTextBlock = false;
            continue;
        }
        
        // 收集文本内容
        if (currentLevel >= 0 && inTextBlock && !trimmedLine.isEmpty()) {
            if (!currentLevelContent.isEmpty()) {
                currentLevelContent += "\n";
            }
            currentLevelContent += trimmedLine;
        }
    }
    
    // 保存最后一个关卡
    if (currentLevel >= 0) {
        levelContents[currentLevel] = currentLevelContent;
    }
    
    // 将每个关卡的内容分割成段落并建立索引
    for (int level = 0; level < levelContents.size(); ++level) {
        if (!levelContents[level].isEmpty()) {
            int startIndex = m_storySegments.size();
            
            // 按段落分割（双换行分割）
            QStringList paragraphs = levelContents[level].split("\n\n", Qt::SkipEmptyParts);
            if (paragraphs.isEmpty()) {
                // 如果没有双换行，按单换行分割
                paragraphs = levelContents[level].split("\n", Qt::SkipEmptyParts);
            }
            
            for (const QString& paragraph : paragraphs) {
                QString cleanParagraph = paragraph.trimmed();
                if (!cleanParagraph.isEmpty()) {
                    m_storySegments.push_back(cleanParagraph.toStdString());
                }
            }
            
            int endIndex = m_storySegments.size();
            m_levelSegmentRanges[level] = qMakePair(startIndex, endIndex);
        } else {
            m_levelSegmentRanges[level] = qMakePair(-1, -1);
        }
    }
}

void StoryDisplayWindow::showPrologue()
{
    m_currentLevel = 0;
    
    // 检查序章是否存在
    if (m_levelSegmentRanges.size() > 0 && m_levelSegmentRanges[0].first >= 0) {
        m_currentSegmentIndex = m_levelSegmentRanges[0].first;
        showNextSegment();
    } else {
        // 如果没有序章，直接完成
        emit storyFinished();
    }
}

void StoryDisplayWindow::loadStoryForLevel(int level)
{
    if (level < 1 || level > 5) {
        emit storyFinished();
        return;
    }
    
    m_currentLevel = level;
    
    // 检查关卡剧情是否存在
    if (m_levelSegmentRanges.size() > level && m_levelSegmentRanges[level].first >= 0) {
        m_currentSegmentIndex = m_levelSegmentRanges[level].first;
        showNextSegment();
    } else {
        // 如果没有找到关卡剧情，直接完成
        emit storyFinished();
    }
}

void StoryDisplayWindow::showEpilogue()
{
    m_currentLevel = -1;
    
    // 找到结尾段落
    for (size_t i = 0; i < m_storySegments.size(); ++i) {
        QString segment = QString::fromStdString(m_storySegments[i]);
        if (segment.contains("Epilogue:")) {
            m_currentSegmentIndex = i;
            break;
        }
    }
    
    showNextSegment();
}

void StoryDisplayWindow::showNextSegment()
{
    // 检查是否已经到达当前关卡的结尾
    if (m_currentLevel >= 0 && m_currentLevel < m_levelSegmentRanges.size()) {
        QPair<int, int> range = m_levelSegmentRanges[m_currentLevel];
        if (range.first >= 0 && m_currentSegmentIndex >= range.second) {
            // 已经到达当前关卡的结尾
            emit storyFinished();
            return;
        }
    }
    
    // 检查是否超出所有段落
    if (m_currentSegmentIndex >= static_cast<int>(m_storySegments.size())) {
        emit storyFinished();
        return;
    }
    
    m_targetText = QString::fromStdString(m_storySegments[m_currentSegmentIndex]);
    m_currentText.clear();
    m_currentCharIndex = 0;
    m_isTyping = true;
    
    // 清空显示
    m_storyTextLabel->clear();
    
    // 开始打字机效果
    startTypewriterEffect();
    
    // 显示跳过提示
    QTimer::singleShot(SKIP_HINT_DELAY, this, &StoryDisplayWindow::showSkipHint);
}

void StoryDisplayWindow::startTypewriterEffect()
{
    if (m_typewriterTimer->isActive()) {
        m_typewriterTimer->stop();
    }
    
    m_typewriterTimer->start(TYPEWRITER_SPEED);
}

void StoryDisplayWindow::onTypewriterTimer()
{
    if (m_currentCharIndex < m_targetText.length()) {
        m_currentText += m_targetText[m_currentCharIndex];
        m_currentCharIndex++;
        
        m_storyTextLabel->setText(m_currentText);
        
        // 播放打字音效（每3个字符播放一次）
        if (m_currentCharIndex % 3 == 0 && m_typewriterSound) {
            m_typewriterSound->play();
        }
        
        // 自动滚动到底部
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
    } else {
        m_typewriterTimer->stop();
        m_isTyping = false;
        showSkipHint();
    }
}

void StoryDisplayWindow::completeCurrentSegment()
{
    if (m_isTyping) {
        m_typewriterTimer->stop();
        m_currentText = m_targetText;
        m_storyTextLabel->setText(m_currentText);
        m_isTyping = false;
        showSkipHint();
    }
}

void StoryDisplayWindow::onNextSegment()
{
    if (m_isTyping) {
        completeCurrentSegment();
    } else {
        hideSkipHint();
        m_currentSegmentIndex++;
        showNextSegment();
    }
}

void StoryDisplayWindow::onSkipAnimation()
{
    if (m_isTyping) {
        completeCurrentSegment();
    }
}

void StoryDisplayWindow::showSkipHint()
{
    m_skipHintLabel->setVisible(true);
}

void StoryDisplayWindow::hideSkipHint()
{
    m_skipHintLabel->setVisible(false);
}

void StoryDisplayWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit skipToGame();
    } else if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return) {
        if (m_clickSound) {
            m_clickSound->play();
        }
        onNextSegment();
    }
    
    QMainWindow::keyPressEvent(event);
}

void StoryDisplayWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_clickSound) {
            m_clickSound->play();
        }
        onNextSegment();
    }
    
    QMainWindow::mousePressEvent(event);
}

void StoryDisplayWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    startBackgroundMusic();
}

void StoryDisplayWindow::hideEvent(QHideEvent *event)
{
    QMainWindow::hideEvent(event);
    stopBackgroundMusic();
} 