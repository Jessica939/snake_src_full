# 贪吃蛇游戏项目结构说明

## 项目概述
这是一个集成了Qt GUI和ncurses的贪吃蛇游戏，支持剧情模式和经典模式。

## 目录结构

```
snake_src_full/
├── src/                    # 核心源代码
│   ├── main.cpp           # 主程序入口
│   ├── game.cpp           # 游戏逻辑实现
│   ├── snake.cpp          # 蛇类实现
│   └── map.cpp            # 地图类实现
│
├── include/               # 头文件目录
│   ├── game.h            # 游戏类头文件
│   ├── snake.h           # 蛇类头文件
│   ├── map.h             # 地图类头文件
│   └── gui/              # Qt GUI头文件
│       ├── gui_manager.h           # GUI管理器
│       ├── mode_select_window.h    # 模式选择窗口
│       └── story_level_window.h    # 剧情关卡选择窗口
│
├── gui/                  # Qt GUI实现文件
│   ├── gui_manager.cpp           # GUI管理器实现
│   ├── mode_select_window.cpp    # 模式选择窗口实现
│   └── story_level_window.cpp    # 剧情关卡选择窗口实现
│
├── assets/               # 资源文件目录
│   └── images/          # 图片资源
│       ├── modeselect.png      # 模式选择界面背景
│       └── worldmap.png        # 剧情关卡地图背景
│
├── maps/                # 游戏地图文件
│   ├── map1.txt         # 地图1
│   ├── map2.txt         # 地图2
│   ├── map3.txt         # 地图3
│   ├── level1.txt       # 剧情关卡1
│   ├── level2.txt       # 剧情关卡2
│   ├── level3.txt       # 剧情关卡3
│   ├── level4.txt       # 剧情关卡4
│   └── level5.txt       # 剧情关卡5
│
├── Makefile             # 编译配置文件
├── readme.md            # 原始说明文档
├── .gitignore          # Git忽略配置
├── record.dat          # 排行榜数据文件
├── level_progress.dat  # 关卡进度文件
└── snakegame           # 编译生成的可执行文件
```

## 编译产生的临时文件
以下文件是编译过程中产生的，不应加入版本控制：
- `*.o` - 目标文件
- `*_moc.cpp` - Qt MOC生成的文件
- `*_moc.o` - MOC目标文件

## 核心功能模块

### 1. 主程序 (main.cpp)
- 程序入口点
- GUI环境检测和自动回退机制
- Qt GUI和ncurses模式的切换逻辑

### 2. 游戏核心 (game.cpp/game.h)
- 游戏主逻辑
- 关卡系统
- ncurses界面渲染
- 文件I/O处理

### 3. Qt GUI模块
- **GUIManager**: 管理所有GUI窗口
- **ModeSelectWindow**: 模式选择界面
- **StoryLevelWindow**: 剧情关卡选择界面

### 4. 游戏对象
- **Snake**: 蛇的行为和渲染
- **Map**: 地图数据和碰撞检测

## 特性
- **智能启动**: 自动检测图形环境，失败时回退到ncurses模式
- **剧情模式**: 5个逐步解锁的关卡
- **经典模式**: 传统的地图选择游戏
- **进度保存**: 自动保存关卡解锁进度
- **美观界面**: Qt GUI提供现代化的用户界面 