/**
 * @file test_appservice.cpp
 * @brief AppService 单元测试
 */

#include <QSignalSpy>
#include <QTest>

#include "core/application/AppService.h"
#include "plugin/PluginManager.h"

using namespace wekey;

class TestAppService : public QObject {
    Q_OBJECT

private slots:
    void cleanup() {
        auto& mgr = PluginManager::instance();
        for (const auto& name : mgr.listPlugins()) {
            mgr.unregisterPlugin(name);
        }
    }

    void testEnumAppsNoActivePlugin() {
        auto result = AppService::instance().enumApps("dev");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testCreateAppNoActivePlugin() {
        auto result = AppService::instance().createApp("dev", "app", {});
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testDeleteAppNoActivePlugin() {
        auto result = AppService::instance().deleteApp("dev", "app");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testLoginNoActivePlugin() {
        auto result = AppService::instance().login("dev", "app", "user", "1234");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testLogoutNoActivePlugin() {
        auto result = AppService::instance().logout("dev", "app");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testChangePinNoActivePlugin() {
        auto result = AppService::instance().changePin("dev", "app", "user", "old", "new");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testUnlockPinNoActivePlugin() {
        auto result = AppService::instance().unlockPin("dev", "app", "admin", "newUser", {});
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testGetRetryCountNoActivePlugin() {
        auto result = AppService::instance().getRetryCount("dev", "app", "user");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testLoginStateChangedSignalNotEmittedOnFailure() {
        QSignalSpy spy(&AppService::instance(), &AppService::loginStateChanged);
        AppService::instance().login("dev", "app", "user", "1234");
        QCOMPARE(spy.count(), 0);
    }

    void testErrorContext() {
        auto result = AppService::instance().enumApps("dev");
        QVERIFY(result.isErr());
        QVERIFY(result.error().context().contains("AppService"));
    }
};

QTEST_MAIN(TestAppService)
#include "test_appservice.moc"
