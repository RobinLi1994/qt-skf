# wekey-skf UI 重新设计方案

> **日期**: 2026-02-09
> **设计系统**: Material Design 3
> **配色方案**: 专业蓝色系
> **状态**: 已确认，待实施

---

## 1. 设计目标

视觉风格现代化，采用 Material Design 3 设计语言，保持专业蓝色配色，提升用户体验。

## 2. 核心设计原则

### 2.1 卡片化 (Elevation & Surfaces)
- 所有内容使用卡片承载
- Material elevation 阴影层级
- 卡片圆角：8px → 12px

### 2.2 间距系统 (8dp Grid)
- 组件间距：16px
- 页面边距：24px (当前 20px)
- 卡片内边距：20px

### 2.3 排版层级
```
页面标题：24px Bold (当前 20px)
节标题：  18px Medium (新增)
卡片标题：16px Medium
正文：    14px Regular
辅助文字：12px Regular (新增)
```

### 2.4 配色规范
```
主色 Primary:      #2979FF
主色变体 Variant:   #1565C0
次要色 Secondary:   #546E7A
表面色 Surface:     #FFFFFF
背景色 Background:  #F8F9FA
边框色 Outline:     #DFE3E8
```

## 3. 主窗口布局

### 3.1 整体结构
```
┌─────────────────────────────────────────────────────────┐
│  wekey-skf                                    ⚙️ 👤 ─ □ ✕ │
├──────────┬──────────────────────────────────────────────┤
│          │  📍 页面标题                    [操作按钮]    │
│          ├──────────────────────────────────────────────┤
│  导航栏   │                                              │
│  240px   │            内容区域（卡片式）                  │
│  白色     │                                              │
│          │                                              │
├──────────┴──────────────────────────────────────────────┤
│  🟢 状态信息                                              │
└─────────────────────────────────────────────────────────┘
```

### 3.2 导航栏设计

**视觉规格：**
- 背景：#FFFFFF (白色，不再是深蓝 #1A237E)
- 宽度：240px (从 160px 增加)
- 右侧分隔线：1px solid #E8EAED
- 内边距：16px

**导航项状态：**
```
未选中：
  背景：transparent
  文字：#5F6368
  图标：24px
  高度：48px

选中：
  背景：#E8F0FE (浅蓝)
  文字：#1967D2 (深蓝)
  圆角：12px

悬停：
  背景：#F8F9FA
  动画：200ms ease
```

**导航列表（分组）：**
```
设备操作
  📱 设备管理
  📦 应用管理
  🔐 容器管理
  📄 文件管理

系统设置
  🔷 模块管理
  ⚙️  配置管理
  📋 日志查看
```

## 4. 内容区域设计

### 4.1 页面头部（统一模式）
```
标题：24px Bold, #202124
操作按钮：右对齐
  - 主操作：Filled Button
  - 次操作：Outlined Button
```

### 4.2 卡片网格（设备/应用列表）
```
卡片尺寸：280px × 200px
圆角：12px
阴影：elevation-1
间距：16px gap
悬停：elevation-2 + 轻微上移
```

**卡片内容结构：**
```
┌────────────────────┐
│ 🟢 状态 + 标题     │
│ 序列号/描述        │
│ ─────────────────  │
│ 📦 统计信息        │
│ 🔐 统计信息        │
│ ─────────────────  │
│ [详情] [操作]      │
└────────────────────┘
```

### 4.3 表格优化（文件/日志）
- 去除竖线网格
- 行高：56px (从 36px 增加)
- 悬停：背景 #F8F9FA
- 选中：背景 #E8F0FE
- 操作：图标按钮 + Tooltip

### 4.4 空状态
```
图标：64px, #9AA0A6
标题：16px Medium, #5F6368
描述：14px Regular, #80868B
操作：Outlined Button
```

## 5. 组件库规范

### 5.1 按钮类型

**Filled Button (主要操作):**
```css
background: #2979FF
color: #FFFFFF
height: 40px
padding: 0 24px
border-radius: 20px
font-weight: 500
```

**Outlined Button (次要操作):**
```css
border: 1px solid #2979FF
color: #2979FF
background: transparent
height: 40px
border-radius: 20px
```

**Text Button (轻量操作):**
```css
color: #2979FF
background: transparent
height: 36px
padding: 0 12px
```

**Icon Button (图标):**
```css
size: 40px × 40px
border-radius: 20px
```

### 5.2 输入框

**Text Field:**
```
高度：56px
边框：1px solid #E0E0E0
圆角：4px
聚焦：边框 #2979FF, Label 上移
```

**Select/ComboBox:**
```
下拉菜单圆角：8px
选中项背景：#E8F0FE
阴影：elevation-2
```

### 5.3 对话框

**规格：**
```
宽度：480-600px
圆角：28px
内边距：24px
标题：20px Medium
按钮：右对齐，间距 8px
```

### 5.4 徽章与标签

**Status Badge:**
```
高度：24px
圆角：12px
内边距：0 12px
字号：12px Medium

颜色：
🟢 在线：#34A853
🟡 警告：#FBBC04
🔴 错误：#EA4335
⚪ 离线：#9AA0A6
```

**Chip (标签):**
```
高度：32px
背景：#E8F4FD
文字：#1967D2
圆角：16px
```

### 5.5 进度指示器

**Linear Progress:**
```
高度：4px
圆角：2px
背景：#E8EAED
填充：#2979FF
```

**Circular Progress:**
```
直径：40px
粗细：4px
颜色：#2979FF
动画：旋转 1s infinite
```

## 6. 动画与过渡

### 6.1 标准过渡
```css
duration: 200ms
easing: cubic-bezier(0.4, 0.0, 0.2, 1)
```

### 6.2 悬停效果
- 按钮：背景色变化
- 卡片：阴影提升 + 上移 2px
- 导航项：背景淡入

### 6.3 页面切换
- 淡入淡出：300ms
- 内容延迟：50ms (避免闪烁)

## 7. 实施优先级

### P0 (立即实施)
1. 更新全局样式表 (style.qss)
2. 导航栏视觉改造
3. 按钮系统标准化

### P1 (第二阶段)
1. 设备/应用页面卡片化
2. 表格组件优化
3. 对话框现代化

### P2 (优化阶段)
1. 动画细节完善
2. 深色主题支持
3. 响应式布局

## 8. 技术实现

### 8.1 使用技术
- Qt 6.10.2 Widgets
- QSS (Qt Style Sheets)
- QPainter (自定义绘制)

### 8.2 关键类
- `style.qss` - 全局样式
- 各页面类 - 布局调整
- 自定义 Widget - 卡片组件

### 8.3 兼容性
- 保持现有功能完整
- 渐进式改造
- 测试覆盖

---

## 附录：参考资料

- [Material Design 3](https://m3.material.io/)
- [Material Color System](https://m3.material.io/styles/color/system/overview)
- [Material Components](https://m3.material.io/components)
