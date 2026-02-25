# wekey-skf API Sketch

> **版本**: 1.0.0
> **日期**: 2026-02-06
> **用途**: 开发参考，描述各模块对外暴露的主要接口

---

## 目录结构

```
src/
├── app/                    # 应用入口
│   ├── main.cpp           # main() 函数
│   └── Application.h/cpp  # QApplication 子类，单实例控制
│
├── core/                   # 核心业务逻辑 (纯 C++，不依赖 Qt GUI)
│   ├── device/            # 设备管理
│   │   ├── DeviceManager.h/cpp
│   │   └── DeviceInfo.h
│   ├── application/       # 应用管理
│   │   ├── AppManager.h/cpp
│   │   └── AppInfo.h
│   ├── container/         # 容器管理
│   │   ├── ContainerManager.h/cpp
│   │   └── ContainerInfo.h
│   ├── crypto/            # 证书与签名
│   │   ├── CertManager.h/cpp
│   │   ├── CertInfo.h
│   │   └── CsrBuilder.h/cpp
│   └── file/              # 文件操作
│       └── FileManager.h/cpp
│
├── api/                    # HTTP REST API
│   ├── HttpServer.h/cpp   # 服务器主类
│   ├── ApiRouter.h/cpp    # 路由分发
│   ├── handlers/          # 请求处理器
│   │   ├── PublicHandlers.h/cpp
│   │   ├── BusinessHandlers.h/cpp
│   │   └── AdminHandlers.h/cpp
│   └── dto/               # 数据传输对象
│       ├── Request.h
│       └── Response.h
│
├── gui/                    # Qt Widgets GUI
│   ├── MainWindow.h/cpp   # 主窗口
│   ├── pages/             # 页面组件
│   │   ├── ModulePage.h/cpp
│   │   ├── DevicePage.h/cpp
│   │   ├── AppPage.h/cpp
│   │   ├── ContainerPage.h/cpp
│   │   ├── ConfigPage.h/cpp
│   │   └── LogPage.h/cpp
│   ├── widgets/           # 可复用控件
│   │   ├── DeviceListWidget.h/cpp
│   │   └── CertInfoWidget.h/cpp
│   ├── dialogs/           # 对话框
│   │   ├── LoginDialog.h/cpp
│   │   ├── CsrDialog.h/cpp
│   │   └── ImportCertDialog.h/cpp
│   └── SystemTray.h/cpp   # 系统托盘
│
├── plugin/                 # 插件系统
│   ├── interface/         # 插件接口定义
│   │   ├── IDriverPlugin.h
│   │   └── PluginTypes.h
│   ├── PluginManager.h/cpp
│   └── skf/               # SKF 插件实现 (可独立编译为 .dll/.dylib)
│       ├── SkfPlugin.h/cpp
│       ├── SkfApi.h       # C API 声明
│       └── SkfTypes.h     # SKF 数据结构
│
├── config/                 # 配置管理
│   ├── Config.h/cpp       # 配置读写
│   └── Defaults.h         # 默认值常量
│
├── log/                    # 日志系统
│   ├── Logger.h/cpp       # 日志核心
│   └── LogModel.h/cpp     # Qt Model (供 GUI 使用)
│
└── common/                 # 公共工具
    ├── Error.h            # 错误码定义
    ├── Result.h           # Result<T, E> 模板
    └── Utils.h/cpp        # 工具函数

tests/
├── unit/                   # 单元测试
│   ├── test_config.cpp
│   ├── test_csrbuilder.cpp
│   └── ...
└── integration/            # 集成测试
    └── test_skf_plugin.cpp

resources/
├── icons/                  # 图标资源
├── lib/
│   ├── win/               # Windows 内嵌库
│   │   └── libskf.dll
│   └── mac/               # macOS 内嵌库
│       └── libskf.dylib
└── wekey-skf.qrc          # Qt 资源文件
```

---

## 核心接口定义

### 1. 插件接口 (IDriverPlugin)

```cpp
// src/plugin/interface/IDriverPlugin.h

#pragma once

#include <QtPlugin>
#include <QList>
#include <QVariantMap>
#include "PluginTypes.h"

namespace wekey {

/**
 * @brief 驱动插件接口
 *
 * 所有驱动（SKF、P11、Longmai 等）必须实现此接口。
 * 使用 Qt 插件机制动态加载。
 */
class IDriverPlugin {
public:
    virtual ~IDriverPlugin() = default;

    //=== 设备管理 ===

    /**
     * @brief 枚举已连接设备
     * @param login 是否返回登录状态
     * @return 设备信息列表
     */
    virtual Result<QList<DeviceInfo>> enumDevices(bool login = false) = 0;

    /**
     * @brief 修改设备认证密钥
     */
    virtual Result<void> changeDeviceAuth(const QString& devName,
                                          const QString& oldPin,
                                          const QString& newPin) = 0;

    /**
     * @brief 设置设备标签
     */
    virtual Result<void> setDeviceLabel(const QString& devName,
                                        const QString& label) = 0;

    /**
     * @brief 等待设备插拔事件 (阻塞)
     * @return 事件类型: 1=插入, 2=拔出
     */
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

    /**
     * @brief 获取 PIN 剩余尝试次数
     * @param role "user" 或 "admin"
     */
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

    /**
     * @brief 生成 CSR
     * @param args 包含: keyType, commonName, organization, unit, renew
     * @return PEM 格式的 CSR
     */
    virtual Result<QByteArray> generateCsr(const QString& devName,
                                           const QString& appName,
                                           const QString& containerName,
                                           const QVariantMap& args) = 0;

    /**
     * @brief 导入证书
     * @param args 包含: cert (PEM), certChain (可选)
     */
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

    virtual Result<QByteArray> generateRandom(const QString& devName,
                                              int count) = 0;
};

} // namespace wekey

#define IDriverPlugin_iid "com.trustasia.wekey.IDriverPlugin/1.0"
Q_DECLARE_INTERFACE(wekey::IDriverPlugin, IDriverPlugin_iid)
```

---

### 2. 数据类型 (PluginTypes)

```cpp
// src/plugin/interface/PluginTypes.h

#pragma once

#include <QString>
#include <QDateTime>

namespace wekey {

struct DeviceInfo {
    QString deviceName;      // 设备名称（唯一标识）
    QString devicePath;      // 设备路径
    QString manufacturer;    // 制造商
    QString label;           // 用户标签
    QString serialNumber;    // 序列号
    QString hardwareVersion; // 硬件版本
    QString firmwareVersion; // 固件版本
    bool isLoggedIn = false; // 是否已登录
};

struct AppInfo {
    QString appName;         // 应用名称
    bool isLoggedIn = false; // 是否已登录
};

struct ContainerInfo {
    QString containerName;   // 容器名称
    bool keyGenerated = false;  // 是否已生成密钥
    int keyType = 0;         // 密钥类型 (0=未知, 1=RSA, 2=SM2)
    bool certImported = false;  // 是否已导入证书
};

struct CertInfo {
    QString serialNumber;    // 证书序列号
    QString subject;         // 主题 DN
    QString commonName;      // 通用名
    QString issuer;          // 颁发者 DN
    QDateTime notBefore;     // 生效时间
    QDateTime notAfter;      // 过期时间
    int certType = 0;        // 证书类型 (1=签名, 2=加密)
    QString certPem;         // PEM 格式证书
    QString publicKeyHash;   // 公钥哈希
};

// 密钥类型枚举
enum class KeyType {
    Unknown = 0,
    RSA_2048 = 1,
    RSA_3072 = 2,
    RSA_4096 = 3,
    SM2 = 4
};

} // namespace wekey
```

---

### 3. 错误处理 (Result 模板)

```cpp
// src/common/Result.h

#pragma once

#include <variant>
#include <QString>
#include "Error.h"

namespace wekey {

/**
 * @brief 操作结果类型
 *
 * 遵循项目宪法 §3.1：所有错误都必须被显式处理。
 * 使用 Result<T> 替代异常，强制调用方处理错误。
 *
 * 用法:
 *   Result<DeviceInfo> result = enumDevices();
 *   if (result.isOk()) {
 *       auto info = result.value();
 *   } else {
 *       auto err = result.error();
 *       // 处理错误
 *   }
 */
template<typename T>
class Result {
public:
    // 成功构造
    static Result ok(T value) {
        return Result(std::move(value));
    }

    // 失败构造
    static Result err(Error error) {
        return Result(std::move(error));
    }

    bool isOk() const { return std::holds_alternative<T>(data_); }
    bool isErr() const { return !isOk(); }

    const T& value() const { return std::get<T>(data_); }
    T& value() { return std::get<T>(data_); }

    const Error& error() const { return std::get<Error>(data_); }

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

    bool isOk() const { return success_; }
    bool isErr() const { return !success_; }

    const Error& error() const { return error_; }

private:
    explicit Result(bool) : success_(true) {}
    explicit Result(Error error) : success_(false), error_(std::move(error)) {}

    bool success_ = false;
    Error error_;
};

} // namespace wekey
```

---

### 4. 错误码定义 (Error)

```cpp
// src/common/Error.h

#pragma once

#include <QString>
#include <cstdint>

namespace wekey {

/**
 * @brief 错误码定义
 *
 * 遵循项目宪法 §3.1：错误必须携带上下文信息。
 */
class Error {
public:
    // 应用层错误码
    enum Code : uint32_t {
        Success         = 0x00,
        Fail            = 0x01,
        InvalidParam    = 0x03,
        NoActiveModule  = 0x04,
        NotLoggedIn     = 0x09,
        NotAuthorized   = 0x0B,

        // SKF 错误码 (直接映射)
        SkfOk           = 0x00000000,
        SkfFail         = 0x0A000001,
        SkfUnknown      = 0x0A000002,
        SkfNotSupported = 0x0A000003,
        SkfFileError    = 0x0A000004,
        SkfInvalidHandle= 0x0A000005,
        SkfInvalidParam = 0x0A000006,
        SkfDeviceRemoved= 0x0A000023,
        SkfPinIncorrect = 0x0A000024,
        SkfPinLocked    = 0x0A000025,
        SkfUserNotLogin = 0x0A00002D,
        SkfAppNotExists = 0x0A00002E,
    };

    Error() = default;
    Error(Code code, const QString& message = {}, const QString& context = {})
        : code_(code), message_(message), context_(context) {}

    Code code() const { return code_; }
    const QString& message() const { return message_; }
    const QString& context() const { return context_; }

    /**
     * @brief 获取用户友好的错误描述
     * @param detailed 是否包含详细信息
     */
    QString toString(bool detailed = false) const;

    /**
     * @brief 从 SKF 返回值构造错误
     */
    static Error fromSkf(uint32_t skfResult, const QString& function = {});

private:
    Code code_ = Success;
    QString message_;
    QString context_;  // 如: 函数名、设备名
};

} // namespace wekey
```

---

### 5. 配置管理 (Config)

```cpp
// src/config/Config.h

#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace wekey {

/**
 * @brief 配置管理类
 *
 * 配置文件: ~/.wekeytool.json (兼容原 Go 版本)
 */
class Config : public QObject {
    Q_OBJECT

public:
    static Config& instance();

    // 加载/保存
    bool load();
    bool save();

    // 基本配置
    QString listenPort() const;
    void setListenPort(const QString& port);

    QString logLevel() const;
    void setLogLevel(const QString& level);

    QString logPath() const;
    void setLogPath(const QString& path);

    bool systrayDisabled() const;
    void setSystrayDisabled(bool disabled);

    QString errorMode() const;  // "simple" | "detailed"
    void setErrorMode(const QString& mode);

    // 模块管理
    QVariantMap modPaths() const;
    void setModPath(const QString& name, const QString& path);
    void removeModPath(const QString& name);

    QString activedModName() const;
    void setActivedModName(const QString& name);

    // 默认值
    QString defaultAppName() const;
    QString defaultContainerName() const;
    QString defaultCommonName() const;
    QString defaultOrganization() const;
    QString defaultUnit() const;
    QString defaultRole() const;

    void setDefault(const QString& key, const QString& value);

signals:
    void configChanged();

private:
    Config() = default;

    QString configPath() const;

    QVariantMap data_;
};

} // namespace wekey
```

---

### 6. 日志系统 (Logger)

```cpp
// src/log/Logger.h

#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>

namespace wekey {

enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error
};

struct LogEntry {
    QDateTime timestamp;
    LogLevel level;
    QString message;
    QString source;  // 来源模块
};

/**
 * @brief 日志管理类
 *
 * 支持:
 * - 文件输出
 * - GUI 实时显示 (通过信号)
 */
class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& instance();

    void setLevel(LogLevel level);
    void setOutputPath(const QString& path);

    void debug(const QString& message, const QString& source = {});
    void info(const QString& message, const QString& source = {});
    void warn(const QString& message, const QString& source = {});
    void error(const QString& message, const QString& source = {});

signals:
    /**
     * @brief 新日志条目信号 (供 GUI 使用)
     */
    void logAdded(const LogEntry& entry);

private:
    Logger() = default;
    void log(LogLevel level, const QString& message, const QString& source);

    LogLevel level_ = LogLevel::Info;
    QString outputPath_;
};

// 便捷宏
#define LOG_DEBUG(msg) wekey::Logger::instance().debug(msg, __FUNCTION__)
#define LOG_INFO(msg)  wekey::Logger::instance().info(msg, __FUNCTION__)
#define LOG_WARN(msg)  wekey::Logger::instance().warn(msg, __FUNCTION__)
#define LOG_ERROR(msg) wekey::Logger::instance().error(msg, __FUNCTION__)

} // namespace wekey
```

---

### 7. 插件管理器 (PluginManager)

```cpp
// src/plugin/PluginManager.h

#pragma once

#include <QObject>
#include <QMap>
#include <memory>
#include "interface/IDriverPlugin.h"

namespace wekey {

/**
 * @brief 插件管理器
 *
 * 使用 QPluginLoader 动态加载驱动插件。
 */
class PluginManager : public QObject {
    Q_OBJECT

public:
    static PluginManager& instance();

    /**
     * @brief 注册插件
     * @param name 插件名称
     * @param path 插件文件路径 (.dll/.dylib)
     */
    Result<void> registerPlugin(const QString& name, const QString& path);

    /**
     * @brief 卸载插件
     */
    Result<void> unregisterPlugin(const QString& name);

    /**
     * @brief 获取插件
     */
    IDriverPlugin* getPlugin(const QString& name);

    /**
     * @brief 获取当前激活的插件
     */
    IDriverPlugin* activePlugin();

    /**
     * @brief 设置激活的插件
     */
    Result<void> setActivePlugin(const QString& name);

    /**
     * @brief 列出所有已注册插件
     */
    QStringList listPlugins() const;

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

---

### 8. HTTP 服务器 (HttpServer)

```cpp
// src/api/HttpServer.h

#pragma once

#include <QObject>
#include <QTcpServer>
#include <memory>

namespace wekey {

class ApiRouter;

/**
 * @brief HTTP REST API 服务器
 *
 * 监听端口: 默认 9001
 * 兼容原 Go 版本 API。
 */
class HttpServer : public QObject {
    Q_OBJECT

public:
    explicit HttpServer(QObject* parent = nullptr);
    ~HttpServer();

    /**
     * @brief 启动服务器
     * @param port 监听端口，如 ":9001"
     */
    Result<void> start(const QString& port);

    /**
     * @brief 停止服务器
     */
    void stop();

    bool isRunning() const;

signals:
    void started(quint16 port);
    void stopped();
    void requestReceived(const QString& method, const QString& path);
    void errorOccurred(const QString& error);

private:
    std::unique_ptr<QTcpServer> server_;
    std::unique_ptr<ApiRouter> router_;
};

} // namespace wekey
```

---

### 9. 主窗口 (MainWindow)

```cpp
// src/gui/MainWindow.h

#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>

namespace wekey {

class ModulePage;
class DevicePage;
class AppPage;
class ContainerPage;
class ConfigPage;
class LogPage;
class SystemTray;

/**
 * @brief 主窗口
 *
 * 布局: 左侧导航 + 右侧内容区
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onNavigationChanged(int index);
    void onTrayActivated();

private:
    void setupUi();
    void setupNavigation();
    void setupPages();
    void setupStatusBar();
    void setupTray();

    // 导航
    QListWidget* navList_ = nullptr;
    QStackedWidget* pageStack_ = nullptr;

    // 页面
    ModulePage* modulePage_ = nullptr;
    DevicePage* devicePage_ = nullptr;
    AppPage* appPage_ = nullptr;
    ContainerPage* containerPage_ = nullptr;
    ConfigPage* configPage_ = nullptr;
    LogPage* logPage_ = nullptr;

    // 系统托盘
    SystemTray* tray_ = nullptr;
};

} // namespace wekey
```

---

## 模块依赖关系

```
                    ┌─────────────┐
                    │    app/     │
                    │  (入口)     │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           ▼               ▼               ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │    gui/     │ │    api/     │ │   plugin/   │
    │  (界面)     │ │ (HTTP API)  │ │  (驱动)     │
    └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
           │               │               │
           └───────────────┼───────────────┘
                           ▼
                    ┌─────────────┐
                    │    core/    │
                    │ (业务逻辑)  │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           ▼               ▼               ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │   config/   │ │    log/     │ │   common/   │
    └─────────────┘ └─────────────┘ └─────────────┘
```

**依赖规则:**
1. `app/` 可依赖所有模块
2. `gui/` 和 `api/` 依赖 `core/`，不互相依赖
3. `core/` 依赖 `plugin/interface/`（仅接口）
4. `plugin/skf/` 实现 `plugin/interface/`
5. `common/`、`config/`、`log/` 是基础模块，无外部依赖

---

## 构建系统

```makefile
# Makefile (简化示例)

.PHONY: all build run test clean

# Qt 路径 (根据平台调整)
QMAKE := qmake6

all: build

build:
	mkdir -p build && cd build && $(QMAKE) ../wekey-skf.pro && make

run: build
	./build/wekey-skf

test:
	cd build && make check

clean:
	rm -rf build
```

---

## 文档结束
