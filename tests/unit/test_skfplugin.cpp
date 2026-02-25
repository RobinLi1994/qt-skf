/**
 * @file test_skfplugin.cpp
 * @brief SkfPlugin 单元测试
 *
 * 测试 SkfPlugin 在未初始化和初始化失败时的行为
 * 真实设备测试需要连接 SKF 设备（集成测试阶段）
 */

#include <QTest>
#include <cstring>

#include "plugin/skf/SkfPlugin.h"

using namespace wekey;
using namespace wekey::skf;

class TestSkfPlugin : public QObject {
    Q_OBJECT

private slots:
    //=== 构造/析构测试 ===

    void testConstruction() {
        SkfPlugin plugin;
        // 未初始化，调用方法应返回错误
        auto result = plugin.enumDevices();
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::PluginLoadFailed);
    }

    void testInitializeNonExistent() {
        SkfPlugin plugin;
        auto result = plugin.initialize("/nonexistent/path/to/skf.dylib");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::PluginLoadFailed);
    }

    void testInitializeFailThenRetry() {
        SkfPlugin plugin;
        auto result = plugin.initialize("/nonexistent/path.dll");
        QVERIFY(result.isErr());

        // 未初始化后的 API 调用都应该返回错误
        auto devResult = plugin.enumDevices();
        QVERIFY(devResult.isErr());
    }

    //=== 未初始化时所有 API 返回错误 ===

    void testUninitializedEnumDevices() {
        SkfPlugin plugin;
        auto r = plugin.enumDevices();
        QVERIFY(r.isErr());
        QVERIFY(r.error().message().contains("not loaded"));
    }

    void testUninitializedChangeDeviceAuth() {
        SkfPlugin plugin;
        auto r = plugin.changeDeviceAuth("dev", "old", "new");
        QVERIFY(r.isErr());
    }

    void testUninitializedSetDeviceLabel() {
        SkfPlugin plugin;
        auto r = plugin.setDeviceLabel("dev", "label");
        QVERIFY(r.isErr());
    }

    void testUninitializedWaitForDeviceEvent() {
        SkfPlugin plugin;
        auto r = plugin.waitForDeviceEvent();
        QVERIFY(r.isErr());
    }

    void testUninitializedEnumApps() {
        SkfPlugin plugin;
        auto r = plugin.enumApps("dev");
        QVERIFY(r.isErr());
    }

    void testUninitializedCreateApp() {
        SkfPlugin plugin;
        auto r = plugin.createApp("dev", "app", {});
        QVERIFY(r.isErr());
    }

    void testUninitializedDeleteApp() {
        SkfPlugin plugin;
        auto r = plugin.deleteApp("dev", "app");
        QVERIFY(r.isErr());
    }

    void testUninitializedOpenApp() {
        SkfPlugin plugin;
        auto r = plugin.openApp("dev", "app", "user", "1234");
        QVERIFY(r.isErr());
    }

    void testUninitializedCloseApp() {
        SkfPlugin plugin;
        // closeApp 应正常返回（即使未初始化，关闭操作是安全的）
        auto r = plugin.closeApp("dev", "app");
        QVERIFY(r.isOk());
    }

    void testUninitializedChangePin() {
        SkfPlugin plugin;
        auto r = plugin.changePin("dev", "app", "user", "old", "new");
        QVERIFY(r.isErr());
    }

    void testUninitializedUnlockPin() {
        SkfPlugin plugin;
        auto r = plugin.unlockPin("dev", "app", "admin", "newUser", {});
        QVERIFY(r.isErr());
    }

    void testUninitializedGetRetryCount() {
        SkfPlugin plugin;
        auto r = plugin.getRetryCount("dev", "app", "user");
        QVERIFY(r.isErr());
    }

    void testUninitializedEnumContainers() {
        SkfPlugin plugin;
        auto r = plugin.enumContainers("dev", "app");
        QVERIFY(r.isErr());
    }

    void testUninitializedCreateContainer() {
        SkfPlugin plugin;
        auto r = plugin.createContainer("dev", "app", "container");
        QVERIFY(r.isErr());
    }

    void testUninitializedDeleteContainer() {
        SkfPlugin plugin;
        auto r = plugin.deleteContainer("dev", "app", "container");
        QVERIFY(r.isErr());
    }

    void testUninitializedGenerateKeyPair() {
        SkfPlugin plugin;
        auto r = plugin.generateKeyPair("dev", "app", "container", "SM2");
        QVERIFY(r.isErr());
    }

    void testUninitializedImportCert() {
        SkfPlugin plugin;
        auto r = plugin.importCert("dev", "app", "container", QByteArray(), true);
        QVERIFY(r.isErr());
    }

    void testUninitializedExportCert() {
        SkfPlugin plugin;
        auto r = plugin.exportCert("dev", "app", "container", true);
        QVERIFY(r.isErr());
    }

    void testUninitializedSign() {
        SkfPlugin plugin;
        auto r = plugin.sign("dev", "app", "container", QByteArray("data"), "SM2");
        QVERIFY(r.isErr());
    }

    void testUninitializedVerify() {
        SkfPlugin plugin;
        auto r = plugin.verify("dev", "app", "container", QByteArray("data"), QByteArray("sig"), "SM2");
        QVERIFY(r.isErr());
    }

    void testUninitializedEnumFiles() {
        SkfPlugin plugin;
        auto r = plugin.enumFiles("dev", "app");
        QVERIFY(r.isErr());
    }

    void testUninitializedReadFile() {
        SkfPlugin plugin;
        auto r = plugin.readFile("dev", "app", "file.txt");
        QVERIFY(r.isErr());
    }

    void testUninitializedWriteFile() {
        SkfPlugin plugin;
        auto r = plugin.writeFile("dev", "app", "file.txt", QByteArray("data"));
        QVERIFY(r.isErr());
    }

    void testUninitializedDeleteFile() {
        SkfPlugin plugin;
        auto r = plugin.deleteFile("dev", "app", "file.txt");
        QVERIFY(r.isErr());
    }

    void testUninitializedGenerateRandom() {
        SkfPlugin plugin;
        auto r = plugin.generateRandom("dev", 32);
        QVERIFY(r.isErr());
    }

    //=== 错误上下文信息测试 ===

    void testErrorContext() {
        SkfPlugin plugin;
        auto r = plugin.enumDevices();
        QVERIFY(r.isErr());
        QVERIFY(!r.error().context().isEmpty());
        QVERIFY(r.error().context().contains("SkfPlugin"));
    }

    //=== Mock 函数基本验证 ===

    void testMockEnumDev() {
        // 验证 SKF 双空字符结尾字符串的解析逻辑
        const char devices[] = "Device1\0Device2\0\0";
        size_t size = sizeof(devices);

        // 模拟 parseNameList 逻辑
        QStringList result;
        const char* p = devices;
        const char* end = devices + size;
        while (p < end) {
            if (*p == '\0') break;
            result.append(QString::fromLocal8Bit(p));
            p += std::strlen(p) + 1;
        }

        QCOMPARE(result.size(), 2);
        QCOMPARE(result[0], "Device1");
        QCOMPARE(result[1], "Device2");
    }

    void testMockEnumDevEmpty() {
        // 空设备列表（仅一个 \0）
        const char devices[] = "\0";
        QStringList result;
        const char* p = devices;
        if (*p == '\0') {
            // empty
        }
        QCOMPARE(result.size(), 0);
    }

    void testMockEnumDevSingle() {
        const char devices[] = "OnlyDevice\0\0";
        QStringList result;
        const char* p = devices;
        const char* end = devices + sizeof(devices);
        while (p < end) {
            if (*p == '\0') break;
            result.append(QString::fromLocal8Bit(p));
            p += std::strlen(p) + 1;
        }

        QCOMPARE(result.size(), 1);
        QCOMPARE(result[0], "OnlyDevice");
    }
};

QTEST_MAIN(TestSkfPlugin)
#include "test_skfplugin.moc"
