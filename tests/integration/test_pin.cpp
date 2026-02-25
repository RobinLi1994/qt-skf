/**
 * @file test_pin.cpp
 * @brief PIN 管理集成测试
 *
 * 通过 MockPlugin 测试 AppService 的 PIN 登录、修改、解锁流程。
 */

#include <QSignalSpy>
#include <QTest>
#include <memory>

#include "integration/MockPlugin.h"
#include "core/application/AppService.h"
#include "plugin/PluginManager.h"

using namespace wekey;
using namespace wekey::test;

class TestPin : public QObject {
    Q_OBJECT

private:
    std::shared_ptr<MockPlugin> mock_;

    void setupMockPlugin() {
        auto& pm = PluginManager::instance();
        if (pm.listPlugins().contains("mock")) {
            pm.unregisterPlugin("mock");
        }

        mock_ = std::make_shared<MockPlugin>();
        mock_->addDevice("DEV001");
        mock_->addApp("DEV001", "APP001", "123456", "admin123");

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

    // --- 登录成功 ---

    void testLoginSuccess() {
        QSignalSpy loginSpy(&AppService::instance(), &AppService::loginStateChanged);

        auto result = AppService::instance().login("DEV001", "APP001", "user", "123456");
        QVERIFY(result.isOk());
        QCOMPARE(loginSpy.count(), 1);

        auto args = loginSpy.takeFirst();
        QCOMPARE(args.at(0).toString(), QString("DEV001"));
        QCOMPARE(args.at(1).toString(), QString("APP001"));
        QCOMPARE(args.at(2).toBool(), true);
    }

    // --- PIN 错误 ---

    void testLoginPinIncorrect() {
        QSignalSpy pinErrSpy(&AppService::instance(), &AppService::pinError);
        QSignalSpy loginSpy(&AppService::instance(), &AppService::loginStateChanged);

        auto result = AppService::instance().login("DEV001", "APP001", "user", "wrong");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::SkfPinIncorrect);

        QCOMPARE(pinErrSpy.count(), 1);
        QCOMPARE(loginSpy.count(), 0);

        auto args = pinErrSpy.takeFirst();
        QCOMPARE(args.at(0).toString(), QString("DEV001"));
        QCOMPARE(args.at(1).toString(), QString("APP001"));
        int retryCount = args.at(2).toInt();
        QVERIFY(retryCount < 10);
    }

    // --- PIN 锁定 ---

    void testLoginPinLocked() {
        // 先耗尽重试次数
        mock_->retryCount_["DEV001/APP001/user"] = 0;

        QSignalSpy lockedSpy(&AppService::instance(), &AppService::pinLocked);

        auto result = AppService::instance().login("DEV001", "APP001", "user", "123456");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::SkfPinLocked);
        QCOMPARE(lockedSpy.count(), 1);
    }

    // --- 修改 PIN ---

    void testChangePinSuccess() {
        auto result = AppService::instance().changePin("DEV001", "APP001", "user", "123456", "newpin");
        QVERIFY(result.isOk());

        // 旧 PIN 登录失败
        auto loginResult = AppService::instance().login("DEV001", "APP001", "user", "123456");
        QVERIFY(loginResult.isErr());

        // 新 PIN 登录成功
        loginResult = AppService::instance().login("DEV001", "APP001", "user", "newpin");
        QVERIFY(loginResult.isOk());
    }

    void testChangePinWrongOld() {
        auto result = AppService::instance().changePin("DEV001", "APP001", "user", "wrongold", "newpin");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::SkfPinIncorrect);

        // 原 PIN 仍然有效
        auto loginResult = AppService::instance().login("DEV001", "APP001", "user", "123456");
        QVERIFY(loginResult.isOk());
    }

    // --- 解锁 PIN ---

    void testUnlockPinSuccess() {
        // 锁定用户 PIN
        mock_->retryCount_["DEV001/APP001/user"] = 0;

        auto result = AppService::instance().unlockPin("DEV001", "APP001", "admin123", "newpin", {});
        QVERIFY(result.isOk());

        // 解锁后新 PIN 可用
        auto loginResult = AppService::instance().login("DEV001", "APP001", "user", "newpin");
        QVERIFY(loginResult.isOk());
    }

    // --- 获取重试次数（表格驱动）---

    void testGetRetryCount_data() {
        QTest::addColumn<QString>("role");
        QTest::addColumn<int>("expectedCount");

        QTest::newRow("user default") << "user" << 10;
        QTest::newRow("admin default") << "admin" << 10;
    }

    void testGetRetryCount() {
        QFETCH(QString, role);
        QFETCH(int, expectedCount);

        auto result = AppService::instance().getRetryCount("DEV001", "APP001", role);
        QVERIFY(result.isOk());
        QCOMPARE(result.value(), expectedCount);
    }

    // --- PIN 重试递减 ---

    void testPinRetryDecrement() {
        auto initialResult = AppService::instance().getRetryCount("DEV001", "APP001", "user");
        QVERIFY(initialResult.isOk());
        int initial = initialResult.value();

        // 第一次错误
        AppService::instance().login("DEV001", "APP001", "user", "wrong1");
        auto afterFirst = AppService::instance().getRetryCount("DEV001", "APP001", "user");
        QVERIFY(afterFirst.isOk());
        QCOMPARE(afterFirst.value(), initial - 1);

        // 第二次错误
        AppService::instance().login("DEV001", "APP001", "user", "wrong2");
        auto afterSecond = AppService::instance().getRetryCount("DEV001", "APP001", "user");
        QVERIFY(afterSecond.isOk());
        QCOMPARE(afterSecond.value(), initial - 2);
    }

    // --- 登录成功重置重试次数 ---

    void testLoginResetsRetryCount() {
        // 先产生一次错误
        AppService::instance().login("DEV001", "APP001", "user", "wrong");
        auto after = AppService::instance().getRetryCount("DEV001", "APP001", "user");
        QVERIFY(after.isOk());
        QVERIFY(after.value() < 10);

        // 正确登录重置
        AppService::instance().login("DEV001", "APP001", "user", "123456");
        auto reset = AppService::instance().getRetryCount("DEV001", "APP001", "user");
        QVERIFY(reset.isOk());
        QCOMPARE(reset.value(), 10);
    }
};

QTEST_MAIN(TestPin)
#include "test_pin.moc"
