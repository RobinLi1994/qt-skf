/**
 * @file test_e2e.cpp
 * @brief 端到端集成测试
 *
 * 通过 MockPlugin 测试 Service 层完整工作流，验证各服务正确委托到插件并发射信号。
 */

#include <QSignalSpy>
#include <QTest>
#include <memory>

#include "integration/MockPlugin.h"
#include "core/application/AppService.h"
#include "core/container/ContainerService.h"
#include "core/crypto/CertService.h"
#include "core/device/DeviceService.h"
#include "core/file/FileService.h"
#include "plugin/PluginManager.h"

using namespace wekey;
using namespace wekey::test;

class TestE2E : public QObject {
    Q_OBJECT

private:
    std::shared_ptr<MockPlugin> mock_;

    void setupMockPlugin() {
        // 先清理旧注册
        auto& pm = PluginManager::instance();
        if (pm.listPlugins().contains("mock")) {
            pm.unregisterPlugin("mock");
        }

        mock_ = std::make_shared<MockPlugin>();
        mock_->addDevice("DEV001");
        mock_->addApp("DEV001", "APP001");
        mock_->addContainer("DEV001", "APP001", "CTN001");

        auto result = pm.registerPluginInstance("mock", mock_);
        QVERIFY(result.isOk());
        result = pm.setActivePlugin("mock");
        QVERIFY(result.isOk());
    }

private slots:
    void init() {
        setupMockPlugin();
    }

    void cleanup() {
        auto& pm = PluginManager::instance();
        if (pm.listPlugins().contains("mock")) {
            pm.unregisterPlugin("mock");
        }
    }

    // --- 设备服务测试 ---

    void testEnumDevicesViaService_data() {
        QTest::addColumn<int>("deviceCount");
        QTest::addColumn<bool>("expectOk");

        QTest::newRow("one device") << 1 << true;
        QTest::newRow("no devices") << 0 << true;
        QTest::newRow("multiple devices") << 3 << true;
    }

    void testEnumDevicesViaService() {
        QFETCH(int, deviceCount);
        QFETCH(bool, expectOk);

        mock_->devices_.clear();
        for (int i = 0; i < deviceCount; ++i) {
            mock_->addDevice(QString("DEV%1").arg(i));
        }

        auto result = DeviceService::instance().enumDevices();
        QCOMPARE(result.isOk(), expectOk);
        if (result.isOk()) {
            QCOMPARE(result.value().size(), deviceCount);
        }
    }

    void testEnumDevicesSignal() {
        QSignalSpy spy(&DeviceService::instance(), &DeviceService::deviceListChanged);
        auto result = DeviceService::instance().enumDevices();
        QVERIFY(result.isOk());
        QCOMPARE(spy.count(), 1);
    }

    // --- 登录/登出循环 ---

    void testLoginLogoutCycle() {
        QSignalSpy loginSpy(&AppService::instance(), &AppService::loginStateChanged);

        auto loginResult = AppService::instance().login("DEV001", "APP001", "user", "123456");
        QVERIFY(loginResult.isOk());
        QCOMPARE(loginSpy.count(), 1);
        auto args = loginSpy.takeFirst();
        QCOMPARE(args.at(2).toBool(), true);

        auto logoutResult = AppService::instance().logout("DEV001", "APP001");
        QVERIFY(logoutResult.isOk());
        QCOMPARE(loginSpy.count(), 1);
        args = loginSpy.takeFirst();
        QCOMPARE(args.at(2).toBool(), false);
    }

    // --- 应用管理 ---

    void testCreateDeleteApp() {
        auto createResult = AppService::instance().createApp("DEV001", "NEWAPP", {});
        QVERIFY(createResult.isOk());

        auto enumResult = AppService::instance().enumApps("DEV001");
        QVERIFY(enumResult.isOk());
        bool found = false;
        for (const auto& app : enumResult.value()) {
            if (app.appName == "NEWAPP") { found = true; break; }
        }
        QVERIFY(found);

        auto deleteResult = AppService::instance().deleteApp("DEV001", "NEWAPP");
        QVERIFY(deleteResult.isOk());

        enumResult = AppService::instance().enumApps("DEV001");
        QVERIFY(enumResult.isOk());
        found = false;
        for (const auto& app : enumResult.value()) {
            if (app.appName == "NEWAPP") { found = true; break; }
        }
        QVERIFY(!found);
    }

    // --- 容器操作 ---

    void testContainerOperations() {
        auto createResult = ContainerService::instance().createContainer("DEV001", "APP001", "NEWCTN");
        QVERIFY(createResult.isOk());

        auto enumResult = ContainerService::instance().enumContainers("DEV001", "APP001");
        QVERIFY(enumResult.isOk());
        QVERIFY(enumResult.value().size() >= 2);

        auto deleteResult = ContainerService::instance().deleteContainer("DEV001", "APP001", "NEWCTN");
        QVERIFY(deleteResult.isOk());

        enumResult = ContainerService::instance().enumContainers("DEV001", "APP001");
        QVERIFY(enumResult.isOk());
        bool found = false;
        for (const auto& c : enumResult.value()) {
            if (c.containerName == "NEWCTN") { found = true; break; }
        }
        QVERIFY(!found);
    }

    // --- 证书流程 ---

    void testCertFlow() {
        auto keyResult = CertService::instance().generateKeyPair("DEV001", "APP001", "CTN001", "SM2");
        QVERIFY(keyResult.isOk());
        QVERIFY(!keyResult.value().isEmpty());

        QByteArray certData = "MOCK_CERT_DATA";
        auto importResult = CertService::instance().importCert("DEV001", "APP001", "CTN001", certData, true);
        QVERIFY(importResult.isOk());

        auto exportResult = CertService::instance().exportCert("DEV001", "APP001", "CTN001", true);
        QVERIFY(exportResult.isOk());
        QCOMPARE(exportResult.value(), certData);
    }

    // --- 签名验签 ---

    void testSignVerify() {
        QByteArray data = "Hello, World!";
        auto signResult = CertService::instance().sign("DEV001", "APP001", "CTN001", data, "SM3withSM2");
        QVERIFY(signResult.isOk());
        QByteArray signature = signResult.value();
        QVERIFY(!signature.isEmpty());

        auto verifyResult = CertService::instance().verify("DEV001", "APP001", "CTN001", data, signature, "SM3withSM2");
        QVERIFY(verifyResult.isOk());
        QVERIFY(verifyResult.value());

        // 篡改签名应验证失败
        auto badResult = CertService::instance().verify("DEV001", "APP001", "CTN001", data, "BAD_SIG", "SM3withSM2");
        QVERIFY(badResult.isOk());
        QVERIFY(!badResult.value());
    }

    // --- 文件操作 ---

    void testFileOperations() {
        QByteArray content = "file content 123";
        auto writeResult = FileService::instance().writeFile("DEV001", "APP001", "test.txt", content);
        QVERIFY(writeResult.isOk());

        auto readResult = FileService::instance().readFile("DEV001", "APP001", "test.txt");
        QVERIFY(readResult.isOk());
        QCOMPARE(readResult.value(), content);

        auto enumResult = FileService::instance().enumFiles("DEV001", "APP001");
        QVERIFY(enumResult.isOk());
        QVERIFY(enumResult.value().contains("test.txt"));

        auto deleteResult = FileService::instance().deleteFile("DEV001", "APP001", "test.txt");
        QVERIFY(deleteResult.isOk());

        enumResult = FileService::instance().enumFiles("DEV001", "APP001");
        QVERIFY(enumResult.isOk());
        QVERIFY(!enumResult.value().contains("test.txt"));
    }

    // --- 随机数生成 ---

    void testGenerateRandom() {
        auto result = FileService::instance().generateRandom("DEV001", 32);
        QVERIFY(result.isOk());
        QCOMPARE(result.value().size(), 32);
    }

    // --- 完整工作流 ---

    void testFullWorkflow() {
        // 1. 枚举设备
        auto devResult = DeviceService::instance().enumDevices();
        QVERIFY(devResult.isOk());
        QVERIFY(!devResult.value().isEmpty());
        QString devName = devResult.value().first().deviceName;

        // 2. 枚举应用
        auto appResult = AppService::instance().enumApps(devName);
        QVERIFY(appResult.isOk());
        QVERIFY(!appResult.value().isEmpty());
        QString appName = appResult.value().first().appName;

        // 3. 登录
        auto loginResult = AppService::instance().login(devName, appName, "user", "123456");
        QVERIFY(loginResult.isOk());

        // 4. 枚举容器
        auto ctnResult = ContainerService::instance().enumContainers(devName, appName);
        QVERIFY(ctnResult.isOk());
        QVERIFY(!ctnResult.value().isEmpty());
        QString ctnName = ctnResult.value().first().containerName;

        // 5. 签名
        QByteArray data = "test data";
        auto signResult = CertService::instance().sign(devName, appName, ctnName, data, "SM3withSM2");
        QVERIFY(signResult.isOk());

        // 6. 验签
        auto verifyResult = CertService::instance().verify(devName, appName, ctnName, data,
                                                            signResult.value(), "SM3withSM2");
        QVERIFY(verifyResult.isOk());
        QVERIFY(verifyResult.value());

        // 7. 登出
        auto logoutResult = AppService::instance().logout(devName, appName);
        QVERIFY(logoutResult.isOk());
    }

    // --- 错误注入 ---

    void testErrorInjection() {
        mock_->nextError_ = Error(Error::Fail, "Injected error", "test");
        auto result = DeviceService::instance().enumDevices();
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::Fail);

        // 后续调用应正常
        auto result2 = DeviceService::instance().enumDevices();
        QVERIFY(result2.isOk());
    }

    // --- 信号传播 ---

    void testServiceSignalPropagation() {
        QSignalSpy devSpy(&DeviceService::instance(), &DeviceService::deviceListChanged);
        QSignalSpy loginSpy(&AppService::instance(), &AppService::loginStateChanged);
        QSignalSpy pinErrSpy(&AppService::instance(), &AppService::pinError);

        // 设备枚举触发 deviceListChanged
        DeviceService::instance().enumDevices();
        QCOMPARE(devSpy.count(), 1);

        // 登录触发 loginStateChanged
        AppService::instance().login("DEV001", "APP001", "user", "123456");
        QCOMPARE(loginSpy.count(), 1);

        // 错误PIN触发 pinError
        AppService::instance().login("DEV001", "APP001", "user", "wrong");
        QCOMPARE(pinErrSpy.count(), 1);
    }
};

QTEST_MAIN(TestE2E)
#include "test_e2e.moc"
