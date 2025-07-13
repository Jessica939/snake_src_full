# 图标和界面元素放置指南

## 1. 模式选择界面 (`modeselect.png`)

### 当前按钮位置
在 `gui/mode_select_window.cpp` 的 `setupUI()` 函数中：

```cpp
// 设置按钮位置（根据modeselect.png的布局调整）
// 图片尺寸：1536x1175
directionButton->setGeometry(1136, 325, 225, 190);    // Direction按钮（剧情模式）
classicButton->setGeometry(135, 541, 230, 190);       // Classic按钮（经典模式）
shopButton->setGeometry(1200, 220, 180, 175);         // Shop按钮（商店）
exitButton->setGeometry(1174, 500, 171, 180);         // Exit按钮
```

### 如何调整按钮位置
1. **编辑文件**: `gui/mode_select_window.cpp`
2. **找到函数**: `ModeSelectWindow::setupUI()`
3. **修改坐标**: 
   - `setGeometry(x, y, width, height)`
   - `x, y` 是按钮左上角坐标
   - `width, height` 是按钮尺寸

### 按钮样式自定义（完全透明设计）
在同一函数中可以修改 `buttonStyle` 变量：
```cpp
QString buttonStyle = 
    "QPushButton {"
    "    background-color: transparent;"              // 透明背景
    "    border: none;"                               // 无边框
    "    color: transparent;"                         // 透明文字
    "    font-size: 1px;"                            // 最小字体
    "}"
    "QPushButton:hover {"
    "    background-color: transparent;"              // 悬停时保持透明
    "    border: none;"                               // 悬停时无边框
    "}"
    "QPushButton:pressed {"
    "    background-color: transparent;"              // 按下时保持透明
    "    border: none;"                               // 按下时无边框
    "}"
```

## 2. 剧情关卡地图 (`worldmap.png`)

### 当前关卡按钮位置
在 `gui/story_level_window.cpp` 的 `setupLevelButtons()` 函数中：

```cpp
// 设置关卡按钮位置（根据worldmap.png的布局分布在地图上）
level1Button->setGeometry(150, 450, 50, 50);    // 关卡1 - 左下角开始
level2Button->setGeometry(300, 350, 50, 50);    // 关卡2 - 向右上移动
level3Button->setGeometry(450, 250, 50, 50);    // 关卡3 - 继续向右上
level4Button->setGeometry(600, 200, 50, 50);    // 关卡4 - 右上区域
level5Button->setGeometry(650, 100, 50, 50);    // 关卡5 - 最右上角
backButton->setGeometry(50, 50, 100, 40);       // 返回按钮
```

### 关卡按钮设计理念
- **路径感**: 从左下角到右上角形成游戏进程路径
- **递进式**: 每个关卡都比前一个更高更远
- **视觉平衡**: 避免按钮过于集中或分散

### 如何添加新关卡按钮
1. **在头文件中添加按钮声明**: `include/gui/story_level_window.h`
```cpp
QPushButton *level6Button;  // 新增关卡6按钮
```

2. **在实现文件中创建按钮**: `gui/story_level_window.cpp`
```cpp
level6Button = new QPushButton("6", centralWidget);
level6Button->setGeometry(700, 50, 50, 50);  // 设置位置
```

3. **连接信号槽**:
```cpp
connect(level6Button, &QPushButton::clicked, this, &StoryLevelWindow::onLevel6Clicked);
```

## 3. 关卡按钮状态样式

### 解锁状态（绿色）
```cpp
QString unlockedStyle = 
    "QPushButton {"
    "    background-color: rgba(46, 204, 113, 200);"  // 绿色背景
    "    border: 3px solid white;"
    "    border-radius: 25px;"                        // 圆形按钮
    "    color: white;"
    "    font-size: 20px;"
    "    font-weight: bold;"
    "}";
```

### 锁定状态（灰色）
```cpp
QString lockedStyle = 
    "QPushButton {"
    "    background-color: rgba(149, 165, 166, 200);" // 灰色背景
    "    border: 3px solid gray;"
    "    border-radius: 25px;"
    "    color: gray;"
    "    font-size: 20px;"
    "}";
```

### 完成状态（金色，可选）
如果要添加已完成关卡的特殊显示：
```cpp
QString completedStyle = 
    "QPushButton {"
    "    background-color: rgba(241, 196, 15, 200);"  // 金色背景
    "    border: 3px solid gold;"
    "    border-radius: 25px;"
    "    color: white;"
    "    font-size: 20px;"
    "    font-weight: bold;"
    "}";
```

## 4. 图片资源规格建议

### 模式选择背景 (`modeselect.png`)
- **当前尺寸**: 1536x1175 像素
- **格式**: PNG（支持透明度）
- **设计要点**: 
  - 按钮分布区域：左下角（Classic）、右上角（Direction、Shop、Exit）
  - 可以有装饰性元素，但不要干扰按钮可读性
- **按钮布局**:
  - Direction: 右上区域 (1136, 325)
  - Classic: 左下区域 (135, 541)  
  - Shop: 右上区域 (1200, 220)
  - Exit: 右中区域 (1174, 500)

### 关卡地图背景 (`worldmap.png`)
- **推荐尺寸**: 800x600 像素
- **格式**: PNG
- **设计要点**:
  - 左下到右上应该有明显的"路径"指引
  - 关卡位置应该有明显的地标或特征点
  - 保持整体色调不要过于鲜艳，以免影响按钮可见性

### 关卡图标（可选）
如果要为每个关卡添加特色图标：
- **尺寸**: 64x64 或 32x32 像素
- **位置**: `assets/images/levels/`
- **命名**: `level1_icon.png`, `level2_icon.png` 等

## 5. 实用工具

### 获取鼠标点击坐标
如果需要精确定位按钮位置，可以临时添加此代码到窗口类：
```cpp
void MyWindow::mousePressEvent(QMouseEvent *event) {
    qDebug() << "点击坐标:" << event->x() << "," << event->y();
}
```

### 动态调整工具
创建一个简单的位置调试工具：
```cpp
// 在构造函数中添加
QPushButton *testBtn = new QPushButton("测试", this);
testBtn->setGeometry(0, 0, 100, 30);
connect(testBtn, [=]() {
    // 输出当前所有按钮位置
    qDebug() << "Level1:" << level1Button->geometry();
    qDebug() << "Level2:" << level2Button->geometry();
    // ... 其他按钮
});
```

## 6. 编译和测试

### 重新编译
```bash
cd /home/hamilton/snake_src_full
make clean
make
```

### 测试界面
```bash
# 如果有图形环境，会显示Qt界面
./snakegame

# 没有图形环境会自动回退到ncurses模式
./snakegame
```

### 调试图片加载
如果图片不显示，检查：
1. 图片路径是否正确
2. 图片格式是否支持
3. 文件权限是否正确
4. 在代码中添加调试信息：
```cpp
QPixmap background("assets/images/modeselect.png");
if (background.isNull()) {
    qDebug() << "图片加载失败";
} else {
    qDebug() << "图片加载成功，尺寸:" << background.size();
}
``` 