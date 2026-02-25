/**
 * @file test_fileservice.cpp
 * @brief FileService 单元测试
 */

#include <QTest>

#include "core/file/FileService.h"
#include "plugin/PluginManager.h"

using namespace wekey;

class TestFileService : public QObject {
    Q_OBJECT

private slots:
    void cleanup() {
        auto& mgr = PluginManager::instance();
        for (const auto& name : mgr.listPlugins()) {
            mgr.unregisterPlugin(name);
        }
    }

    void testEnumFilesNoActivePlugin() {
        auto result = FileService::instance().enumFiles("dev", "app");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testReadFileNoActivePlugin() {
        auto result = FileService::instance().readFile("dev", "app", "file.txt");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testWriteFileNoActivePlugin() {
        auto result = FileService::instance().writeFile("dev", "app", "file.txt", QByteArray("data"));
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testDeleteFileNoActivePlugin() {
        auto result = FileService::instance().deleteFile("dev", "app", "file.txt");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testGenerateRandomNoActivePlugin() {
        auto result = FileService::instance().generateRandom("dev", 32);
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testErrorContext() {
        auto result = FileService::instance().enumFiles("dev", "app");
        QVERIFY(result.isErr());
        QVERIFY(result.error().context().contains("FileService"));
    }
};

QTEST_MAIN(TestFileService)
#include "test_fileservice.moc"
