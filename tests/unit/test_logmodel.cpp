/**
 * @file test_logmodel.cpp
 * @brief LogModel 类单元测试
 *
 * 测试用例 M1.4.6T
 */

#include <QSignalSpy>
#include <QtTest>

#include "log/LogModel.h"
#include "log/Logger.h"

using namespace wekey;

class TestLogModel : public QObject {
    Q_OBJECT

private slots:
    void cleanup() {
        // 重置 Logger 级别
        Logger::instance().setLevel(LogLevel::Debug);
    }

    /**
     * @brief 测试模型初始状态
     */
    void testInitialState() {
        LogModel model;

        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), static_cast<int>(LogModel::Column::ColumnCount));
    }

    /**
     * @brief 测试列头数据
     */
    void testHeaderData() {
        LogModel model;

        // 水平头
        QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QString("时间"));
        QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QString("级别"));
        QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), QString("来源"));
        QCOMPARE(model.headerData(3, Qt::Horizontal).toString(), QString("消息"));

        // 垂直头应该返回空
        QVERIFY(model.headerData(0, Qt::Vertical).isNull());
    }

    /**
     * @brief 测试添加日志条目
     */
    void testAddEntry() {
        LogModel model;

        LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = LogLevel::Info;
        entry.source = "TestSource";
        entry.message = "Test message";

        QSignalSpy rowsInsertedSpy(&model, &QAbstractItemModel::rowsInserted);

        model.addEntry(entry);

        QCOMPARE(rowsInsertedSpy.count(), 1);
        QCOMPARE(model.rowCount(), 1);
    }

    /**
     * @brief 测试数据获取
     */
    void testData() {
        LogModel model;

        LogEntry entry;
        entry.timestamp = QDateTime(QDate(2024, 1, 15), QTime(10, 30, 45));
        entry.level = LogLevel::Warn;
        entry.source = "MySource";
        entry.message = "Warning message";

        model.addEntry(entry);

        // 测试各列数据
        QModelIndex idx0 = model.index(0, 0);
        QModelIndex idx1 = model.index(0, 1);
        QModelIndex idx2 = model.index(0, 2);
        QModelIndex idx3 = model.index(0, 3);

        QVERIFY(model.data(idx0).toString().contains("10:30:45"));
        QCOMPARE(model.data(idx1).toString(), QString("WARN"));
        QCOMPARE(model.data(idx2).toString(), QString("MySource"));
        QCOMPARE(model.data(idx3).toString(), QString("Warning message"));
    }

    /**
     * @brief 测试清空日志
     */
    void testClear() {
        LogModel model;

        // 添加几条日志
        for (int i = 0; i < 5; ++i) {
            LogEntry entry;
            entry.timestamp = QDateTime::currentDateTime();
            entry.level = LogLevel::Info;
            entry.source = "Test";
            entry.message = QString("Message %1").arg(i);
            model.addEntry(entry);
        }

        QCOMPARE(model.rowCount(), 5);

        QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);

        model.clear();

        QCOMPARE(modelResetSpy.count(), 1);
        QCOMPARE(model.rowCount(), 0);
    }

    /**
     * @brief 测试最大条目限制
     */
    void testMaxEntries() {
        LogModel model;
        model.setMaxEntries(5);

        // 添加 10 条日志
        for (int i = 0; i < 10; ++i) {
            LogEntry entry;
            entry.timestamp = QDateTime::currentDateTime();
            entry.level = LogLevel::Info;
            entry.source = "Test";
            entry.message = QString("Message %1").arg(i);
            model.addEntry(entry);
        }

        // 应该只保留最新的 5 条
        QCOMPARE(model.rowCount(), 5);

        // 验证保留的是最新的条目
        QModelIndex idx = model.index(0, 3);
        QCOMPARE(model.data(idx).toString(), QString("Message 5"));
    }

    /**
     * @brief 测试过滤功能
     */
    void testFilter() {
        LogModel model;

        // 添加不同级别的日志
        LogEntry debugEntry;
        debugEntry.timestamp = QDateTime::currentDateTime();
        debugEntry.level = LogLevel::Debug;
        debugEntry.source = "Test";
        debugEntry.message = "Debug message";
        model.addEntry(debugEntry);

        LogEntry infoEntry;
        infoEntry.timestamp = QDateTime::currentDateTime();
        infoEntry.level = LogLevel::Info;
        infoEntry.source = "Test";
        infoEntry.message = "Info message";
        model.addEntry(infoEntry);

        LogEntry errorEntry;
        errorEntry.timestamp = QDateTime::currentDateTime();
        errorEntry.level = LogLevel::Error;
        errorEntry.source = "Test";
        errorEntry.message = "Error message";
        model.addEntry(errorEntry);

        QCOMPARE(model.rowCount(), 3);

        // 设置过滤级别
        model.setFilterLevel(LogLevel::Info);
        QCOMPARE(model.rowCount(), 2);  // 只有 Info 和 Error

        model.setFilterLevel(LogLevel::Error);
        QCOMPARE(model.rowCount(), 1);  // 只有 Error

        model.setFilterLevel(LogLevel::Debug);
        QCOMPARE(model.rowCount(), 3);  // 全部显示
    }

    /**
     * @brief 测试文本搜索过滤
     */
    void testSearchFilter() {
        LogModel model;

        LogEntry entry1;
        entry1.timestamp = QDateTime::currentDateTime();
        entry1.level = LogLevel::Info;
        entry1.source = "ModuleA";
        entry1.message = "Connection established";
        model.addEntry(entry1);

        LogEntry entry2;
        entry2.timestamp = QDateTime::currentDateTime();
        entry2.level = LogLevel::Error;
        entry2.source = "ModuleB";
        entry2.message = "Connection failed";
        model.addEntry(entry2);

        LogEntry entry3;
        entry3.timestamp = QDateTime::currentDateTime();
        entry3.level = LogLevel::Info;
        entry3.source = "ModuleA";
        entry3.message = "Data received";
        model.addEntry(entry3);

        QCOMPARE(model.rowCount(), 3);

        // 搜索 "Connection"
        model.setSearchText("Connection");
        QCOMPARE(model.rowCount(), 2);

        // 搜索 "ModuleA"
        model.setSearchText("ModuleA");
        QCOMPARE(model.rowCount(), 2);

        // 搜索 "failed"
        model.setSearchText("failed");
        QCOMPARE(model.rowCount(), 1);

        // 清空搜索
        model.setSearchText("");
        QCOMPARE(model.rowCount(), 3);
    }

    /**
     * @brief 测试与 Logger 集成
     */
    void testLoggerIntegration() {
        LogModel model;
        model.connectToLogger();

        Logger& logger = Logger::instance();
        logger.setLevel(LogLevel::Debug);

        QCOMPARE(model.rowCount(), 0);

        logger.info("Integration test", "IntegrationTest");

        // Logger 发送信号后模型应该自动添加
        QCOMPARE(model.rowCount(), 1);

        QModelIndex idx = model.index(0, 3);
        QCOMPARE(model.data(idx).toString(), QString("Integration test"));
    }

    /**
     * @brief 测试获取原始日志条目
     */
    void testGetEntry() {
        LogModel model;

        LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = LogLevel::Error;
        entry.source = "TestSource";
        entry.message = "Test message";

        model.addEntry(entry);

        const LogEntry& retrieved = model.entry(0);
        QCOMPARE(retrieved.level, LogLevel::Error);
        QCOMPARE(retrieved.source, QString("TestSource"));
        QCOMPARE(retrieved.message, QString("Test message"));
    }
};

QTEST_MAIN(TestLogModel)
#include "test_logmodel.moc"
