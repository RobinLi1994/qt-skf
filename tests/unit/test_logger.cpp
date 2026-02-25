/**
 * @file test_logger.cpp
 * @brief Logger 类单元测试
 *
 * 测试用例 M1.4.3T
 */

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "log/Logger.h"

using namespace wekey;

class TestLogger : public QObject {
    Q_OBJECT

private:
    QTemporaryDir tempDir_;

private slots:
    void cleanup() {
        // 每个测试后重置
        Logger::instance().setLevel(LogLevel::Debug);
    }

    /**
     * @brief 测试单例模式
     */
    void testSingleton() {
        Logger& logger1 = Logger::instance();
        Logger& logger2 = Logger::instance();
        QCOMPARE(&logger1, &logger2);
    }

    /**
     * @brief 测试日志级别设置
     */
    void testSetLevel() {
        Logger& logger = Logger::instance();

        logger.setLevel(LogLevel::Info);
        // 级别设置后应该只输出 >= Info 的日志
        // 这个通过信号测试验证

        logger.setLevel(LogLevel::Error);
        // 只输出 Error 级别

        logger.setLevel(LogLevel::Debug);
        // 输出所有级别
    }

    /**
     * @brief 测试日志信号发射
     */
    void testLogSignal() {
        Logger& logger = Logger::instance();
        logger.setLevel(LogLevel::Debug);

        qRegisterMetaType<LogEntry>("LogEntry");
        QSignalSpy spy(&logger, &Logger::logAdded);

        logger.info("Test message", "TestSource");

        QCOMPARE(spy.count(), 1);

        QList<QVariant> arguments = spy.takeFirst();
        LogEntry entry = arguments.at(0).value<LogEntry>();

        QCOMPARE(entry.level, LogLevel::Info);
        QCOMPARE(entry.message, QString("Test message"));
        QCOMPARE(entry.source, QString("TestSource"));
        QVERIFY(!entry.timestamp.isNull());
    }

    /**
     * @brief 测试日志级别过滤
     */
    void testLogLevelFiltering() {
        Logger& logger = Logger::instance();

        qRegisterMetaType<LogEntry>("LogEntry");
        QSignalSpy spy(&logger, &Logger::logAdded);

        // 设置为 Warn 级别
        logger.setLevel(LogLevel::Warn);

        // Debug 和 Info 应该被过滤
        logger.debug("Debug message", "Test");
        logger.info("Info message", "Test");
        QCOMPARE(spy.count(), 0);

        // Warn 和 Error 应该通过
        logger.warn("Warn message", "Test");
        QCOMPARE(spy.count(), 1);

        logger.error("Error message", "Test");
        QCOMPARE(spy.count(), 2);
    }

    /**
     * @brief 测试各级别日志方法
     */
    void testLogMethods() {
        Logger& logger = Logger::instance();
        logger.setLevel(LogLevel::Debug);

        qRegisterMetaType<LogEntry>("LogEntry");
        QSignalSpy spy(&logger, &Logger::logAdded);

        logger.debug("Debug", "Src");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.last().at(0).value<LogEntry>().level, LogLevel::Debug);

        logger.info("Info", "Src");
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.last().at(0).value<LogEntry>().level, LogLevel::Info);

        logger.warn("Warn", "Src");
        QCOMPARE(spy.count(), 3);
        QCOMPARE(spy.last().at(0).value<LogEntry>().level, LogLevel::Warn);

        logger.error("Error", "Src");
        QCOMPARE(spy.count(), 4);
        QCOMPARE(spy.last().at(0).value<LogEntry>().level, LogLevel::Error);
    }

    /**
     * @brief 测试日志文件输出
     */
    void testFileOutput() {
        Logger& logger = Logger::instance();
        logger.setLevel(LogLevel::Debug);

        QString logPath = tempDir_.path() + "/test.log";
        logger.setOutputPath(logPath);

        logger.info("File test message", "FileTest");

        // 等待文件写入
        QTest::qWait(100);

        QFile file(logPath);
        QVERIFY(file.exists());
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

        QString content = file.readAll();
        file.close();

        QVERIFY(content.contains("File test message"));
        QVERIFY(content.contains("FileTest"));
        QVERIFY(content.contains("INFO"));

        // 清理：关闭输出
        logger.setOutputPath("");
    }

    /**
     * @brief 测试 LogEntry 结构体
     */
    void testLogEntry() {
        LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = LogLevel::Error;
        entry.message = "Test";
        entry.source = "Source";

        QCOMPARE(entry.level, LogLevel::Error);
        QCOMPARE(entry.message, QString("Test"));
        QCOMPARE(entry.source, QString("Source"));
    }

    /**
     * @brief 测试日志级别字符串转换
     */
    void testLevelToString() {
        QCOMPARE(logLevelToString(LogLevel::Debug), QString("DEBUG"));
        QCOMPARE(logLevelToString(LogLevel::Info), QString("INFO"));
        QCOMPARE(logLevelToString(LogLevel::Warn), QString("WARN"));
        QCOMPARE(logLevelToString(LogLevel::Error), QString("ERROR"));
    }

    /**
     * @brief 测试字符串到日志级别转换
     */
    void testStringToLevel() {
        QCOMPARE(stringToLogLevel("debug"), LogLevel::Debug);
        QCOMPARE(stringToLogLevel("info"), LogLevel::Info);
        QCOMPARE(stringToLogLevel("warn"), LogLevel::Warn);
        QCOMPARE(stringToLogLevel("error"), LogLevel::Error);

        // 大小写不敏感
        QCOMPARE(stringToLogLevel("DEBUG"), LogLevel::Debug);
        QCOMPARE(stringToLogLevel("Info"), LogLevel::Info);

        // 无效字符串默认返回 Info
        QCOMPARE(stringToLogLevel("invalid"), LogLevel::Info);
    }
};

QTEST_MAIN(TestLogger)
#include "test_logger.moc"
