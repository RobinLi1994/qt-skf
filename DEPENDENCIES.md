# 第三方依赖管理

本文件记录了项目使用的第三方依赖及其版本信息，确保构建的可重复性。

## ElaWidgetTools

**用途：** Fluent-UI 风格的 Qt6 组件库，提供现代化的 UI 界面

**集成方式：** CMake FetchContent（源码构建）

**版本信息：**
- 仓库: https://github.com/Liniyous/ElaWidgetTools
- Commit: `6d46c5a4fd95cc2ad76099f849e3fe2465dff5a3`
- 分支: main
- 锁定日期: 2026-02-11
- 构建类型: 静态库

**版本历史：**
| 日期 | Commit | 说明 |
|------|--------|------|
| 2026-02-11 | 6d46c5a4 | 初始版本锁定 |

**更新步骤：**

1. 检查上游更新
```bash
git ls-remote https://github.com/Liniyous/ElaWidgetTools.git HEAD
```

2. 在本地测试新版本
```bash
# 临时修改 CMakeLists.txt 中的 GIT_TAG
rm -rf build
make build
make run
```

3. 测试通过后更新 CMakeLists.txt 和本文档

4. 提交版本变更
```bash
git add CMakeLists.txt DEPENDENCIES.md
git commit -m "chore(deps): update ElaWidgetTools to [commit-hash]"
```

## Qt 6

**版本：** 6.10.2 (系统安装)

**必需组件：**
- Qt6::Core
- Qt6::Widgets
- Qt6::WidgetsPrivate
- Qt6::Network
- Qt6::HttpServer
- Qt6::Test (开发环境)

**安装方式：**
- macOS: 通过 Qt 官方安装程序安装
- 其他平台: 参考 README.md

## 依赖管理原则

1. **版本锁定：** 所有第三方依赖必须锁定到具体版本（commit hash 或 tag）
2. **文档更新：** 每次更新依赖都必须更新本文档
3. **测试验证：** 依赖更新前必须进行完整的编译和功能测试
4. **变更记录：** 在版本历史表中记录每次依赖变更的原因
