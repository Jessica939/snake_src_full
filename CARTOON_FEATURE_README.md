# 漫画功能说明

## 概述
已经成功为贪吃蛇游戏添加了漫画显示功能。在特定的剧情节点后，游戏会自动显示相应的漫画图片。

## 功能特性

### 漫画显示时机
1. **0_0.png** - 在prologue（序章）剧情播放完成后显示
2. **1_0.png** - 在level1关卡胜利后显示
3. **5_1'.png** - 在level5关卡前的剧情播放完成后显示

### 漫画显示窗口功能
- 支持多张漫画的浏览
- 使用左右箭头键或鼠标点击进行翻页
- 支持跳过功能（ESC键或点击跳过按钮）
- 自动缩放适应屏幕尺寸
- 美观的用户界面

### 操作方式
- **左右箭头键**: 浏览上一张/下一张漫画
- **空格键**: 下一张漫画
- **鼠标左键**: 下一张漫画
- **ESC键**: 跳过漫画显示
- **上一张/下一张按钮**: 鼠标操作
- **跳过按钮**: 鼠标操作跳过

## 文件结构

### 新增文件
```
include/gui/cartoon_display_window.h    # 漫画显示窗口头文件
gui/cartoon_display_window.cpp          # 漫画显示窗口实现
assets/cartoon/                         # 漫画资源目录
├── 0_0.png                            # 序章后显示的漫画
├── 1_0.png                            # level1胜利后显示的漫画
└── 5_1'.png                           # level5前剧情后显示的漫画
```

### 修改的文件
- `include/gui/gui_manager.h`: 添加了漫画管理功能
- `gui/gui_manager.cpp`: 实现了漫画显示逻辑
- `src/main.cpp`: 添加了level1胜利后漫画显示
- `include/game.h`: 将isLevelCompleted()方法设为公有
- `Makefile`: 添加了新文件的编译规则

## 使用说明

### 添加新漫画
1. 将漫画图片放置在 `assets/cartoon/` 目录下
2. 支持的图片格式：PNG, JPG, JPEG, BMP等Qt支持的格式
3. 建议分辨率：适合1536x1024显示窗口的图片

### 自定义漫画显示
要为其他关卡添加漫画，可以修改以下文件：

#### 在GUI管理器中添加新的触发条件 (`gui/gui_manager.cpp`)
```cpp
void GUIManager::onCartoonFinished()
{
    // 添加新的触发条件处理
    if (mCurrentCartoonTrigger == "your_new_trigger") {
        // 处理新触发条件的逻辑
    }
}
```

#### 在漫画显示窗口中添加新的关卡映射 (`gui/cartoon_display_window.cpp`)
```cpp
void CartoonDisplayWindow::showCartoonsForLevel(int level, const QString& trigger)
{
    QStringList cartoons;
    
    // 添加新的关卡和触发条件
    if (level == 2 && trigger == "victory") {
        cartoons << getCartoonPath("2_0.png");
    }
    // ... 其他条件
    
    showCartoons(cartoons);
}
```

### 测试漫画功能
1. 编译游戏：`make`
2. 运行游戏：`./snakegame`
3. 选择剧情模式
4. 完成序章或level1来查看漫画效果

## 注意事项

1. **图片文件路径**: 漫画文件必须放在正确的路径下，否则会显示加载失败的提示
2. **文件命名**: 确保文件名完全匹配，包括特殊字符（如 `5_1'.png` 中的单引号）
3. **图片大小**: 大尺寸图片会自动缩放，但建议使用适当尺寸以获得最佳显示效果
4. **编译依赖**: 需要Qt5开发环境支持

## 扩展建议

1. **添加音效**: 可以为翻页添加音效支持
2. **动画效果**: 可以添加页面切换的动画效果
3. **更多触发点**: 可以为每个关卡的不同阶段添加漫画显示
4. **漫画序列**: 支持为同一个触发点显示多张连续漫画

## 故障排除

### 常见问题
1. **漫画不显示**: 检查图片文件是否存在于正确路径
2. **编译错误**: 确保Qt5开发环境已正确安装
3. **窗口显示异常**: 检查图形环境是否支持Qt应用

### 调试信息
漫画加载失败时会在漫画窗口中显示错误信息，包含具体的文件路径。

---

现在你可以开始使用新的漫画功能了！将真实的漫画图片替换assets/cartoon/目录下的占位符文件，即可在游戏中看到精美的漫画展示。 