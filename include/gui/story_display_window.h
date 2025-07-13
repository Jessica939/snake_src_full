#ifndef STORY_DISPLAY_WINDOW_H
#define STORY_DISPLAY_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QTextEdit>
#include <QScrollArea>
#include <vector>
#include <string>
#include <QVector>
#include <QPair>

class StoryDisplayWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StoryDisplayWindow(QWidget *parent = nullptr);
    ~StoryDisplayWindow();
    
    void loadStoryForLevel(int level);  // 加载指定关卡的剧情
    void showPrologue();                // 显示序章
    void showEpilogue();                // 显示尾声

signals:
    void storyFinished();               // 剧情播放完成信号
    void skipToGame();                  // 跳过剧情直接开始游戏

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onTypewriterTimer();           // 打字机效果计时器
    void onNextSegment();               // 显示下一段
    void onSkipAnimation();             // 跳过当前动画

private:
    void setupUI();                     // 设置界面
    void setupBackgroundImage();        // 设置背景图片
    void setupAudioEffects();           // 设置音效
    void setupBackgroundMusic();        // 设置背景音乐
    void startBackgroundMusic();        // 开始播放背景音乐
    void stopBackgroundMusic();         // 停止播放背景音乐
    
    void loadStoryText();               // 加载剧情文本
    void parseStorySegments();          // 解析剧情片段
    void startTypewriterEffect();       // 开始打字机效果
    void completeCurrentSegment();      // 完成当前段落显示
    void showNextSegment();             // 显示下一个段落
    void showSkipHint();                // 显示跳过提示
    void hideSkipHint();                // 隐藏跳过提示
    
    // UI组件
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QLabel *m_backgroundLabel;
    QLabel *m_storyTextLabel;
    QLabel *m_skipHintLabel;
    QScrollArea *m_scrollArea;
    
    // 音效和音乐
    QSoundEffect *m_clickSound;
    QSoundEffect *m_typewriterSound;
    QMediaPlayer *m_backgroundMusic;
    
    // 剧情内容
    std::vector<std::string> m_storySegments;   // 所有剧情段落
    QVector<QPair<int, int>> m_levelSegmentRanges; // 每个关卡的段落范围 [start, end)
    int m_currentSegmentIndex;                  // 当前段落索引
    int m_currentLevel;                         // 当前关卡
    
    // 打字机效果
    QTimer *m_typewriterTimer;
    QString m_currentText;
    QString m_targetText;
    int m_currentCharIndex;
    bool m_isTyping;
    
    // 常量
    static const int TYPEWRITER_SPEED = 50; // 打字速度（毫秒）
    static const int SKIP_HINT_DELAY = 3000; // 跳过提示延迟（毫秒）
};

#endif // STORY_DISPLAY_WINDOW_H 