# 项目指令：基于 SFML 的 C++ Wordle 游戏开发计划

## 1. 项目概览

- **目标**：开发一个拥有流畅动画效果的 Wordle 桌面版游戏。
    
- **核心技术栈**：C++ 17/20, SFML (Simple and Fast Multimedia Library), CMake。
    
- **关键特性**：6x5 网格、实时输入响应、平滑的翻转动画、震动提醒动效、完善的词库验证。
    

## 2. 目录结构规范

```
WordleCPP/
├── CMakeLists.txt        # 项目构建配置
├── src/
│   ├── main.cpp          # 入口点及游戏循环
│   ├── Game.cpp/.hpp     # 游戏引擎主类
│   ├── Board.cpp/.hpp    # 棋盘逻辑与渲染
│   ├── Tile.cpp/.hpp     # 最小字母方块类
│   └── Dictionary.cpp/hpp# 词库管理类
├── assets/
│   ├── fonts/            # .ttf 字体文件
│   └── data/             # words.txt (5字母单词表)
└── include/              # (可选) 头文件存放处
```

## 3. 分阶段实施计划 (Tasks for ClaudeCode)

### 阶段一：基础架构与资源准备

- **Task 1.1**: 生成 `CMakeLists.txt`，配置 SFML 依赖库。
    
- **Task 1.2**: 设计 `Dictionary` 类。
    
    - 实现从 `assets/data/words.txt` 加载 5 字母单词的功能。
        
    - 使用 `std::unordered_set` 存储合法词库以实现 O(1) 的查询。
        
- **Task 1.3**: 初始化 `Game` 类。
    
    - 创建 `sf::RenderWindow` (建议 600x800)。
        
    - 实现基本的游戏循环（ProcessInput, Update, Render）。
        

### 阶段二：网格系统与 Tile 类设计

- **Task 2.1**: 实现 `Tile` 类。
    
    - **属性**：`char letter`, `State` (EMPTY, WRONG, PRESENT, CORRECT), `sf::Color`, `sf::RectangleShape`。
        
    - **动画支持**：添加 `float scaleY`（用于翻转）和 `float offset`（用于震动）。
        
- **Task 2.2**: 实现 `Board` 类。
    
    - 管理 6×5 的 `Tile` 二维向量。
        
    - 实现绘制网格的方法，确保居中对齐。
        

### 阶段三：核心游戏逻辑与状态机

- **Task 3.1**: 处理键盘输入。
    
    - 监听 `sf::Event::TextEntered`。
        
    - 处理 `Backspace` 删除和 `Enter` 提交。
        
- **Task 3.2**: 单词对比逻辑。
    
    - 输入 5 个字母后按 Enter 进行比对。
        
    - **算法要求**：必须正确处理重复字母（例如：谜底 ABBEY，输入 BABES，需准确标记每个 B 的状态）。
        

### 阶段四：动画系统（项目的核心）

- **Task 4.1**: 实现“弹出”效果 (Pop Animation)。
    
    - 当字母填入时，方块从 0.8→1.1→1.0 缩放。
        
- **Task 4.2**: 实现“翻转”效果 (Reveal Animation)。
    
    - 提交单词后，整行 Tile 依次沿 X 轴中线翻转（修改 `scaleY`）。
        
    - 翻转到 90 度时切换方块背景颜色。
        
- **Task 4.3**: 实现“震动”效果 (Shake Animation)。
    
    - 当单词不在词库时，该行方块产生正弦波动位的移：x=sin(t)×Intensity。
        

### 阶段五：UI 美化与胜负判定

- **Task 5.1**: 渲染虚拟键盘。
    
    - 在屏幕底部绘制 QWERTY 键盘，并根据玩家猜测同步按键颜色。
        
- **Task 5.2**: 结算界面。
    
    - 游戏结束时，弹出半透明遮罩显示谜底和结果。
        

---

## 4. 技术细节规范 (Technical Specs)

### 颜色定义 (Color Palette)

- **背景**: `#121213` (深黑)
    
- **正确 (Correct)**: `#538d4e` (绿)
    
- **存在 (Present)**: `#b59f3b` (黄)
    
- **错误 (Wrong)**: `#3a3a3c` (深灰)
    
- **未输入**: 边框 `#3a3a3c`
    

### 动画参数

- **翻转耗时**: 0.5s 每格，总计 1.5s 左右完成一行。
    
- **帧率目标**: 强制开启 `window.setFramerateLimit(60)`。
    
- **Delta Time**: 在 `update(float dt)` 中使用 SFML 的 `sf::Clock` 确保平滑度。