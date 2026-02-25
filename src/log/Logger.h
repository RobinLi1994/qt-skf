/**
 * @file Logger.h
 * @brief 日志记录器单例类
 *
 * 提供多级别日志输出，支持文件和信号两种输出方式
 */

#pragma once

#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QString>

namespace wekey {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel { Debug = 0, Info = 1, Warn = 2, Error = 3 };

/**
 * @brief 日志条目结构体
 */
struct LogEntry {
    QDateTime timestamp;
    LogLevel level;
    QString message;
    QString source;
};

/**
 * @brief 日志级别转字符串
 * @param level 日志级别
 * @return 级别字符串（如 "DEBUG", "INFO"）
 */
QString logLevelToString(LogLevel level);

/**
 * @brief 字符串转日志级别
 * @param str 级别字符串
 * @return 日志级别，无效字符串返回 Info
 */
LogLevel stringToLogLevel(const QString& str);

/**
 * @brief 日志记录器单例类
 *
 * 线程安全的日志记录器，支持多级别日志和文件输出
 */
class Logger : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return Logger 单例引用
     */
    static Logger& instance();

    // 禁用拷贝和移动
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief 设置日志级别
     * @param level 最低输出级别
     */
    void setLevel(LogLevel level);

    /**
     * @brief 获取当前日志级别
     * @return 当前日志级别
     */
    LogLevel level() const;

    /**
     * @brief 设置日志文件输出路径
     * @param path 文件路径，空字符串表示关闭文件输出
     */
    void setOutputPath(const QString& path);

    /**
     * @brief 输出 Debug 级别日志
     * @param message 日志消息
     * @param source 日志来源
     */
    void debug(const QString& message, const QString& source = {});

    /**
     * @brief 输出 Info 级别日志
     * @param message 日志消息
     * @param source 日志来源
     */
    void info(const QString& message, const QString& source = {});

    /**
     * @brief 输出 Warn 级别日志
     * @param message 日志消息
     * @param source 日志来源
     */
    void warn(const QString& message, const QString& source = {});

    /**
     * @brief 输出 Error 级别日志
     * @param message 日志消息
     * @param source 日志来源
     */
    void error(const QString& message, const QString& source = {});

signals:
    /**
     * @brief 日志添加信号
     * @param entry 新添加的日志条目
     */
    void logAdded(const LogEntry& entry);

private:
    Logger();
    ~Logger() override;

    /**
     * @brief 内部日志处理
     */
    void log(LogLevel level, const QString& message, const QString& source);

    /**
     * @brief 写入日志文件
     */
    void writeToFile(const LogEntry& entry);

    LogLevel level_{LogLevel::Debug};
    QFile file_;
    QMutex mutex_;
};

// 便捷宏
#define LOG_DEBUG(msg) wekey::Logger::instance().debug(msg, __FUNCTION__)
#define LOG_INFO(msg) wekey::Logger::instance().info(msg, __FUNCTION__)
#define LOG_WARN(msg) wekey::Logger::instance().warn(msg, __FUNCTION__)
#define LOG_ERROR(msg) wekey::Logger::instance().error(msg, __FUNCTION__)

}  // namespace wekey

Q_DECLARE_METATYPE(wekey::LogEntry)
