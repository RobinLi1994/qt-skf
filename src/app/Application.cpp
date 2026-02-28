/**
 * @file Application.cpp
 * @brief 应用程序主类实现
 */

#include "app/Application.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QMessageLogger>
#include <QStandardPaths>

#include "config/Config.h"
#include "log/Logger.h"
#include "plugin/PluginManager.h"

namespace wekey {

Application::Application(int& argc, char** argv) : QApplication(argc, argv) {
    // 设置应用程序信息
    setApplicationName("wekey-skf");
    setApplicationVersion("1.0.0");
    setOrganizationName("TrustAsia");
    setOrganizationDomain("trustasia.com");
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    // 1. 获取单例锁
    if (!acquireSingleInstanceLock()) {
        emit secondInstanceStarted();
        // 仍然继续初始化，但标记为非主实例
    }

    // 2. 加载配置
    if (!loadConfig()) {
        LOG_ERROR("加载配置失败");
        return false;
    }

    // 3. 初始化日志
    initLogging();

    // 4. 从配置恢复插件
    loadPlugins();

    LOG_INFO("应用程序初始化完成");
    return true;
}

void Application::shutdown() {
    LOG_INFO("应用程序关闭");

    // 释放锁文件
    if (lockFile_ && lockFile_->isLocked()) {
        lockFile_->unlock();
    }
}

bool Application::isPrimaryInstance() const {
    return isPrimary_;
}

bool Application::acquireSingleInstanceLock() {
    // 使用 QLockFile 实现单例检测
    QString lockPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    lockPath += "/wekey-skf.lock";

    lockFile_ = std::make_unique<QLockFile>(lockPath);
    lockFile_->setStaleLockTime(0);  // 不自动删除旧锁

    if (lockFile_->tryLock(100)) {
        isPrimary_ = true;
        return true;
    }

    // 检查锁是否是由已死进程持有
    qint64 pid;
    QString hostname;
    QString appname;

    if (lockFile_->getLockInfo(&pid, &hostname, &appname)) {
        // 检查进程是否仍在运行
        // 这里简化处理，假设锁有效
        isPrimary_ = false;
        return false;
    }

    // 锁文件损坏或过期，强制删除并重试
    lockFile_->removeStaleLockFile();
    if (lockFile_->tryLock(100)) {
        isPrimary_ = true;
        return true;
    }

    isPrimary_ = false;
    return false;
}

bool Application::loadConfig() {
    Config& config = Config::instance();
    return config.load();
}

// Qt 消息处理器：将 qDebug/qInfo/qWarning/qCritical 转发到 Logger 单例
// 从 ctx 提取文件名和行号追加到消息末尾，方便定位代码位置
static void qtMessageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    // 提取文件名（去掉路径，只保留 basename）和行号
    QString location;
    if (ctx.file && ctx.line > 0) {
        QString file = QString::fromUtf8(ctx.file);
        int slash = file.lastIndexOf('/');
        if (slash < 0) slash = file.lastIndexOf('\\');
        file = (slash >= 0) ? file.mid(slash + 1) : file;
        location = QString(" (%1:%2)").arg(file).arg(ctx.line);
    }
    QString fullMsg = msg + location;

    Logger& logger = Logger::instance();
    switch (type) {
        case QtDebugMsg:    logger.debug(fullMsg); break;
        case QtInfoMsg:     logger.info(fullMsg);  break;
        case QtWarningMsg:  logger.warn(fullMsg);  break;
        case QtCriticalMsg: logger.error(fullMsg); break;
        case QtFatalMsg:    logger.error(fullMsg); break;
    }
}

void Application::initLogging() {
    Config& config = Config::instance();
    Logger& logger = Logger::instance();

    // 设置日志级别
    LogLevel level = stringToLogLevel(config.logLevel());
    logger.setLevel(level);

    // 设置日志输出路径
    QString logPath = config.logPath();
    if (!logPath.isEmpty()) {
        QDir dir(logPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QString logFile = logPath + "/wekey-skf.log";
        logger.setOutputPath(logFile);
    }

    // 将所有 qDebug/qInfo/qWarning/qCritical 转发到 Logger 单例
    qInstallMessageHandler(qtMessageHandler);

    LOG_INFO(QString("日志系统初始化，级别: %1").arg(config.logLevel()));
}

void Application::loadPlugins() {
    Config& config = Config::instance();
    auto& pm = PluginManager::instance();

    QJsonObject paths = config.modPaths();

    // 策略：用户配置的模块优先，无配置时用内置模块兜底
    if (paths.isEmpty()) {
        LOG_INFO("用户未配置任何模块，尝试注册内置 SKF 库");
        QString builtinPath = registerBuiltinModule();
        if (!builtinPath.isEmpty()) {
            paths = config.modPaths();
        }
    } else {
        LOG_INFO(QString("从配置加载 %1 个用户模块").arg(paths.size()));
    }

    for (auto it = paths.begin(); it != paths.end(); ++it) {
        QString name = it.key();
        QString path = it.value().toString();
        auto result = pm.registerPlugin(name, path);
        if (result.isOk()) {
            LOG_INFO(QString("已加载模块: %1 (%2)").arg(name, path));
        } else {
            LOG_ERROR(QString("加载模块失败: %1 (%2)").arg(name, result.error().message()));
        }
    }

    QString activeName = config.activedModName();
    if (!activeName.isEmpty() && pm.listPlugins().contains(activeName)) {
        pm.setActivePlugin(activeName);
        LOG_INFO(QString("已激活模块: %1").arg(activeName));
    }
}

QString Application::registerBuiltinModule() {
    QString libPath = findBuiltinLibPath();
    if (libPath.isEmpty()) {
        LOG_INFO("未找到内置 SKF 库，跳过内置模块注册");
        return {};
    }

    Config& config = Config::instance();
    config.setModPath("gm3000", libPath);
    config.setActivedModName("gm3000");
    config.save();

    LOG_INFO(QString("已注册内置模块: gm3000 (%1)").arg(libPath));
    return libPath;
}

QString Application::findBuiltinLibPath() const {
    QString appDir = QCoreApplication::applicationDirPath();

#ifdef Q_OS_MACOS
    QString libName = "libgm3000.dylib";
#elif defined(Q_OS_WIN)
    QString libName = "mtoken_gm3000.dll";
#else
    QString libName = "libgm3000.so";
#endif

    // 按优先级搜索内置 SKF 库位置
    QStringList searchPaths;

#ifdef Q_OS_MACOS
    // macOS 打包环境: wekey-skf.app/Contents/MacOS/../Frameworks/
    searchPaths << appDir + "/../Frameworks/" + libName;
#endif

#ifdef Q_OS_WIN
    // Windows 打包环境: exe 同目录
    searchPaths << appDir + "/" + libName;
#endif

    // 开发环境: build/lib/ (CMake POST_BUILD 拷贝的位置)
    // macOS .app bundle: appDir = build/src/app/wekey-skf.app/Contents/MacOS
    //   -> ../../../../../lib = build/lib
    // Linux/Windows 非 bundle: appDir = build/src/app
    //   -> ../../lib = build/lib
    searchPaths << appDir + "/../../../../../lib/" + libName;
    searchPaths << appDir + "/../../lib/" + libName;

    for (const auto& candidate : searchPaths) {
        QString absPath = QDir(candidate).absolutePath();
        // QDir::absolutePath 会规范化路径中的 .. 等
        QFileInfo fi(absPath);
        if (fi.exists() && fi.isFile()) {
            LOG_INFO(QString("找到内置 SKF 库: %1").arg(absPath));
            return absPath;
        }
        LOG_DEBUG(QString("内置 SKF 库候选路径不存在: %1").arg(absPath));
    }

    return {};
}

}  // namespace wekey
