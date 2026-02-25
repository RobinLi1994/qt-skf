/**
 * @file Logger.cpp
 * @brief 日志记录器实现
 */

#include "log/Logger.h"

#include <QDir>
#include <QMutexLocker>
#include <QTextStream>
#include <cstdio>

namespace wekey {

QString logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
    }
    return "INFO";
}

LogLevel stringToLogLevel(const QString& str) {
    QString lower = str.toLower();
    if (lower == "debug") {
        return LogLevel::Debug;
    } else if (lower == "info") {
        return LogLevel::Info;
    } else if (lower == "warn" || lower == "warning") {
        return LogLevel::Warn;
    } else if (lower == "error") {
        return LogLevel::Error;
    }
    return LogLevel::Info;
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : QObject(nullptr), level_(LogLevel::Debug) {
    qRegisterMetaType<LogEntry>("LogEntry");
}

Logger::~Logger() {
    if (file_.isOpen()) {
        file_.close();
    }
}

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

LogLevel Logger::level() const {
    return level_;
}

void Logger::setOutputPath(const QString& path) {
    QMutexLocker locker(&mutex_);

    if (file_.isOpen()) {
        file_.close();
    }

    if (path.isEmpty()) {
        return;
    }

    // 确保目录存在
    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    file_.setFileName(path);
    if (!file_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning("Logger: Failed to open log file: %s", qPrintable(path));
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

    // 递归保护：避免 qtMessageHandler → Logger::log → qDebug → qtMessageHandler 死循环
    thread_local bool inLog = false;
    if (inLog) return;
    inLog = true;

    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.message = message;
    entry.source = source;

    // 直接用 fprintf(stderr) 输出到控制台，避免调用 qDebug/qWarning 触发消息处理器
    // 格式：时间戳 [级别] 消息，ANSI 颜色区分级别
    static const char* kReset   = "\033[0m";
    static const char* kGray    = "\033[90m";   // Debug
    static const char* kGreen   = "\033[32m";   // Info
    static const char* kYellow  = "\033[33m";   // Warn
    static const char* kRed     = "\033[31m";   // Error
    const char* color = kReset;
    switch (level) {
        case LogLevel::Debug: color = kGray;   break;
        case LogLevel::Info:  color = kGreen;  break;
        case LogLevel::Warn:  color = kYellow; break;
        case LogLevel::Error: color = kRed;    break;
    }
    QString timeStr = entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logLine = source.isEmpty()
        ? QString("%1 [%2] %3").arg(timeStr, logLevelToString(level), message)
        : QString("%1 [%2] [%3] %4").arg(timeStr, logLevelToString(level), source, message);
    fprintf(stderr, "%s%s%s\n", color, qPrintable(logLine), kReset);
    fflush(stderr);

    writeToFile(entry);
    emit logAdded(entry);

    inLog = false;
}

void Logger::writeToFile(const LogEntry& entry) {
    QMutexLocker locker(&mutex_);

    if (!file_.isOpen()) {
        return;
    }

    QTextStream stream(&file_);
    stream << entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz") << " [" << logLevelToString(entry.level) << "] ";

    if (!entry.source.isEmpty()) {
        stream << "[" << entry.source << "] ";
    }

    stream << entry.message << "\n";
    stream.flush();
}

}  // namespace wekey
