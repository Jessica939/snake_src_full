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
    , m_cartoonLabel(nullptr)
    , m_skipHintLabel(nullptr)
    , m_scrollArea(nullptr)
    , m_clickSound(nullptr)
    , m_typewriterSound(nullptr)
    , m_backgroundMusic(nullptr)
    , m_currentSegmentIndex(0)
    , m_currentLevel(0)
    , m_isShowingVictoryStory(false)
    , m_typewriterTimer(new QTimer(this))
    , m_currentCharIndex(0)
    , m_isTyping(false)
    , m_currentCartoonIndex(0)
    , m_isInCartoonMode(false)
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
    
    // ===== 📝 剧情文字显示区域 =====
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFixedSize(800, 430);  // 📍 文字框大小
    m_scrollArea->setStyleSheet("QScrollArea { background: rgba(255, 255, 255, 0); border: none; border-radius: 20px; }");
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 剧情文本标签 - 仅用于文字显示
    m_storyTextLabel = new QLabel();
    m_storyTextLabel->setWordWrap(true);
    m_storyTextLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_storyTextLabel->setStyleSheet("QLabel { color: #2F2F2F; font-size: 20px; font-family: 'Georgia', serif; padding: 45px; line-height: 2.8; font-weight: bold; background: rgba(255, 255, 255, 0); }");
    m_storyTextLabel->setMinimumSize(800, 430);  // 📍 文字区域大小
    m_scrollArea->setWidget(m_storyTextLabel);
    
    // ===== 🎨 漫画显示区域 =====
    m_cartoonLabel = new QLabel(this);
    m_cartoonLabel->setFixedSize(600, 1000);  // 📍 漫画框大小（宽×高）
    m_cartoonLabel->setAlignment(Qt::AlignCenter);
    m_cartoonLabel->setStyleSheet("QLabel { background: rgba(255, 255, 255, 0); border: none; }");
    m_cartoonLabel->setVisible(false);  // 初始隐藏
    
    // 跳过提示标签 - 调整样式
    m_skipHintLabel = new QLabel("点击任意位置继续 | 按ESC跳过", this);
    m_skipHintLabel->setStyleSheet("QLabel { color:rgb(184, 164, 52); font-size: 14px; font-family: '微软雅黑'; background: rgba(0, 0, 0, 150); padding: 12px; border-radius: 8px; border: 2px solid #FFD700; }");
    m_skipHintLabel->setAlignment(Qt::AlignCenter);
    m_skipHintLabel->setVisible(false);
    
    // 布局设置
    m_mainLayout->addWidget(m_backgroundLabel);
    
    // ===== 📝 文字框位置设置 =====
    m_scrollArea->setParent(this);
    m_scrollArea->move(350, 240);  // 📍 文字框位置 (X, Y)
    
    // ===== 🎨 漫画框位置设置 =====  
    m_cartoonLabel->setParent(this);
    m_cartoonLabel->move(468, 10);  // 📍 漫画框位置 (X, Y) - 居中偏上
    
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
    m_victorySegments.clear();
    m_levelSegmentRanges.clear();
    m_victorySegmentRanges.clear();
    m_levelSegmentRanges.resize(6); // 0=序章, 1-5=各关卡
    m_victorySegmentRanges.resize(6); // 0=序章, 1-5=各关卡
    
    QString allContent = in.readAll();
    file.close();
    
    QStringList lines = allContent.split('\n');
    
    // 存储各关卡的剧情内容和通关剧情内容
    QStringList levelContents;
    QStringList victoryContents;
    for (int i = 0; i < 6; ++i) {
        levelContents << QString();
        victoryContents << QString();
    }
    
    QString currentLevelContent;
    QString currentVictoryContent;
    int currentLevel = -1;
    bool inTextBlock = false;
    bool inVictorySection = false;
    
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        
        // 跳过空行和标题行
        if (trimmedLine.isEmpty() || trimmedLine.startsWith("《") || 
            trimmedLine.startsWith("English") || trimmedLine.contains("Description")) {
            continue;
        }
        
        // 检测序章开始 - 跳过序章内容，因为Level1有类似内容
        if (trimmedLine.contains("Prologue:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
                victoryContents[currentLevel] = currentVictoryContent;
            }
            currentLevel = -1; // 跳过序章内容
            currentLevelContent = "";
            currentVictoryContent = "";
            inTextBlock = false;
            inVictorySection = false;
            continue;
        }
        
        // 检测各关卡开始
        if (trimmedLine.contains("Level 1:")) {
            // 正常处理Level1
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
                victoryContents[currentLevel] = currentVictoryContent;
            }
            currentLevel = 1;
            currentLevelContent = "";
            currentVictoryContent = "";
            inTextBlock = false;
            inVictorySection = false;
            continue;
        }
        if (trimmedLine.contains("Level 2:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
                victoryContents[currentLevel] = currentVictoryContent;
            }
            currentLevel = 2;
            currentLevelContent = "";
            currentVictoryContent = "";
            inTextBlock = false;
            inVictorySection = false;
            continue;
        }
        if (trimmedLine.contains("Level 3:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
                victoryContents[currentLevel] = currentVictoryContent;
            }
            currentLevel = 3;
            currentLevelContent = "";
            currentVictoryContent = "";
            inTextBlock = false;
            inVictorySection = false;
            continue;
        }
        if (trimmedLine.contains("Level 4:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
                victoryContents[currentLevel] = currentVictoryContent;
            }
            currentLevel = 4;
            currentLevelContent = "";
            currentVictoryContent = "";
            inTextBlock = false;
            inVictorySection = false;
            continue;
        }
        if (trimmedLine.contains("Final Level:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
                victoryContents[currentLevel] = currentVictoryContent;
            }
            currentLevel = 5;
            currentLevelContent = "";
            currentVictoryContent = "";
            inTextBlock = false;
            inVictorySection = false;
            continue;
        }
        
        // 结束当前关卡（遇到Epilogue）
        if (trimmedLine.contains("Epilogue:")) {
            if (currentLevel >= 0) {
                levelContents[currentLevel] = currentLevelContent;
                victoryContents[currentLevel] = currentVictoryContent;
            }
            currentLevel = -1;
            continue;
        }
        
        // 检测通关剧情开始
        if (trimmedLine.contains("[After the level]")) {
            inVictorySection = true;
            inTextBlock = false;
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
            if (inVictorySection) {
                // 保存通关剧情内容
                if (!currentVictoryContent.isEmpty()) {
                    currentVictoryContent += "\n";
                }
                currentVictoryContent += trimmedLine;
            } else {
                // 保存关卡前剧情内容
                if (!currentLevelContent.isEmpty()) {
                    currentLevelContent += "\n";
                }
                currentLevelContent += trimmedLine;
            }
        }
    }
    
    // 保存最后一个关卡
    if (currentLevel >= 0) {
        levelContents[currentLevel] = currentLevelContent;
        victoryContents[currentLevel] = currentVictoryContent;
    }
    
    // 将每个关卡的内容分割成段落并建立索引
    for (int level = 0; level < levelContents.size(); ++level) {
        // 处理关卡前剧情
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
        
        // 处理通关剧情
        if (!victoryContents[level].isEmpty()) {
            int startIndex = m_victorySegments.size();
            
            // 按段落分割（双换行分割）
            QStringList paragraphs = victoryContents[level].split("\n\n", Qt::SkipEmptyParts);
            if (paragraphs.isEmpty()) {
                // 如果没有双换行，按单换行分割
                paragraphs = victoryContents[level].split("\n", Qt::SkipEmptyParts);
            }
            
            for (const QString& paragraph : paragraphs) {
                QString cleanParagraph = paragraph.trimmed();
                if (!cleanParagraph.isEmpty()) {
                    m_victorySegments.push_back(cleanParagraph.toStdString());
                }
            }
            
            int endIndex = m_victorySegments.size();
            m_victorySegmentRanges[level] = qMakePair(startIndex, endIndex);
        } else {
            m_victorySegmentRanges[level] = qMakePair(-1, -1);
        }
    }
}

void StoryDisplayWindow::showPrologue()
{
    // 序章内容现在合并到Level1中，直接跳到地图选择
    emit storyFinished();
}

void StoryDisplayWindow::loadStoryForLevel(int level)
{
    if (level < 1 || level > 5) {
        emit storyFinished();
        return;
    }
    
    m_currentLevel = level;
    m_isShowingVictoryStory = false;
    
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

void StoryDisplayWindow::showVictoryStoryForLevel(int level)
{
    if (level < 1 || level > 5) {
        emit storyFinished();
        return;
    }
    
    m_currentLevel = level;
    m_isShowingVictoryStory = true;
    
    // 检查通关剧情是否存在
    if (m_victorySegmentRanges.size() > level && m_victorySegmentRanges[level].first >= 0) {
        m_currentSegmentIndex = m_victorySegmentRanges[level].first;
        showVictorySegment();
    } else {
        // 如果没有找到通关剧情，直接完成
        emit storyFinished();
    }
}

void StoryDisplayWindow::showVictorySegment()
{
    // 检查是否已经到达当前关卡通关剧情的结尾
    if (m_currentLevel >= 0 && m_currentLevel < m_victorySegmentRanges.size()) {
        QPair<int, int> range = m_victorySegmentRanges[m_currentLevel];
        if (range.first >= 0 && m_currentSegmentIndex >= range.second) {
            // 已经到达当前关卡通关剧情的结尾
            emit storyFinished();
            return;
        }
    }
    
    // 检查是否超出所有通关剧情段落
    if (m_currentSegmentIndex >= static_cast<int>(m_victorySegments.size())) {
        emit storyFinished();
        return;
    }
    
    m_targetText = QString::fromStdString(m_victorySegments[m_currentSegmentIndex]);
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
        if (m_isShowingVictoryStory) {
            showVictorySegment();
        } else {
            showNextSegment();
        }
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
    if (m_isInCartoonMode) {
        // 漫画模式下的键盘处理
        switch (event->key()) {
            case Qt::Key_Escape:
            case Qt::Key_Space:
            case Qt::Key_Return:
                onNextCartoon();
                break;
            default:
                QMainWindow::keyPressEvent(event);
                break;
        }
    } else {
        // 剧情模式下的键盘处理
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
}

void StoryDisplayWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isInCartoonMode) {
            onNextCartoon();
        } else {
            if (m_clickSound) {
                m_clickSound->play();
            }
            onNextSegment();
        }
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

// ========== 漫画相关函数实现 ==========

void StoryDisplayWindow::showCartoonForLevel(int level, const QString& trigger)
{
    m_cartoonPaths.clear();
    m_currentLevel = level; // 设置当前关卡
    
    // 🎯 按照关卡编号配置漫画显示
    if (level == 1 && trigger == "start") {
        // 第1关开始前 - 包含序章漫画
        m_cartoonPaths << getCartoonPath("0_0.png");
        m_cartoonPaths << getCartoonPath("0_1.png");
    }
    else if (level == 1 && trigger == "victory") {
        // 第1关胜利后
        m_cartoonPaths << getCartoonPath("1_0.png");
    }
    else if (level == 2 && trigger == "victory") {
        // 第2关胜利后
        m_cartoonPaths << getCartoonPath("2_0.png");
    }
    else if (level == 3 && trigger == "victory") {
        // 第3关胜利后
        m_cartoonPaths << getCartoonPath("3_0.png");
    }
    else if (level == 4 && trigger == "victory") {
        // 第4关胜利后 - 多张漫画
        m_cartoonPaths << getCartoonPath("4_0.png");
        m_cartoonPaths << getCartoonPath("4_1.png");
    }
    else if (level == 5 && trigger == "victory") {
        // 第5关胜利后 - 结局漫画
        m_cartoonPaths << getCartoonPath("5_0.png");
        m_cartoonPaths << getCartoonPath("5_1.png");
        m_cartoonPaths << getCartoonPath("5_1'.png");
        m_cartoonPaths << getCartoonPath("5_2.png");
    }
    
    if (!m_cartoonPaths.isEmpty()) {
        m_currentCartoonIndex = 0;
        m_isInCartoonMode = true;
        showCartoonImage(m_cartoonPaths[0]);
        
        // 不显示提示文本
        m_skipHintLabel->setVisible(true);
    } else {
        // 没有漫画要显示，直接触发完成信号
        emit storyFinished();
    }
}

void StoryDisplayWindow::showCartoonImage(const QString& path)
{
    // 🎨 漫画纯净显示（专为1024×1536竖版漫画优化）：
    // 
    // 📏 如何调大/调小漫画显示尺寸：
    //    1. 【智能自适应模式】当前默认自动放大20%，可修改放大倍数：
    //       找到 "targetWidth * 1.2f" 这行，修改1.2f为其他值：
    //       - 1.0f = 原尺寸  1.5f = 放大50%  2.0f = 放大100%
    //    2. 【固定尺寸模式】替换智能尺寸部分为：
    //       QSize displaySize = getOptimalCartoonSize("large"); // 使用预设尺寸
    //    3. 【可选预设(2:3比例)】: 
    //       "tiny"(320×480), "small"(400×600), "medium"(480×720), "large"(560×840), 
    //       "xlarge"(640×960), "full"(768×1152), "original"(1024×1536), "compact"(360×540)
    //    4. 【自定义尺寸】直接指定：QSize displaySize(宽度, 高度); // 如QSize(600, 900);
    //
    // 🎭 纯净显示模式：
    //    - 无边框、无阴影、无装饰
    //    - 透明背景
    //    - 完全聚焦于漫画内容本身
    
    if (QFile::exists(path)) {
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            // 🎨 针对1024×1536漫画的智能尺寸调整（2:3纵向比例）
            // 获取漫画显示区域的可用大小，为竖版漫画优化边距
            QSize availableSize = m_cartoonLabel->size();
            int maxWidth = qMax(600, availableSize.width() - 120);    // 充分利用漫画框宽度，预留边距
            int maxHeight = qMax(1400, availableSize.height() - 120); // 充分利用漫画框高度，预留边距
            
            // 根据原始1024×1536比例(2:3)选择合适的显示尺寸
            QSize displaySize;
            float aspectRatio = 1024.0f / 1536.0f; // 原始宽高比 ≈ 0.667
            
            // 优先保证更大的显示尺寸，适当调大显示效果
            int targetWidth = qMin(maxWidth, static_cast<int>(maxHeight * aspectRatio));
            int targetHeight = static_cast<int>(targetWidth / aspectRatio);
            
            // 确保高度不超限，如果超限则按高度重新计算
            if (targetHeight > maxHeight) {
                targetHeight = maxHeight;
                targetWidth = static_cast<int>(targetHeight * aspectRatio);
            }
            
            // 适当放大显示尺寸（增加20%），提供更好的视觉效果
            targetWidth = static_cast<int>(targetWidth * 1.2f);
            targetHeight = static_cast<int>(targetHeight * 1.2f);
            
            // 再次检查是否超出边界，如果超出则回调到安全尺寸
            if (targetWidth > maxWidth || targetHeight > maxHeight) {
                float scale = qMin(static_cast<float>(maxWidth) / targetWidth, 
                                 static_cast<float>(maxHeight) / targetHeight);
                targetWidth = static_cast<int>(targetWidth * scale);
                targetHeight = static_cast<int>(targetHeight * scale);
            }
            
            // 设置最终显示尺寸，保持2:3比例
            displaySize = QSize(targetWidth, targetHeight);
            
            QPixmap scaledPixmap = pixmap.scaled(displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            
            // 创建纯净的画布
            QPixmap canvas(scaledPixmap.width(), scaledPixmap.height());
            canvas.fill(Qt::transparent);
            
            QPainter painter(&canvas);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
            
            // 🎨 纯净显示 - 无边框无阴影
            QRect imageRect = QRect(0, 0, scaledPixmap.width(), scaledPixmap.height());
            painter.drawPixmap(imageRect, scaledPixmap);
            
            // 设置艺术化漫画到专门的漫画标签
            m_cartoonLabel->clear();
            m_cartoonLabel->setPixmap(canvas);
            m_cartoonLabel->setAlignment(Qt::AlignCenter);
            m_cartoonLabel->setVisible(true);  // 显示漫画框
            
            // 隐藏文字框，显示漫画框
            m_scrollArea->setVisible(false);
            
            // 🎭 纯净样式
            QString artStyle = QString(
                "QLabel {"
                "   background: transparent;"
                "   border: none;"
                "   padding: 0px;"
                "   margin: 0px;"
                "}"
            );
            m_cartoonLabel->setStyleSheet(artStyle);
            
        }
    } else {
        // 🎨 艺术化的错误提示
        m_storyTextLabel->setText("🎭 漫画作品暂时无法展示\n" + path);
        m_storyTextLabel->setStyleSheet(
            "QLabel { "
            "   color: #8B4513; "
            "   font-size: 22px; "
            "   font-family: 'Georgia', serif; "
            "   font-weight: bold; "
            "   text-align: center; "
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
            "       stop:0 rgba(255, 228, 196, 240), "
            "       stop:0.5 rgba(255, 248, 220, 200), "
            "       stop:1 rgba(255, 255, 240, 220)); "
            "   border-radius: 25px; "
            "   padding: 40px; "
            "   border: 3px dashed rgba(139, 69, 19, 180);"
            "   margin: 20px;"
            "}"
        );
    }
}

QString StoryDisplayWindow::getCartoonPath(const QString& filename)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString cartoonPath = appDir + "/assets/cartoon/" + filename;
    
    // 如果相对路径不存在，尝试绝对路径
    if (!QFile::exists(cartoonPath)) {
        cartoonPath = "/home/hamilton/snake_src_full/assets/cartoon/" + filename;
    }
    
    return cartoonPath;
}

// 🎨 艺术效果配置函数 - 针对1024×1536漫画的尺寸预设
QSize StoryDisplayWindow::getOptimalCartoonSize(const QString& sizePreset)
{
    // 📏 基于原始1024×1536比例(2:3)的尺寸预设
    if (sizePreset == "tiny") {
        return QSize(320, 480);        // 2:3 超小尺寸，适合小屏幕
    } else if (sizePreset == "small") {
        return QSize(400, 600);        // 2:3 小尺寸，适合细节展示
    } else if (sizePreset == "medium") {
        return QSize(480, 720);        // 2:3 中等尺寸，平衡效果
    } else if (sizePreset == "large") {
        return QSize(560, 840);        // 2:3 大尺寸，标准展示
    } else if (sizePreset == "xlarge") {
        return QSize(640, 960);        // 2:3 超大尺寸，震撼效果
    } else if (sizePreset == "full") {
        return QSize(768, 1152);       // 2:3 接近全尺寸显示
    } else if (sizePreset == "original") {
        return QSize(1024, 1536);      // 原始尺寸（如果空间足够）
    } else if (sizePreset == "compact") {
        return QSize(360, 540);        // 2:3 紧凑尺寸，节省空间
    } else {
        return QSize(480, 720);        // 默认中等尺寸，兼容性最佳
    }
}

void StoryDisplayWindow::onNextCartoon()
{
    if (m_clickSound) {
        m_clickSound->play();
    }
    
    if (m_currentCartoonIndex < m_cartoonPaths.size() - 1) {
        // 显示下一张漫画
        m_currentCartoonIndex++;
        showCartoonImage(m_cartoonPaths[m_currentCartoonIndex]);
    } else {
        // 已经是最后一张，完成漫画显示
        m_isInCartoonMode = false;
        
        // 隐藏漫画框，显示文字框
        m_cartoonLabel->setVisible(false);
        m_scrollArea->setVisible(true);
        
        // 恢复原来的文本样式
        m_storyTextLabel->clear();
        m_storyTextLabel->setStyleSheet("QLabel { color: #2F2F2F; font-size: 20px; font-family: 'Georgia', serif; padding: 45px; line-height: 2.8; font-weight: bold; background: rgba(255, 255, 255, 0); }");
        m_skipHintLabel->setText("点击任意位置继续 | 按ESC跳过");
        
        // 漫画播放完成，触发剧情完成信号
        emit storyFinished();
    }
} 