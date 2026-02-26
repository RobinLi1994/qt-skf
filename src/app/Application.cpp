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

// 进程存活检测：kill(pid, 0) 用于检查进程是否存在
#ifdef Q_OS_UNIX
#include <signal.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif

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

    LOG_INFO(QString("单例锁路径: %1").arg(lockPath));

    lockFile_ = std::make_unique<QLockFile>(lockPath);
    // 设置 5 秒过期时间，让 Qt 能自动检测并清理已死进程的残留锁文件
    // （之前设置为 0 表示永不过期，导致 Rosetta 2 下异常退出后无法重新启动）
    lockFile_->setStaleLockTime(5000);

    if (lockFile_->tryLock(100)) {
        isPrimary_ = true;
        LOG_INFO("成功获取单例锁");
        return true;
    }

    // 检查锁是否是由已死进程持有
    qint64 pid = 0;
    QString hostname;
    QString appname;

    if (lockFile_->getLockInfo(&pid, &hostname, &appname)) {
        LOG_INFO(QString("锁被进程 %1 (%2) 持有，检查进程是否存活").arg(pid).arg(appname));

        // 检查持有锁的进程是否真的还在运行
        bool processAlive = false;
#ifdef Q_OS_UNIX
        // Unix: kill(pid, 0) 返回 0 表示进程存在
        processAlive = (kill(static_cast<pid_t>(pid), 0) == 0);
#elif defined(Q_OS_WIN)
        // Windows: OpenProcess 尝试打开进程句柄
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid));
        if (hProcess != NULL) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE) {
                processAlive = true;
            }
            CloseHandle(hProcess);
        }
#endif

        if (!processAlive) {
            // 持有锁的进程已死亡（如 Rosetta 2 下异常退出），强制清理
            LOG_INFO(QString("进程 %1 已不存在，清理残留锁文件").arg(pid));
            lockFile_->removeStaleLockFile();
            if (lockFile_->tryLock(100)) {
                isPrimary_ = true;
                LOG_INFO("清理残留锁后成功获取单例锁");
                return true;
            }
        } else {
            LOG_INFO(QString("进程 %1 仍在运行，当前为第二实例").arg(pid));
            isPrimary_ = false;
            return false;
        }
    }

    // 锁文件损坏或过期，强制删除并重试
    LOG_INFO("锁文件信息不可读，尝试强制清理");
    lockFile_->removeStaleLockFile();
    if (lockFile_->tryLock(100)) {
        isPrimary_ = true;
        LOG_INFO("强制清理后成功获取单例锁");
        return true;
    }

    LOG_ERROR("无法获取单例锁");
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

    // 如果没有配置任何模块，自动注册内置 SKF 库
    if (paths.isEmpty()) {
        QString builtinPath = registerBuiltinModule();
        if (!builtinPath.isEmpty()) {
            paths = config.modPaths();
        }
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
    QString appDir = QCoreApplication::applicationDirPath();
    // 从 build/src/app 到 build/lib，需要 ../../lib
    QString libDir = appDir + "/../../lib";

#ifdef Q_OS_MACOS
    QString libName = "libgm3000.dylib";
#elif defined(Q_OS_WIN)
    QString libName = "mtoken_gm3000.dll";
#else
    QString libName = "libgm3000.so";
#endif

    QString libPath = QDir(libDir).absoluteFilePath(libName);
    if (!QFile::exists(libPath)) {
        LOG_INFO(QString("未找到内置 SKF 库: %1").arg(libPath));
        return {};
    }

    Config& config = Config::instance();
    config.setModPath("gm3000", libPath);
    config.setActivedModName("gm3000");
    config.save();

    LOG_INFO(QString("已注册内置模块: gm3000 (%1)").arg(libPath));
    return libPath;
}

}  // namespace wekey
