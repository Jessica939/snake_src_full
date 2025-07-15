#include "gui/cartoon_display_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

CartoonDisplayWindow::CartoonDisplayWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_currentIndex(0)
    , m_hintTimer(new QTimer(this))
    , m_pageSound(nullptr)
    , m_clickSound(nullptr)
{
    setupUI();
    setWindowTitle("漫画");
    setFixedSize(1536, 1024);
    
    QDesktopWidget *desktop = QApplication::desktop();
    int x = (desktop->width() - width()) / 2;
    int y = (desktop->height() - height()) / 2;
    move(x, y);
}

CartoonDisplayWindow::~CartoonDisplayWindow()
{
    if (m_pageSound) delete m_pageSound;
    if (m_clickSound) delete m_clickSound;
}

void CartoonDisplayWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setStyleSheet("QScrollArea { background: black; border: 2px solid #FFD700; }");
    m_scrollArea->setAlignment(Qt::AlignCenter);
    
    m_cartoonLabel = new QLabel();
    m_cartoonLabel->setAlignment(Qt::AlignCenter);
    m_cartoonLabel->setStyleSheet("QLabel { background: black; }");
    m_scrollArea->setWidget(m_cartoonLabel);
    
    m_buttonLayout = new QHBoxLayout();
    
    m_previousButton = new QPushButton("◀ 上一张", this);
    m_nextButton = new QPushButton("下一张 ▶", this);
    m_skipButton = new QPushButton("跳过 ✕", this);
    
    connect(m_previousButton, &QPushButton::clicked, this, &CartoonDisplayWindow::onPreviousCartoon);
    connect(m_nextButton, &QPushButton::clicked, this, &CartoonDisplayWindow::onNextCartoon);
    connect(m_skipButton, &QPushButton::clicked, this, &CartoonDisplayWindow::onSkipCartoons);
    
    m_hintLabel = new QLabel("使用左右箭头键或点击按钮浏览漫画 | 按ESC跳过", this);
    m_hintLabel->setStyleSheet("QLabel { color: #FFD700; font-size: 14px; background: rgba(0,0,0,180); padding: 12px; border-radius: 8px; }");
    m_hintLabel->setAlignment(Qt::AlignCenter);
    m_hintLabel->setVisible(false);
    
    m_buttonLayout->addWidget(m_previousButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_skipButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_nextButton);
    
    m_mainLayout->addWidget(m_scrollArea, 1);
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_hintLabel);
}

void CartoonDisplayWindow::showCartoons(const QStringList& cartoonPaths)
{
    m_cartoonPaths = cartoonPaths;
    m_currentIndex = 0;
    
    if (m_cartoonPaths.isEmpty()) {
        emit cartoonFinished();
        return;
    }
    
    showCurrentCartoon();
    updateNavigationButtons();
}

void CartoonDisplayWindow::showCartoonsForLevel(int level, const QString& trigger)
{
    QStringList cartoons;
    
    if (level == 0 && trigger == "prologue") {
        cartoons << getCartoonPath("0_0.png");
    }
    else if (level == 1 && trigger == "victory") {
        cartoons << getCartoonPath("1_0.png");
    }
    else if (level == 5 && trigger == "pre_story") {
        cartoons << getCartoonPath("5_1'.png");
    }
    
    showCartoons(cartoons);
}

void CartoonDisplayWindow::onNextCartoon()
{
    if (m_currentIndex < m_cartoonPaths.size() - 1) {
        m_currentIndex++;
        showCurrentCartoon();
        updateNavigationButtons();
    } else {
        emit cartoonFinished();
    }
}

void CartoonDisplayWindow::onPreviousCartoon()
{
    if (m_currentIndex > 0) {
        m_currentIndex--;
        showCurrentCartoon();
        updateNavigationButtons();
    }
}

void CartoonDisplayWindow::onSkipCartoons()
{
    emit cartoonFinished();
}

void CartoonDisplayWindow::showCurrentCartoon()
{
    if (m_currentIndex >= 0 && m_currentIndex < m_cartoonPaths.size()) {
        loadCartoon(m_cartoonPaths[m_currentIndex]);
    }
}

void CartoonDisplayWindow::loadCartoon(const QString& path)
{
    if (QFile::exists(path)) {
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            QSize scrollAreaSize = m_scrollArea->size() - QSize(20, 20);
            QPixmap scaledPixmap = pixmap.scaled(scrollAreaSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_cartoonLabel->setPixmap(scaledPixmap);
        }
    } else {
        m_cartoonLabel->setText("无法加载漫画: " + path);
        m_cartoonLabel->setStyleSheet("QLabel { color: red; font-size: 18px; background: black; }");
    }
}

void CartoonDisplayWindow::updateNavigationButtons()
{
    m_previousButton->setEnabled(m_currentIndex > 0);
    m_nextButton->setEnabled(m_currentIndex < m_cartoonPaths.size() - 1);
}

QString CartoonDisplayWindow::getCartoonPath(const QString& filename)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString cartoonPath = appDir + "/assets/cartoon/" + filename;
    
    if (!QFile::exists(cartoonPath)) {
        cartoonPath = "/home/hamilton/snake_src_full/assets/cartoon/" + filename;
    }
    
    return cartoonPath;
}

void CartoonDisplayWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            onSkipCartoons();
            break;
        case Qt::Key_Left:
            onPreviousCartoon();
            break;
        case Qt::Key_Right:
        case Qt::Key_Space:
            onNextCartoon();
            break;
        default:
            QMainWindow::keyPressEvent(event);
            break;
    }
}

void CartoonDisplayWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        onNextCartoon();
    }
    QMainWindow::mousePressEvent(event);
}

void CartoonDisplayWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    m_hintTimer->start(HINT_DELAY);
    connect(m_hintTimer, &QTimer::timeout, this, [this]() {
        m_hintLabel->show();
    });
}

void CartoonDisplayWindow::hideEvent(QHideEvent *event)
{
    QMainWindow::hideEvent(event);
    m_hintTimer->stop();
    m_hintLabel->hide();
}

void CartoonDisplayWindow::setupAudioEffects()
{
    // 可以后续添加音效支持
}

void CartoonDisplayWindow::showCartoon(const QString& cartoonPath)
{
    QStringList paths;
    paths << cartoonPath;
    showCartoons(paths);
} 