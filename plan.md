# wekey-skf 技术实现方案

> **版本**: 1.0.0
> **日期**: 2026-02-06
> **作者**: 首席架构师
> **状态**: 待评审

---

## 目录

1. [项目概述](#1-项目概述)
2. [技术栈与工具链](#2-技术栈与工具链)
3. [里程碑规划](#3-里程碑规划)
4. [M1: 基础框架](#4-m1-基础框架)
5. [M2: SKF 驱动插件](#5-m2-skf-驱动插件)
6. [M3: 核心功能](#6-m3-核心功能)
7. [M4: HTTP API](#7-m4-http-api)
8. [M5: GUI 实现](#8-m5-gui-实现)
9. [M6: 测试与打包](#9-m6-测试与打包)
10. [风险评估与缓解](#10-风险评估与缓解)
11. [附录](#11-附录)

---

## 1. 项目概述

### 1.1 目标

将原 Go + CGO 实现的 `ts-wekey-skf` 完整重构为 C++ + Qt 实现，解决 CGO 内存管理问题，同时保持 HTTP API 100% 兼容。

### 1.2 核心约束 (源自 constitution.md)

| 原则 | 要求 |
|------|------|
| **简单性** | YAGNI，标准库优先，反过度工程 |
| **测试先行** | TDD 循环，表格驱动测试 |
| **明确性** | 显式错误处理，无全局变量 |

### 1.3 成功标准

- [ ] 所有原项目功能正常工作
- [ ] HTTP API 通过兼容性测试
- [ ] 无内存泄漏（Valgrind/AddressSanitizer 验证）
- [ ] 设备热插拔不崩溃
- [ ] Windows/macOS 双平台可用

---

## 2. 技术栈与工具链

### 2.1 核心技术

| 组件 | 技术选型 | 版本 | 理由 |
|------|----------|------|------|
| 语言 | C++17 | - | 支持 `std::variant`、`std::optional`、结构化绑定 |
| GUI | Qt Widgets | 6.5+ | 成熟稳定，跨平台，原生外观 |
| HTTP | Qt Network | 6.5+ | 与 Qt 集成，无额外依赖 |
| JSON | QJsonDocument | Qt 内置 | 标准库优先原则 |
| 构建 | CMake | 3.20+ | 跨平台，Qt 官方推荐 |
| 测试 | Qt Test | Qt 内置 | 与 Qt 深度集成 |

### 2.2 开发工具

| 工具 | 用途 |
|------|------|
| Qt Creator / VS Code | IDE |
| clang-format | 代码格式化 |
| clang-tidy | 静态分析 |
| AddressSanitizer | 内存检测 (Debug) |
| Valgrind | 内存泄漏检测 (Linux/macOS) |

### 2.3 目录结构

```
wekey-skf/
├── CMakeLists.txt              # 根 CMake
├── Makefile                    # 便捷命令入口
├── src/
│   ├── CMakeLists.txt
│   ├── app/                    # 应用入口
│   ├── common/                 # 公共工具
│   ├── config/                 # 配置管理
│   ├── log/                    # 日志系统
│   ├── plugin/                 # 插件系统
│   │   ├── interface/          # 插件接口
│   │   └── skf/                # SKF 插件
│   ├── core/                   # 业务逻辑
│   ├── api/                    # HTTP API
│   └── gui/                    # GUI
├── tests/
│   ├── CMakeLists.txt
│   ├── unit/
│   └── integration/
├── resources/
│   ├── icons/
│   ├── lib/
│   └── wekey-skf.qrc
└── docs/
    ├── spec.md
    ├── plan.md
    └── api-sketch.md
```

---

## 3. 里程碑规划

```
┌─────────────────────────────────────────────────────────────────┐
│                        开发时间线                                │
├─────────┬─────────┬─────────┬─────────┬─────────┬─────────────┤
│   M1    │   M2    │   M3    │   M4    │   M5    │     M6      │
│ 基础框架 │ SKF驱动 │ 核心功能 │ HTTP API│ GUI实现 │ 测试与打包   │
├─────────┼─────────┼─────────┼─────────┼─────────┼─────────────┤
│ common/ │ plugin/ │ core/   │ api/    │ gui/    │ 集成测试     │
│ config/ │ skf/    │ device  │ router  │ pages   │ 打包分发     │
│ log/    │         │ app     │ handlers│ dialogs │             │
│ plugin/ │         │ container│        │ tray    │             │
│ interface│        │ crypto  │         │         │             │
└─────────┴─────────┴─────────┴─────────┴─────────┴─────────────┘
```

### 3.1 里程碑依赖关系

```
M1 ──→ M2 ──→ M3 ──┬──→ M4 ──┬──→ M6
                   │         │
                   └──→ M5 ──┘
```

- M1 是所有后续里程碑的基础
- M2 依赖 M1 的插件接口
- M3 依赖 M2 的 SKF 驱动
- M4 和 M5 可并行开发，都依赖 M3
- M6 需要 M4 和 M5 都完成

---

## 4. M1: 基础框架

### 4.1 目标

建立项目骨架，实现基础设施模块，为后续开发奠定基础。

### 4.2 任务分解

#### 4.2.1 项目初始化

**任务 M1.1: 创建 CMake 构建系统**

```cmake
# CMakeLists.txt (根目录)
cmake_minimum_required(VERSION 3.20)
project(wekey-skf VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 依赖
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Test
)

# 子目录
add_subdirectory(src)
add_subdirectory(tests)
```

**任务 M1.2: 创建 Makefile 便捷入口**

```makefile
# Makefile
.PHONY: all build run test clean

BUILD_DIR := build
CMAKE := cmake
CTEST := ctest

all: build

configure:
	$(CMAKE) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

build: configure
	$(CMAKE) --build $(BUILD_DIR) -j$(nproc)

run: build
	./$(BUILD_DIR)/src/wekey-skf

test: build
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
```

#### 4.2.2 公共模块 (src/common/)

**任务 M1.3: 实现 Result<T> 模板**

文件: `src/common/Result.h`

```cpp
#pragma once

#include <variant>
#include "Error.h"

namespace wekey {

template<typename T>
class Result {
public:
    static Result ok(T value) { return Result(std::move(value)); }
    static Result err(Error error) { return Result(std::move(error)); }

    [[nodiscard]] bool isOk() const { return std::holds_alternative<T>(data_); }
    [[nodiscard]] bool isErr() const { return !isOk(); }

    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }
    [[nodiscard]] const Error& error() const { return std::get<Error>(data_); }

    // 链式操作
    template<typename F>
    auto map(F&& f) -> Result<decltype(f(std::declval<T>()))> {
        using U = decltype(f(std::declval<T>()));
        if (isOk()) {
            return Result<U>::ok(f(value()));
        }
        return Result<U>::err(error());
    }

    template<typename F>
    auto andThen(F&& f) -> decltype(f(std::declval<T>())) {
        if (isOk()) {
            return f(value());
        }
        return decltype(f(std::declval<T>()))::err(error());
    }

private:
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(Error error) : data_(std::move(error)) {}
    std::variant<T, Error> data_;
};

// void 特化
template<>
class Result<void> {
public:
    static Result ok() { return Result(true); }
    static Result err(Error error) { return Result(std::move(error)); }

    [[nodiscard]] bool isOk() const { return success_; }
    [[nodiscard]] bool isErr() const { return !success_; }
    [[nodiscard]] const Error& error() const { return error_; }

private:
    explicit Result(bool) : success_(true) {}
    explicit Result(Error e) : success_(false), error_(std::move(e)) {}
    bool success_ = false;
    Error error_;
};

} // namespace wekey
```

**任务 M1.4: 实现 Error 类**

文件: `src/common/Error.h`

```cpp
#pragma once

#include <QString>
#include <cstdint>
#include <QHash>

namespace wekey {

class Error {
public:
    enum Code : uint32_t {
        // 应用层错误码 (0x00 - 0xFF)
        Success         = 0x00,
        Fail            = 0x01,
        InvalidParam    = 0x03,
        NoActiveModule  = 0x04,
        NotLoggedIn     = 0x09,
        NotAuthorized   = 0x0B,
        PortInUse       = 0x10,
        PluginLoadFailed = 0x11,

        // SKF 错误码 (0x0A000000+)
        SkfOk             = 0x00000000,
        SkfFail           = 0x0A000001,
        SkfUnknown        = 0x0A000002,
        SkfNotSupported   = 0x0A000003,
        SkfFileError      = 0x0A000004,
        SkfInvalidHandle  = 0x0A000005,
        SkfInvalidParam   = 0x0A000006,
        SkfDeviceRemoved  = 0x0A000023,
        SkfPinIncorrect   = 0x0A000024,
        SkfPinLocked      = 0x0A000025,
        SkfUserNotLogin   = 0x0A00002D,
        SkfAppNotExists   = 0x0A00002E,
    };

    Error() = default;
    explicit Error(Code code, QString message = {}, QString context = {})
        : code_(code), message_(std::move(message)), context_(std::move(context)) {}

    [[nodiscard]] Code code() const { return code_; }
    [[nodiscard]] const QString& message() const { return message_; }
    [[nodiscard]] const QString& context() const { return context_; }

    [[nodiscard]] QString toString(bool detailed = false) const;
    [[nodiscard]] QString friendlyMessage() const;

    static Error fromSkf(uint32_t skfResult, const QString& function = {});

private:
    Code code_ = Success;
    QString message_;
    QString context_;

    static const QHash<Code, QString>& friendlyMessages();
};

} // namespace wekey
```

文件: `src/common/Error.cpp`

```cpp
#include "Error.h"

namespace wekey {

const QHash<Error::Code, QString>& Error::friendlyMessages() {
    static const QHash<Code, QString> messages = {
        {Success, "操作成功"},
        {Fail, "操作失败"},
        {InvalidParam, "参数无效"},
        {NoActiveModule, "未激活驱动模块"},
        {NotLoggedIn, "未登录"},
        {NotAuthorized, "未授权"},
        {PortInUse, "端口已被占用"},
        {PluginLoadFailed, "插件加载失败"},
        {SkfPinIncorrect, "PIN 码错误"},
        {SkfPinLocked, "PIN 码已锁定"},
        {SkfDeviceRemoved, "设备已移除"},
        {SkfUserNotLogin, "用户未登录"},
        {SkfAppNotExists, "应用不存在"},
    };
    return messages;
}

QString Error::friendlyMessage() const {
    auto it = friendlyMessages().find(code_);
    if (it != friendlyMessages().end()) {
        return it.value();
    }
    return message_.isEmpty() ? "未知错误" : message_;
}

QString Error::toString(bool detailed) const {
    QString result = friendlyMessage();
    if (detailed) {
        result += QString("\n错误码: 0x%1").arg(code_, 8, 16, QChar('0'));
        if (!context_.isEmpty()) {
            result += QString("\n上下文: %1").arg(context_);
        }
    }
    return result;
}

Error Error::fromSkf(uint32_t skfResult, const QString& function) {
    return Error(static_cast<Code>(skfResult), {}, function);
}

} // namespace wekey
```

#### 4.2.3 配置模块 (src/config/)

**任务 M1.5: 实现 Config 类**

文件: `src/config/Config.h`

```cpp
#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QJsonObject>

namespace wekey {

class Config : public QObject {
    Q_OBJECT

public:
    static Config& instance();

    // 禁止拷贝
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    bool load();
    bool save();
    void reset();

    // Getters
    [[nodiscard]] QString listenPort() const;
    [[nodiscard]] QString logLevel() const;
    [[nodiscard]] QString logPath() const;
    [[nodiscard]] bool systrayDisabled() const;
    [[nodiscard]] QString errorMode() const;
    [[nodiscard]] QVariantMap modPaths() const;
    [[nodiscard]] QString activedModName() const;

    // 默认值
    [[nodiscard]] QString defaultAppName() const;
    [[nodiscard]] QString defaultContainerName() const;
    [[nodiscard]] QString defaultCommonName() const;
    [[nodiscard]] QString defaultOrganization() const;
    [[nodiscard]] QString defaultUnit() const;
    [[nodiscard]] QString defaultRole() const;

    // Setters
    void setListenPort(const QString& port);
    void setLogLevel(const QString& level);
    void setLogPath(const QString& path);
    void setSystrayDisabled(bool disabled);
    void setErrorMode(const QString& mode);
    void setModPath(const QString& name, const QString& path);
    void removeModPath(const QString& name);
    void setActivedModName(const QString& name);
    void setDefault(const QString& key, const QString& value);

signals:
    void configChanged();

private:
    Config();

    [[nodiscard]] QString configPath() const;
    [[nodiscard]] QJsonObject defaultConfig() const;

    QJsonObject data_;
};

} // namespace wekey
```

文件: `src/config/Config.cpp`

```cpp
#include "Config.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>

namespace wekey {

Config& Config::instance() {
    static Config instance;
    return instance;
}

Config::Config() {
    data_ = defaultConfig();
}

QString Config::configPath() const {
    return QDir::homePath() + "/.wekeytool.json";
}

QJsonObject Config::defaultConfig() const {
    return QJsonObject{
        {"version", "1.0.0"},
        {"listen_port", ":9001"},
        {"log_level", "info"},
        {"log_path", QStandardPaths::writableLocation(QStandardPaths::TempLocation)},
        {"systray_disabled", false},
        {"error_mode", "simple"},
        {"mod_paths", QJsonObject{}},
        {"actived_mod_name", ""},
        {"defaults", QJsonObject{
            {"appName", "TAGM"},
            {"containerName", "TrustAsia"},
            {"commonName", "TrustAsia"},
            {"organization", "TrustAsia Technologies, Inc."},
            {"unit", "GMCA"},
            {"role", "user"}
        }}
    };
}

bool Config::load() {
    QFile file(configPath());
    if (!file.open(QIODevice::ReadOnly)) {
        // 配置文件不存在，使用默认值
        return true;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError) {
        return false;
    }

    // 合并配置（保留默认值中新增的字段）
    QJsonObject loaded = doc.object();
    QJsonObject defaults = defaultConfig();
    for (auto it = defaults.begin(); it != defaults.end(); ++it) {
        if (!loaded.contains(it.key())) {
            loaded[it.key()] = it.value();
        }
    }
    data_ = loaded;
    return true;
}

bool Config::save() {
    QFile file(configPath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(data_);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    emit configChanged();
    return true;
}

void Config::reset() {
    data_ = defaultConfig();
    emit configChanged();
}

// Getters
QString Config::listenPort() const {
    return data_["listen_port"].toString(":9001");
}

QString Config::logLevel() const {
    return data_["log_level"].toString("info");
}

QString Config::logPath() const {
    return data_["log_path"].toString();
}

bool Config::systrayDisabled() const {
    return data_["systray_disabled"].toBool(false);
}

QString Config::errorMode() const {
    return data_["error_mode"].toString("simple");
}

QVariantMap Config::modPaths() const {
    return data_["mod_paths"].toObject().toVariantMap();
}

QString Config::activedModName() const {
    return data_["actived_mod_name"].toString();
}

QString Config::defaultAppName() const {
    return data_["defaults"].toObject()["appName"].toString("TAGM");
}

QString Config::defaultContainerName() const {
    return data_["defaults"].toObject()["containerName"].toString("TrustAsia");
}

QString Config::defaultCommonName() const {
    return data_["defaults"].toObject()["commonName"].toString("TrustAsia");
}

QString Config::defaultOrganization() const {
    return data_["defaults"].toObject()["organization"].toString();
}

QString Config::defaultUnit() const {
    return data_["defaults"].toObject()["unit"].toString("GMCA");
}

QString Config::defaultRole() const {
    return data_["defaults"].toObject()["role"].toString("user");
}

// Setters
void Config::setListenPort(const QString& port) {
    data_["listen_port"] = port;
}

void Config::setLogLevel(const QString& level) {
    data_["log_level"] = level;
}

void Config::setLogPath(const QString& path) {
    data_["log_path"] = path;
}

void Config::setSystrayDisabled(bool disabled) {
    data_["systray_disabled"] = disabled;
}

void Config::setErrorMode(const QString& mode) {
    data_["error_mode"] = mode;
}

void Config::setModPath(const QString& name, const QString& path) {
    QJsonObject mods = data_["mod_paths"].toObject();
    mods[name] = path;
    data_["mod_paths"] = mods;
}

void Config::removeModPath(const QString& name) {
    QJsonObject mods = data_["mod_paths"].toObject();
    mods.remove(name);
    data_["mod_paths"] = mods;
}

void Config::setActivedModName(const QString& name) {
    data_["actived_mod_name"] = name;
}

void Config::setDefault(const QString& key, const QString& value) {
    QJsonObject defaults = data_["defaults"].toObject();
    defaults[key] = value;
    data_["defaults"] = defaults;
}

} // namespace wekey
```

#### 4.2.4 日志模块 (src/log/)

**任务 M1.6: 实现 Logger 类**

文件: `src/log/Logger.h`

```cpp
#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QMutex>

namespace wekey {

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3
};

struct LogEntry {
    QDateTime timestamp;
    LogLevel level;
    QString message;
    QString source;
};

class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void setLevel(LogLevel level);
    void setOutputPath(const QString& path);

    void debug(const QString& message, const QString& source = {});
    void info(const QString& message, const QString& source = {});
    void warn(const QString& message, const QString& source = {});
    void error(const QString& message, const QString& source = {});

    [[nodiscard]] LogLevel level() const { return level_; }

signals:
    void logAdded(const LogEntry& entry);

private:
    Logger();
    void log(LogLevel level, const QString& message, const QString& source);
    void writeToFile(const LogEntry& entry);

    LogLevel level_ = LogLevel::Info;
    QString outputPath_;
    QFile file_;
    QMutex mutex_;
};

// 便捷宏
#define LOG_DEBUG(msg) wekey::Logger::instance().debug(msg, __FUNCTION__)
#define LOG_INFO(msg)  wekey::Logger::instance().info(msg, __FUNCTION__)
#define LOG_WARN(msg)  wekey::Logger::instance().warn(msg, __FUNCTION__)
#define LOG_ERROR(msg) wekey::Logger::instance().error(msg, __FUNCTION__)

} // namespace wekey

Q_DECLARE_METATYPE(wekey::LogEntry)
```

文件: `src/log/Logger.cpp`

```cpp
#include "Logger.h"
#include <QDir>
#include <QTextStream>

namespace wekey {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    qRegisterMetaType<LogEntry>("LogEntry");
}

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

void Logger::setOutputPath(const QString& path) {
    QMutexLocker locker(&mutex_);

    if (file_.isOpen()) {
        file_.close();
    }

    outputPath_ = path;
    if (!path.isEmpty()) {
        QDir dir(path);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QString filePath = path + "/wekey-skf.log";
        file_.setFileName(filePath);
        file_.open(QIODevice::Append | QIODevice::Text);
    }
}

void Logger::debug(const QString& message, const QString& source) {
    log(LogLevel::Debug, message, source);
}

void Logger::info(const QString& message, const QString& source) {
    log(LogLevel::Info, message, source);
}

void Logger::warn(const QString& message, const QString& source) {
    log(LogLevel::Warn, message, source);
}

void Logger::error(const QString& message, const QString& source) {
    log(LogLevel::Error, message, source);
}

void Logger::log(LogLevel level, const QString& message, const QString& source) {
    if (level < level_) {
        return;
    }

    LogEntry entry{
        QDateTime::currentDateTime(),
        level,
        message,
        source
    };

    writeToFile(entry);
    emit logAdded(entry);
}

void Logger::writeToFile(const LogEntry& entry) {
    QMutexLocker locker(&mutex_);

    if (!file_.isOpen()) {
        return;
    }

    static const char* levelNames[] = {"DEBUG", "INFO", "WARN", "ERROR"};

    QTextStream stream(&file_);
    stream << entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz")
           << " [" << levelNames[static_cast<int>(entry.level)] << "] ";

    if (!entry.source.isEmpty()) {
        stream << "[" << entry.source << "] ";
    }

    stream << entry.message << "\n";
    stream.flush();
}

} // namespace wekey
```

**任务 M1.7: 实现 LogModel (供 GUI 使用)**

文件: `src/log/LogModel.h`

```cpp
#pragma once

#include <QAbstractTableModel>
#include <QList>
#include "Logger.h"

namespace wekey {

class LogModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        Timestamp = 0,
        Level,
        Source,
        Message,
        ColumnCount
    };

    explicit LogModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setFilter(LogLevel minLevel);
    void setSearchText(const QString& text);
    void clear();

private slots:
    void onLogAdded(const LogEntry& entry);

private:
    void applyFilter();

    QList<LogEntry> allEntries_;
    QList<LogEntry> filteredEntries_;
    LogLevel minLevel_ = LogLevel::Debug;
    QString searchText_;
};

} // namespace wekey
```

#### 4.2.5 插件接口 (src/plugin/interface/)

**任务 M1.8: 定义 IDriverPlugin 接口**

文件: `src/plugin/interface/IDriverPlugin.h`

```cpp
#pragma once

#include <QtPlugin>
#include <QList>
#include <QVariantMap>
#include "PluginTypes.h"
#include "common/Result.h"

namespace wekey {

class IDriverPlugin {
public:
    virtual ~IDriverPlugin() = default;

    //=== 设备管理 ===
    virtual Result<QList<DeviceInfo>> enumDevices(bool login = false) = 0;
    virtual Result<void> changeDeviceAuth(const QString& devName,
                                          const QString& oldPin,
                                          const QString& newPin) = 0;
    virtual Result<void> setDeviceLabel(const QString& devName,
                                        const QString& label) = 0;
    virtual Result<int> waitForDeviceEvent() = 0;

    //=== 应用管理 ===
    virtual Result<QList<AppInfo>> enumApps(const QString& devName) = 0;
    virtual Result<void> createApp(const QString& devName,
                                   const QString& appName,
                                   const QVariantMap& args) = 0;
    virtual Result<void> deleteApp(const QString& devName,
                                   const QString& appName) = 0;
    virtual Result<void> loginApp(const QString& devName,
                                  const QString& appName,
                                  const QVariantMap& args) = 0;
    virtual Result<void> logoutApp(const QString& devName,
                                   const QString& appName) = 0;
    virtual Result<void> updateAppPin(const QString& devName,
                                      const QString& appName,
                                      const QVariantMap& args) = 0;
    virtual Result<void> unblockApp(const QString& devName,
                                    const QString& appName,
                                    const QVariantMap& args) = 0;
    virtual Result<int> getRetryCount(const QString& devName,
                                      const QString& appName,
                                      const QString& role) = 0;

    //=== 容器管理 ===
    virtual Result<QList<ContainerInfo>> enumContainers(const QString& devName,
                                                        const QString& appName) = 0;
    virtual Result<void> createContainer(const QString& devName,
                                         const QString& appName,
                                         const QString& containerName) = 0;
    virtual Result<void> deleteContainer(const QString& devName,
                                         const QString& appName,
                                         const QString& containerName) = 0;

    //=== 证书操作 ===
    virtual Result<QByteArray> generateCsr(const QString& devName,
                                           const QString& appName,
                                           const QString& containerName,
                                           const QVariantMap& args) = 0;
    virtual Result<void> importCertificate(const QString& devName,
                                           const QString& appName,
                                           const QString& containerName,
                                           const QVariantMap& args) = 0;
    virtual Result<QList<CertInfo>> exportCertificate(const QString& devName,
                                                      const QString& appName,
                                                      const QString& containerName) = 0;
    virtual Result<bool> verifyCertificate(const QString& devName,
                                           const QString& appName,
                                           const QString& containerName) = 0;

    //=== 签名与验签 ===
    virtual Result<QByteArray> sign(const QString& devName,
                                    const QString& appName,
                                    const QString& containerName,
                                    const QByteArray& data) = 0;
    virtual Result<void> verify(const QString& devName,
                                const QString& appName,
                                const QString& containerName,
                                const QByteArray& data,
                                const QByteArray& signature) = 0;

    //=== 文件操作 ===
    virtual Result<QStringList> enumFiles(const QString& devName,
                                          const QString& appName) = 0;
    virtual Result<void> createFile(const QString& devName,
                                    const QString& appName,
                                    const QString& fileName,
                                    const QByteArray& data,
                                    const QVariantMap& args) = 0;
    virtual Result<QByteArray> readFile(const QString& devName,
                                        const QString& appName,
                                        const QString& fileName) = 0;
    virtual Result<void> deleteFile(const QString& devName,
                                    const QString& appName,
                                    const QString& fileName) = 0;

    //=== 其他 ===
    virtual Result<QByteArray> generateRandom(const QString& devName, int count) = 0;
};

} // namespace wekey

#define IDriverPlugin_iid "com.trustasia.wekey.IDriverPlugin/1.0"
Q_DECLARE_INTERFACE(wekey::IDriverPlugin, IDriverPlugin_iid)
```

**任务 M1.9: 定义数据类型**

文件: `src/plugin/interface/PluginTypes.h`

```cpp
#pragma once

#include <QString>
#include <QDateTime>
#include <QMetaType>

namespace wekey {

struct DeviceInfo {
    QString deviceName;
    QString devicePath;
    QString manufacturer;
    QString label;
    QString serialNumber;
    QString hardwareVersion;
    QString firmwareVersion;
    bool isLoggedIn = false;
};

struct AppInfo {
    QString appName;
    bool isLoggedIn = false;
};

struct ContainerInfo {
    QString containerName;
    bool keyGenerated = false;
    int keyType = 0;  // 0=未知, 1=RSA, 2=SM2
    bool certImported = false;
};

struct CertInfo {
    QString serialNumber;
    QString subject;
    QString commonName;
    QString issuer;
    QDateTime notBefore;
    QDateTime notAfter;
    int certType = 0;  // 1=签名, 2=加密
    QString certPem;
    QString publicKeyHash;
};

enum class KeyType {
    Unknown = 0,
    RSA_2048 = 1,
    RSA_3072 = 2,
    RSA_4096 = 3,
    SM2 = 4
};

// 设备事件类型
enum class DeviceEvent {
    None = 0,
    Inserted = 1,
    Removed = 2
};

} // namespace wekey

Q_DECLARE_METATYPE(wekey::DeviceInfo)
Q_DECLARE_METATYPE(wekey::AppInfo)
Q_DECLARE_METATYPE(wekey::ContainerInfo)
Q_DECLARE_METATYPE(wekey::CertInfo)
```

### 4.3 测试用例 (M1)

**测试 M1.T1: Result<T> 测试**

文件: `tests/unit/test_result.cpp`

```cpp
#include <QtTest>
#include "common/Result.h"

using namespace wekey;

class TestResult : public QObject {
    Q_OBJECT

private slots:
    void testOk() {
        auto result = Result<int>::ok(42);
        QVERIFY(result.isOk());
        QVERIFY(!result.isErr());
        QCOMPARE(result.value(), 42);
    }

    void testErr() {
        auto result = Result<int>::err(Error(Error::Fail, "test error"));
        QVERIFY(!result.isOk());
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::Fail);
    }

    void testVoidOk() {
        auto result = Result<void>::ok();
        QVERIFY(result.isOk());
    }

    void testVoidErr() {
        auto result = Result<void>::err(Error(Error::InvalidParam));
        QVERIFY(result.isErr());
    }

    void testMap() {
        auto result = Result<int>::ok(10);
        auto mapped = result.map([](int x) { return x * 2; });
        QVERIFY(mapped.isOk());
        QCOMPARE(mapped.value(), 20);
    }

    void testMapErr() {
        auto result = Result<int>::err(Error(Error::Fail));
        auto mapped = result.map([](int x) { return x * 2; });
        QVERIFY(mapped.isErr());
    }
};

QTEST_MAIN(TestResult)
#include "test_result.moc"
```

**测试 M1.T2: Config 测试**

文件: `tests/unit/test_config.cpp`

```cpp
#include <QtTest>
#include <QTemporaryDir>
#include "config/Config.h"

using namespace wekey;

class TestConfig : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // 使用临时目录避免污染真实配置
    }

    void testDefaultValues() {
        Config& config = Config::instance();
        config.reset();

        QCOMPARE(config.listenPort(), QString(":9001"));
        QCOMPARE(config.logLevel(), QString("info"));
        QCOMPARE(config.errorMode(), QString("simple"));
        QCOMPARE(config.defaultAppName(), QString("TAGM"));
    }

    void testSettersGetters() {
        Config& config = Config::instance();
        config.reset();

        config.setListenPort(":8080");
        QCOMPARE(config.listenPort(), QString(":8080"));

        config.setErrorMode("detailed");
        QCOMPARE(config.errorMode(), QString("detailed"));
    }

    void testModPaths() {
        Config& config = Config::instance();
        config.reset();

        config.setModPath("skf", "/path/to/skf.dll");
        config.setModPath("p11", "/path/to/p11.dll");

        auto paths = config.modPaths();
        QCOMPARE(paths.size(), 2);
        QCOMPARE(paths["skf"].toString(), QString("/path/to/skf.dll"));

        config.removeModPath("p11");
        paths = config.modPaths();
        QCOMPARE(paths.size(), 1);
    }
};

QTEST_MAIN(TestConfig)
#include "test_config.moc"
```

### 4.4 验收标准 (M1)

- [ ] CMake 构建成功，生成可执行文件
- [ ] `make test` 所有单元测试通过
- [ ] Result<T> 支持值类型和 void 类型
- [ ] Error 类支持 SKF 错误码映射
- [ ] Config 能正确读写 `~/.wekeytool.json`
- [ ] Logger 能输出到文件和发送信号

---

## 5. M2: SKF 驱动插件

### 5.1 目标

实现 SKF 驱动插件，封装对厂商 C 库的调用。

### 5.2 任务分解

#### 5.2.1 SKF C API 封装

**任务 M2.1: 定义 SKF C API 头文件**

文件: `src/plugin/skf/SkfApi.h`

```cpp
#pragma once

#include <cstdint>

#ifdef _WIN32
#define SKF_API __stdcall
#else
#define SKF_API
#endif

// 句柄类型
typedef void* DEVHANDLE;
typedef void* HAPPLICATION;
typedef void* HCONTAINER;
typedef void* HANDLE;

// 基本类型
typedef uint8_t BYTE;
typedef uint8_t* LPSTR;
typedef uint32_t ULONG;
typedef uint32_t* PULONG;
typedef int32_t LONG;
typedef uint16_t WORD;
typedef int BOOL;

// 版本结构
#pragma pack(push, 1)
typedef struct {
    BYTE major;
    BYTE minor;
} VERSION;

// 设备信息
typedef struct {
    VERSION Version;
    BYTE Manufacturer[64];
    BYTE Issuer[64];
    BYTE Label[32];
    BYTE SerialNumber[32];
    VERSION HWVersion;
    VERSION FirmwareVersion;
    ULONG AlgSymCap;
    ULONG AlgAsymCap;
    ULONG AlgHashCap;
    ULONG DevAuthAlgId;
    ULONG TotalSpace;
    ULONG FreeSpace;
    ULONG MaxECCBufferSize;
    ULONG MaxBufferSize;
    BYTE Reserved[64];
} DEVINFO, *PDEVINFO;

// ECC 公钥
typedef struct {
    ULONG BitLen;
    BYTE XCoordinate[64];
    BYTE YCoordinate[64];
} ECCPUBLICKEYBLOB, *PECCPUBLICKEYBLOB;

// ECC 签名
typedef struct {
    BYTE r[64];
    BYTE s[64];
} ECCSIGNATUREBLOB, *PECCSIGNATUREBLOB;

// ECC 密文
typedef struct {
    BYTE XCoordinate[64];
    BYTE YCoordinate[64];
    BYTE Hash[32];
    ULONG CipherLen;
    BYTE Cipher[1];
} ECCCIPHERBLOB, *PECCCIPHERBLOB;

#pragma pack(pop)

// 错误码
#define SAR_OK                  0x00000000
#define SAR_FAIL                0x0A000001
#define SAR_UNKNOWNERR          0x0A000002
#define SAR_NOTSUPPORTYETERR    0x0A000003
#define SAR_FILEERR             0x0A000004
#define SAR_INVALIDHANDLEERR    0x0A000005
#define SAR_INVALIDPARAMERR     0x0A000006
#define SAR_READFILEERR         0x0A000007
#define SAR_WRITEFILEERR        0x0A000008
#define SAR_NAMELENERR          0x0A000009
#define SAR_KEYUSAGEERR         0x0A00000A
#define SAR_MODULUSLENERR       0x0A00000B
#define SAR_NOTINITIALIZEERR    0x0A00000C
#define SAR_OBLOCONFLICTERR     0x0A00000D
#define SAR_DEVICE_REMOVED      0x0A000023
#define SAR_PIN_INCORRECT       0x0A000024
#define SAR_PIN_LOCKED          0x0A000025
#define SAR_USER_NOT_LOGGED_IN  0x0A00002D
#define SAR_APPLICATION_NOT_EXISTS 0x0A00002E

// 函数指针类型定义
extern "C" {

// 设备管理
typedef ULONG (SKF_API *PFN_SKF_EnumDev)(BOOL bPresent, LPSTR szNameList, PULONG pulSize);
typedef ULONG (SKF_API *PFN_SKF_ConnectDev)(LPSTR szName, DEVHANDLE* phDev);
typedef ULONG (SKF_API *PFN_SKF_DisConnectDev)(DEVHANDLE hDev);
typedef ULONG (SKF_API *PFN_SKF_GetDevInfo)(DEVHANDLE hDev, PDEVINFO pDevInfo);
typedef ULONG (SKF_API *PFN_SKF_SetLabel)(DEVHANDLE hDev, LPSTR szLabel);
typedef ULONG (SKF_API *PFN_SKF_DevAuth)(DEVHANDLE hDev, BYTE* pbAuthData, ULONG ulLen);
typedef ULONG (SKF_API *PFN_SKF_ChangeDevAuthKey)(DEVHANDLE hDev, BYTE* pbKeyValue, ULONG ulKeyLen);
typedef ULONG (SKF_API *PFN_SKF_WaitForDevEvent)(LPSTR szDevName, PULONG pulDevNameLen, PULONG pulEvent);

// 应用管理
typedef ULONG (SKF_API *PFN_SKF_EnumApplication)(DEVHANDLE hDev, LPSTR szAppName, PULONG pulSize);
typedef ULONG (SKF_API *PFN_SKF_CreateApplication)(DEVHANDLE hDev, LPSTR szAppName,
                                                   LPSTR szAdminPin, ULONG dwAdminPinRetryCount,
                                                   LPSTR szUserPin, ULONG dwUserPinRetryCount,
                                                   ULONG dwCreateFileRights, HAPPLICATION* phApp);
typedef ULONG (SKF_API *PFN_SKF_DeleteApplication)(DEVHANDLE hDev, LPSTR szAppName);
typedef ULONG (SKF_API *PFN_SKF_OpenApplication)(DEVHANDLE hDev, LPSTR szAppName, HAPPLICATION* phApp);
typedef ULONG (SKF_API *PFN_SKF_CloseApplication)(HAPPLICATION hApp);
typedef ULONG (SKF_API *PFN_SKF_VerifyPIN)(HAPPLICATION hApp, ULONG ulPINType,
                                           LPSTR szPIN, PULONG pulRetryCount);
typedef ULONG (SKF_API *PFN_SKF_ChangePIN)(HAPPLICATION hApp, ULONG ulPINType,
                                           LPSTR szOldPIN, LPSTR szNewPIN, PULONG pulRetryCount);
typedef ULONG (SKF_API *PFN_SKF_UnblockPIN)(HAPPLICATION hApp, LPSTR szAdminPIN,
                                            LPSTR szNewUserPIN, PULONG pulRetryCount);

// 容器管理
typedef ULONG (SKF_API *PFN_SKF_EnumContainer)(HAPPLICATION hApp, LPSTR szContainerName, PULONG pulSize);
typedef ULONG (SKF_API *PFN_SKF_CreateContainer)(HAPPLICATION hApp, LPSTR szContainerName, HCONTAINER* phContainer);
typedef ULONG (SKF_API *PFN_SKF_DeleteContainer)(HAPPLICATION hApp, LPSTR szContainerName);
typedef ULONG (SKF_API *PFN_SKF_OpenContainer)(HAPPLICATION hApp, LPSTR szContainerName, HCONTAINER* phContainer);
typedef ULONG (SKF_API *PFN_SKF_CloseContainer)(HCONTAINER hContainer);
typedef ULONG (SKF_API *PFN_SKF_GetContainerType)(HCONTAINER hContainer, PULONG pulContainerType);

// 密钥操作
typedef ULONG (SKF_API *PFN_SKF_GenECCKeyPair)(HCONTAINER hContainer, ULONG ulAlgId, PECCPUBLICKEYBLOB pBlob);
typedef ULONG (SKF_API *PFN_SKF_ExportPublicKey)(HCONTAINER hContainer, BOOL bSignFlag,
                                                  BYTE* pbBlob, PULONG pulBlobLen);
typedef ULONG (SKF_API *PFN_SKF_ECCSignData)(HCONTAINER hContainer, BYTE* pbData, ULONG ulDataLen,
                                              PECCSIGNATUREBLOB pSignature);
typedef ULONG (SKF_API *PFN_SKF_ECCVerify)(DEVHANDLE hDev, PECCPUBLICKEYBLOB pECCPubKeyBlob,
                                            BYTE* pbData, ULONG ulDataLen, PECCSIGNATUREBLOB pSignature);

// 证书操作
typedef ULONG (SKF_API *PFN_SKF_ImportCertificate)(HCONTAINER hContainer, BOOL bSignFlag,
                                                    BYTE* pbCert, ULONG ulCertLen);
typedef ULONG (SKF_API *PFN_SKF_ExportCertificate)(HCONTAINER hContainer, BOOL bSignFlag,
                                                    BYTE* pbCert, PULONG pulCertLen);

// 随机数
typedef ULONG (SKF_API *PFN_SKF_GenRandom)(DEVHANDLE hDev, BYTE* pbRandom, ULONG ulRandomLen);

// 文件操作
typedef ULONG (SKF_API *PFN_SKF_EnumFiles)(HAPPLICATION hApp, LPSTR szFileList, PULONG pulSize);
typedef ULONG (SKF_API *PFN_SKF_CreateFile)(HAPPLICATION hApp, LPSTR szFileName, ULONG ulFileSize,
                                            ULONG ulReadRights, ULONG ulWriteRights);
typedef ULONG (SKF_API *PFN_SKF_DeleteFile)(HAPPLICATION hApp, LPSTR szFileName);
typedef ULONG (SKF_API *PFN_SKF_ReadFile)(HAPPLICATION hApp, LPSTR szFileName, ULONG ulOffset,
                                          ULONG ulSize, BYTE* pbOutData, PULONG pulOutLen);
typedef ULONG (SKF_API *PFN_SKF_WriteFile)(HAPPLICATION hApp, LPSTR szFileName, ULONG ulOffset,
                                           BYTE* pbData, ULONG ulSize);

} // extern "C"
```

**任务 M2.2: 实现 SkfLibrary 动态加载**

文件: `src/plugin/skf/SkfLibrary.h`

```cpp
#pragma once

#include <QLibrary>
#include <QString>
#include <memory>
#include "SkfApi.h"
#include "common/Result.h"

namespace wekey {

class SkfLibrary {
public:
    explicit SkfLibrary(const QString& path);
    ~SkfLibrary();

    SkfLibrary(const SkfLibrary&) = delete;
    SkfLibrary& operator=(const SkfLibrary&) = delete;

    [[nodiscard]] bool isLoaded() const;
    [[nodiscard]] QString errorString() const;

    // 设备管理
    PFN_SKF_EnumDev EnumDev = nullptr;
    PFN_SKF_ConnectDev ConnectDev = nullptr;
    PFN_SKF_DisConnectDev DisConnectDev = nullptr;
    PFN_SKF_GetDevInfo GetDevInfo = nullptr;
    PFN_SKF_SetLabel SetLabel = nullptr;
    PFN_SKF_DevAuth DevAuth = nullptr;
    PFN_SKF_ChangeDevAuthKey ChangeDevAuthKey = nullptr;
    PFN_SKF_WaitForDevEvent WaitForDevEvent = nullptr;

    // 应用管理
    PFN_SKF_EnumApplication EnumApplication = nullptr;
    PFN_SKF_CreateApplication CreateApplication = nullptr;
    PFN_SKF_DeleteApplication DeleteApplication = nullptr;
    PFN_SKF_OpenApplication OpenApplication = nullptr;
    PFN_SKF_CloseApplication CloseApplication = nullptr;
    PFN_SKF_VerifyPIN VerifyPIN = nullptr;
    PFN_SKF_ChangePIN ChangePIN = nullptr;
    PFN_SKF_UnblockPIN UnblockPIN = nullptr;

    // 容器管理
    PFN_SKF_EnumContainer EnumContainer = nullptr;
    PFN_SKF_CreateContainer CreateContainer = nullptr;
    PFN_SKF_DeleteContainer DeleteContainer = nullptr;
    PFN_SKF_OpenContainer OpenContainer = nullptr;
    PFN_SKF_CloseContainer CloseContainer = nullptr;
    PFN_SKF_GetContainerType GetContainerType = nullptr;

    // 密钥操作
    PFN_SKF_GenECCKeyPair GenECCKeyPair = nullptr;
    PFN_SKF_ExportPublicKey ExportPublicKey = nullptr;
    PFN_SKF_ECCSignData ECCSignData = nullptr;
    PFN_SKF_ECCVerify ECCVerify = nullptr;

    // 证书操作
    PFN_SKF_ImportCertificate ImportCertificate = nullptr;
    PFN_SKF_ExportCertificate ExportCertificate = nullptr;

    // 随机数
    PFN_SKF_GenRandom GenRandom = nullptr;

    // 文件操作
    PFN_SKF_EnumFiles EnumFiles = nullptr;
    PFN_SKF_CreateFile CreateFile = nullptr;
    PFN_SKF_DeleteFile DeleteFile = nullptr;
    PFN_SKF_ReadFile ReadFile = nullptr;
    PFN_SKF_WriteFile WriteFile = nullptr;

private:
    void loadSymbols();

    template<typename T>
    T loadSymbol(const char* name) {
        return reinterpret_cast<T>(lib_.resolve(name));
    }

    QLibrary lib_;
};

} // namespace wekey
```

**任务 M2.3: 实现 SkfPlugin**

文件: `src/plugin/skf/SkfPlugin.h`

```cpp
#pragma once

#include <QObject>
#include <QMutex>
#include <QMap>
#include <memory>
#include "plugin/interface/IDriverPlugin.h"
#include "SkfLibrary.h"

namespace wekey {

struct HandleInfo {
    DEVHANDLE devHandle = nullptr;
    HAPPLICATION appHandle = nullptr;
    HCONTAINER containerHandle = nullptr;
    bool isLoggedIn = false;
};

class SkfPlugin : public QObject, public IDriverPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IDriverPlugin_iid FILE "skf.json")
    Q_INTERFACES(wekey::IDriverPlugin)

public:
    explicit SkfPlugin(QObject* parent = nullptr);
    ~SkfPlugin() override;

    // 初始化
    Result<void> initialize(const QString& libPath);

    // IDriverPlugin 实现
    Result<QList<DeviceInfo>> enumDevices(bool login = false) override;
    Result<void> changeDeviceAuth(const QString& devName,
                                  const QString& oldPin,
                                  const QString& newPin) override;
    Result<void> setDeviceLabel(const QString& devName,
                                const QString& label) override;
    Result<int> waitForDeviceEvent() override;

    Result<QList<AppInfo>> enumApps(const QString& devName) override;
    Result<void> createApp(const QString& devName,
                           const QString& appName,
                           const QVariantMap& args) override;
    Result<void> deleteApp(const QString& devName,
                           const QString& appName) override;
    Result<void> loginApp(const QString& devName,
                          const QString& appName,
                          const QVariantMap& args) override;
    Result<void> logoutApp(const QString& devName,
                           const QString& appName) override;
    Result<void> updateAppPin(const QString& devName,
                              const QString& appName,
                              const QVariantMap& args) override;
    Result<void> unblockApp(const QString& devName,
                            const QString& appName,
                            const QVariantMap& args) override;
    Result<int> getRetryCount(const QString& devName,
                              const QString& appName,
                              const QString& role) override;

    Result<QList<ContainerInfo>> enumContainers(const QString& devName,
                                                const QString& appName) override;
    Result<void> createContainer(const QString& devName,
                                 const QString& appName,
                                 const QString& containerName) override;
    Result<void> deleteContainer(const QString& devName,
                                 const QString& appName,
                                 const QString& containerName) override;

    Result<QByteArray> generateCsr(const QString& devName,
                                   const QString& appName,
                                   const QString& containerName,
                                   const QVariantMap& args) override;
    Result<void> importCertificate(const QString& devName,
                                   const QString& appName,
                                   const QString& containerName,
                                   const QVariantMap& args) override;
    Result<QList<CertInfo>> exportCertificate(const QString& devName,
                                              const QString& appName,
                                              const QString& containerName) override;
    Result<bool> verifyCertificate(const QString& devName,
                                   const QString& appName,
                                   const QString& containerName) override;

    Result<QByteArray> sign(const QString& devName,
                            const QString& appName,
                            const QString& containerName,
                            const QByteArray& data) override;
    Result<void> verify(const QString& devName,
                        const QString& appName,
                        const QString& containerName,
                        const QByteArray& data,
                        const QByteArray& signature) override;

    Result<QStringList> enumFiles(const QString& devName,
                                  const QString& appName) override;
    Result<void> createFile(const QString& devName,
                            const QString& appName,
                            const QString& fileName,
                            const QByteArray& data,
                            const QVariantMap& args) override;
    Result<QByteArray> readFile(const QString& devName,
                                const QString& appName,
                                const QString& fileName) override;
    Result<void> deleteFile(const QString& devName,
                            const QString& appName,
                            const QString& fileName) override;

    Result<QByteArray> generateRandom(const QString& devName, int count) override;

private:
    // 句柄管理
    Result<DEVHANDLE> openDevice(const QString& devName);
    void closeDevice(const QString& devName);
    Result<HAPPLICATION> openApp(const QString& devName, const QString& appName);
    void closeApp(const QString& devName, const QString& appName);
    Result<HCONTAINER> openContainer(const QString& devName,
                                     const QString& appName,
                                     const QString& containerName);
    void closeContainer(const QString& devName,
                        const QString& appName,
                        const QString& containerName);

    QString makeKey(const QString& dev, const QString& app = {},
                    const QString& container = {}) const;

    std::unique_ptr<SkfLibrary> lib_;
    QMap<QString, HandleInfo> handles_;
    QMutex mutex_;
};

} // namespace wekey
```

### 5.3 测试用例 (M2)

**测试 M2.T1: SkfLibrary 加载测试**

```cpp
// 需要真实设备或 Mock 库
void TestSkfPlugin::testLibraryLoad() {
    SkfLibrary lib("/path/to/libskf.dll");
    QVERIFY(lib.isLoaded());
    QVERIFY(lib.EnumDev != nullptr);
    QVERIFY(lib.ConnectDev != nullptr);
}
```

### 5.4 验收标准 (M2)

- [ ] SkfLibrary 能正确加载厂商库
- [ ] 所有 SKF 函数指针正确解析
- [ ] SkfPlugin 实现所有 IDriverPlugin 方法
- [ ] 句柄生命周期正确管理（无泄漏）
- [ ] 错误码正确映射到 Error 类

---

## 6. M3: 核心功能

### 6.1 目标

实现业务逻辑层，提供统一的服务接口供 GUI 和 API 使用。

### 6.2 任务分解

#### 6.2.1 PluginManager (src/plugin/)

**任务 M3.1: 实现插件管理器**

文件: `src/plugin/PluginManager.h`

```cpp
#pragma once

#include <QObject>
#include <QMap>
#include <QPluginLoader>
#include <memory>
#include "interface/IDriverPlugin.h"
#include "common/Result.h"

namespace wekey {

class PluginManager : public QObject {
    Q_OBJECT

public:
    static PluginManager& instance();

    Result<void> registerPlugin(const QString& name, const QString& path);
    Result<void> unregisterPlugin(const QString& name);

    IDriverPlugin* getPlugin(const QString& name);
    IDriverPlugin* activePlugin();

    Result<void> setActivePlugin(const QString& name);
    QStringList listPlugins() const;

    // 从配置加载所有插件
    void loadFromConfig();

signals:
    void pluginRegistered(const QString& name);
    void pluginUnregistered(const QString& name);
    void activePluginChanged(const QString& name);

private:
    PluginManager() = default;

    struct PluginEntry {
        QString path;
        std::unique_ptr<QPluginLoader> loader;
        IDriverPlugin* instance = nullptr;
    };

    QMap<QString, PluginEntry> plugins_;
    QString activeName_;
};

} // namespace wekey
```

#### 6.2.2 DeviceService (src/core/device/)

**任务 M3.2: 实现设备服务**

```cpp
// src/core/device/DeviceService.h

#pragma once

#include <QObject>
#include <QThread>
#include "plugin/interface/PluginTypes.h"
#include "common/Result.h"

namespace wekey {

class DeviceService : public QObject {
    Q_OBJECT

public:
    static DeviceService& instance();

    Result<QList<DeviceInfo>> enumDevices(bool login = false);
    Result<void> changeDeviceAuth(const QString& devName,
                                  const QString& oldPin,
                                  const QString& newPin);
    Result<void> setDeviceLabel(const QString& devName, const QString& label);

    // 启动设备监听线程
    void startDeviceMonitor();
    void stopDeviceMonitor();

signals:
    void deviceInserted(const QString& devName);
    void deviceRemoved(const QString& devName);
    void deviceListChanged();

private:
    DeviceService() = default;
    void monitorLoop();

    QThread* monitorThread_ = nullptr;
    bool monitoring_ = false;
};

} // namespace wekey
```

#### 6.2.3 AppService (src/core/application/)

**任务 M3.3: 实现应用服务**

```cpp
// src/core/application/AppService.h

#pragma once

#include <QObject>
#include "plugin/interface/PluginTypes.h"
#include "common/Result.h"

namespace wekey {

class AppService : public QObject {
    Q_OBJECT

public:
    static AppService& instance();

    Result<QList<AppInfo>> enumApps(const QString& devName);
    Result<void> createApp(const QString& devName,
                           const QString& appName,
                           const QString& adminPin,
                           const QString& userPin);
    Result<void> deleteApp(const QString& devName, const QString& appName);

    Result<void> login(const QString& devName,
                       const QString& appName,
                       const QString& pin,
                       const QString& role);
    Result<void> logout(const QString& devName, const QString& appName);

    Result<void> changePin(const QString& devName,
                           const QString& appName,
                           const QString& oldPin,
                           const QString& newPin,
                           const QString& role);
    Result<void> unblockPin(const QString& devName,
                            const QString& appName,
                            const QString& adminPin,
                            const QString& newUserPin);

    Result<int> getRetryCount(const QString& devName,
                              const QString& appName,
                              const QString& role);

signals:
    void loginStateChanged(const QString& devName, const QString& appName, bool loggedIn);
    void pinError(const QString& devName, const QString& appName, int retryCount);
    void pinLocked(const QString& devName, const QString& appName);

private:
    AppService() = default;
};

} // namespace wekey
```

#### 6.2.4 ContainerService 和 CryptoService

类似模式实现，略。

### 6.3 验收标准 (M3)

- [ ] PluginManager 能动态加载/卸载插件
- [ ] DeviceService 设备监听正常工作
- [ ] AppService 登录/登出流程正确
- [ ] PIN 错误和锁定场景正确处理
- [ ] 所有服务通过单元测试

---

## 7. M4: HTTP API

### 7.1 目标

实现与原 Go 版本 100% 兼容的 HTTP REST API。

### 7.2 任务分解

#### 7.2.1 HttpServer

**任务 M4.1: 实现 HTTP 服务器**

```cpp
// src/api/HttpServer.h

#pragma once

#include <QObject>
#include <QTcpServer>
#include <memory>
#include "common/Result.h"

namespace wekey {

class ApiRouter;

class HttpServer : public QObject {
    Q_OBJECT

public:
    explicit HttpServer(QObject* parent = nullptr);
    ~HttpServer();

    Result<void> start(quint16 port);
    void stop();
    bool isRunning() const;
    quint16 port() const;

signals:
    void started(quint16 port);
    void stopped();
    void requestReceived(const QString& method, const QString& path);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void handleRequest(QTcpSocket* socket, const QByteArray& request);

    std::unique_ptr<QTcpServer> server_;
    std::unique_ptr<ApiRouter> router_;
};

} // namespace wekey
```

#### 7.2.2 ApiRouter

**任务 M4.2: 实现路由分发**

```cpp
// src/api/ApiRouter.h

#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <functional>
#include <QMap>

namespace wekey {

struct HttpRequest {
    QString method;
    QString path;
    QMap<QString, QString> headers;
    QMap<QString, QString> queryParams;
    QByteArray body;
};

struct HttpResponse {
    int statusCode = 200;
    QString statusText = "OK";
    QMap<QString, QString> headers;
    QByteArray body;

    void setJson(const QJsonObject& json);
    void setError(int code, const QString& message);
};

using RouteHandler = std::function<void(const HttpRequest&, HttpResponse&)>;

class ApiRouter : public QObject {
    Q_OBJECT

public:
    explicit ApiRouter(QObject* parent = nullptr);

    void addRoute(const QString& method, const QString& path, RouteHandler handler);
    void handleRequest(const HttpRequest& request, HttpResponse& response);

private:
    void setupRoutes();

    // 公共接口
    void handleHealth(const HttpRequest& req, HttpResponse& res);
    void handleExit(const HttpRequest& req, HttpResponse& res);

    // 业务接口
    void handleEnumDev(const HttpRequest& req, HttpResponse& res);
    void handleLogin(const HttpRequest& req, HttpResponse& res);
    void handleGenCsr(const HttpRequest& req, HttpResponse& res);
    void handleImportCert(const HttpRequest& req, HttpResponse& res);
    void handleExportCert(const HttpRequest& req, HttpResponse& res);
    void handleSign(const HttpRequest& req, HttpResponse& res);
    void handleVerify(const HttpRequest& req, HttpResponse& res);
    void handleRandom(const HttpRequest& req, HttpResponse& res);

    // 管理接口
    void handleModCreate(const HttpRequest& req, HttpResponse& res);
    void handleModActive(const HttpRequest& req, HttpResponse& res);
    void handleModDelete(const HttpRequest& req, HttpResponse& res);
    // ... 其他管理接口

    struct Route {
        QString method;
        QString path;
        RouteHandler handler;
    };

    QList<Route> routes_;
};

} // namespace wekey
```

### 7.3 API 兼容性测试

**任务 M4.3: 编写 API 兼容性测试脚本**

```bash
#!/bin/bash
# tests/integration/test_api_compat.sh

BASE_URL="http://localhost:9001"

# 测试健康检查
echo "Testing /health..."
response=$(curl -s "$BASE_URL/health")
echo "$response" | jq -e '.status == "ok"' || exit 1

# 测试枚举设备
echo "Testing /api/v1/enum-dev..."
response=$(curl -s "$BASE_URL/api/v1/enum-dev")
echo "$response" | jq -e '.code == 0' || exit 1

# 测试登录
echo "Testing /api/v1/login..."
response=$(curl -s -X POST "$BASE_URL/api/v1/login" \
    -H "Content-Type: application/json" \
    -d '{"serialNumber":"TEST","appName":"TAGM","role":"user","pin":"123456"}')
echo "$response" | jq -e '.code' || exit 1

echo "All API compatibility tests passed!"
```

### 7.4 验收标准 (M4)

- [ ] HttpServer 能正确监听端口
- [ ] 端口冲突时返回友好错误
- [ ] 所有 `/api/v1/*` 接口实现
- [ ] 所有 `/admin/*` 接口实现
- [ ] JSON 响应格式与原项目一致
- [ ] 错误码与原项目一致

---

## 8. M5: GUI 实现

### 8.1 目标

实现 Qt Widgets 桌面界面，包括 6 个功能页面和系统托盘。

### 8.2 任务分解

#### 8.2.1 MainWindow

**任务 M5.1: 实现主窗口框架**

```cpp
// src/gui/MainWindow.cpp 核心结构

void MainWindow::setupUi() {
    setWindowTitle("wekey-skf");
    setMinimumSize(900, 600);

    // 中心部件
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧导航
    navList_ = new QListWidget;
    navList_->setFixedWidth(150);
    navList_->addItem("模块管理");
    navList_->addItem("设备管理");
    navList_->addItem("应用管理");
    navList_->addItem("容器管理");
    navList_->addItem("配置管理");
    navList_->addItem("日志查看");

    // 右侧页面栈
    pageStack_ = new QStackedWidget;
    pageStack_->addWidget(modulePage_ = new ModulePage);
    pageStack_->addWidget(devicePage_ = new DevicePage);
    pageStack_->addWidget(appPage_ = new AppPage);
    pageStack_->addWidget(containerPage_ = new ContainerPage);
    pageStack_->addWidget(configPage_ = new ConfigPage);
    pageStack_->addWidget(logPage_ = new LogPage);

    mainLayout->addWidget(navList_);
    mainLayout->addWidget(pageStack_, 1);

    connect(navList_, &QListWidget::currentRowChanged,
            pageStack_, &QStackedWidget::setCurrentIndex);

    setupStatusBar();
    setupTray();

    navList_->setCurrentRow(0);
}
```

#### 8.2.2 页面组件

**任务 M5.2 - M5.7: 实现各功能页面**

每个页面遵循相同模式：

```cpp
// src/gui/pages/DevicePage.h

#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>

namespace wekey {

class DevicePage : public QWidget {
    Q_OBJECT

public:
    explicit DevicePage(QWidget* parent = nullptr);

public slots:
    void refresh();

private slots:
    void onRefreshClicked();
    void onDeviceSelected(int row);
    void onSetLabelClicked();
    void onChangeAuthClicked();

private:
    void setupUi();
    void updateDeviceList(const QList<DeviceInfo>& devices);

    QTableWidget* table_ = nullptr;
    QPushButton* refreshBtn_ = nullptr;
    QPushButton* setLabelBtn_ = nullptr;
    QPushButton* changeAuthBtn_ = nullptr;

    // 详情区
    QLabel* manufacturerLabel_ = nullptr;
    QLabel* hwVersionLabel_ = nullptr;
    QLabel* fwVersionLabel_ = nullptr;
};

} // namespace wekey
```

#### 8.2.3 对话框

**任务 M5.8: 实现登录对话框**

```cpp
// src/gui/dialogs/LoginDialog.h

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QLabel>

namespace wekey {

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(const QString& appName, QWidget* parent = nullptr);

    QString pin() const;
    QString role() const;

    void setRetryCount(int count);

private slots:
    void onLoginClicked();

private:
    void setupUi();

    QString appName_;
    QLineEdit* pinEdit_ = nullptr;
    QRadioButton* userRadio_ = nullptr;
    QRadioButton* adminRadio_ = nullptr;
    QLabel* retryLabel_ = nullptr;
};

} // namespace wekey
```

#### 8.2.4 SystemTray

**任务 M5.9: 实现系统托盘**

```cpp
// src/gui/SystemTray.h

#pragma once

#include <QSystemTrayIcon>
#include <QMenu>

namespace wekey {

class SystemTray : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit SystemTray(QWidget* parent = nullptr);

signals:
    void showWindowRequested();
    void exitRequested();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void setupMenu();

    QMenu* menu_ = nullptr;
};

} // namespace wekey
```

### 8.3 验收标准 (M5)

- [ ] 主窗口布局正确（左导航 + 右内容）
- [ ] 6 个页面功能正常
- [ ] 对话框交互流畅
- [ ] 关闭窗口最小化到托盘
- [ ] 托盘菜单功能正常
- [ ] 错误提示支持简洁/详细模式切换
- [ ] 日志页面实时更新、支持搜索过滤

---

## 9. M6: 测试与打包

### 9.1 目标

完成集成测试，制作可分发的安装包。

### 9.2 任务分解

#### 9.2.1 集成测试

**任务 M6.1: 端到端测试**

```cpp
// tests/integration/test_e2e.cpp

class TestE2E : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // 启动应用
    }

    void cleanupTestCase() {
        // 关闭应用
    }

    void testFullWorkflow() {
        // 1. 枚举设备
        // 2. 登录应用
        // 3. 创建容器
        // 4. 生成 CSR
        // 5. 签名
        // 6. 登出
    }

    void testHotPlug() {
        // 模拟设备插拔
    }

    void testPinLock() {
        // 测试 PIN 锁定和解锁
    }

    void testApiCompatibility() {
        // 调用所有 API 端点
    }
};
```

#### 9.2.2 内存检测

**任务 M6.2: AddressSanitizer 集成**

```cmake
# CMakeLists.txt
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
```

#### 9.2.3 Windows 打包

**任务 M6.3: Windows 绿色版打包脚本**

```powershell
# scripts/package_win.ps1

$BuildDir = "build/release"
$OutputDir = "dist/wekey-skf-win64"

# 清理
Remove-Item -Recurse -Force $OutputDir -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $OutputDir

# 复制主程序
Copy-Item "$BuildDir/wekey-skf.exe" $OutputDir

# 复制 Qt 依赖
windeployqt --no-translations --no-opengl-sw "$OutputDir/wekey-skf.exe"

# 复制驱动插件
New-Item -ItemType Directory -Path "$OutputDir/plugins"
Copy-Item "$BuildDir/plugins/skf.dll" "$OutputDir/plugins/"

# 复制厂商库
New-Item -ItemType Directory -Path "$OutputDir/lib"
Copy-Item "resources/lib/win/libskf.dll" "$OutputDir/lib/"

# 创建 ZIP
Compress-Archive -Path $OutputDir -DestinationPath "dist/wekey-skf-win64.zip"
```

#### 9.2.4 macOS 打包

**任务 M6.4: macOS DMG 打包脚本**

```bash
#!/bin/bash
# scripts/package_mac.sh

BUILD_DIR="build/release"
APP_NAME="wekey-skf"
OUTPUT_DIR="dist"

# 清理
rm -rf "$OUTPUT_DIR/$APP_NAME.app"

# 复制 app bundle
cp -R "$BUILD_DIR/$APP_NAME.app" "$OUTPUT_DIR/"

# 部署 Qt 框架
macdeployqt "$OUTPUT_DIR/$APP_NAME.app" -always-overwrite

# 复制厂商库
mkdir -p "$OUTPUT_DIR/$APP_NAME.app/Contents/Resources/lib"
cp resources/lib/mac/libskf.dylib "$OUTPUT_DIR/$APP_NAME.app/Contents/Resources/lib/"

# 代码签名 (需要开发者证书)
# codesign --force --deep --sign "Developer ID Application: ..." "$OUTPUT_DIR/$APP_NAME.app"

# 创建 DMG
hdiutil create -volname "$APP_NAME" -srcfolder "$OUTPUT_DIR/$APP_NAME.app" \
    -ov -format UDZO "$OUTPUT_DIR/$APP_NAME.dmg"
```

### 9.3 验收标准 (M6)

- [ ] 所有单元测试通过
- [ ] 集成测试通过
- [ ] AddressSanitizer 无内存错误
- [ ] Windows 绿色版可运行
- [ ] macOS DMG 安装后可运行
- [ ] 配置文件兼容原 Go 版本
- [ ] HTTP API 通过兼容性测试

---

## 10. 风险评估与缓解

### 10.1 技术风险

| 风险 | 影响 | 可能性 | 缓解措施 |
|------|------|--------|----------|
| SKF 厂商库兼容性 | 高 | 中 | 提前获取多厂商库进行测试 |
| Qt 版本差异 | 中 | 低 | 锁定 Qt 6.5 LTS 版本 |
| 跨平台编译问题 | 中 | 中 | 使用 CI/CD 持续验证双平台 |
| 内存泄漏 | 高 | 中 | 全程使用 RAII，定期 ASAN 检测 |

### 10.2 进度风险

| 风险 | 影响 | 可能性 | 缓解措施 |
|------|------|--------|----------|
| SKF 驱动实现复杂 | 高 | 高 | M2 预留缓冲时间 |
| API 兼容性问题 | 中 | 中 | 尽早开始兼容性测试 |
| GUI 细节调整多 | 低 | 高 | GUI 功能可渐进式完善 |

### 10.3 依赖风险

| 依赖 | 风险 | 缓解措施 |
|------|------|----------|
| 厂商 SKF 库 | 可能有 bug 或文档不全 | 建立厂商沟通渠道 |
| Qt 6.x | 框架可能有 bug | 使用 LTS 版本，关注更新 |

---

## 11. 附录

### 11.1 编码规范

- 命名：类名 `PascalCase`，方法 `camelCase`，成员变量 `name_`
- 缩进：4 空格
- 行宽：100 字符
- 注释：公共 API 必须有 Doxygen 注释

### 11.2 Git 工作流

- 主分支：`main`
- 开发分支：`dev`
- 功能分支：`feature/m1-xxx`
- 修复分支：`fix/xxx`

### 11.3 CI/CD 配置

```yaml
# .github/workflows/build.yml
name: Build

on: [push, pull_request]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Qt
        uses: jurplel/install-qt-action@v3
        with:
          version: '6.5.*'
      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build --config Release
      - name: Test
        run: ctest --test-dir build -C Release

  build-macos:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Qt
        uses: jurplel/install-qt-action@v3
        with:
          version: '6.5.*'
      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build
      - name: Test
        run: ctest --test-dir build
```

---

## 文档结束

**下一步行动**：确认本方案后，从 M1 开始实施，首先创建 CMake 构建系统和基础模块。
