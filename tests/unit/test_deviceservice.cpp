/**
 * @file test_deviceservice.cpp
 * @brief DeviceService 单元测试
 */

#include <QSignalSpy>
#include <QTest>

#include "core/device/DeviceService.h"
#include "plugin/PluginManager.h"

using namespace wekey;

class TestDeviceService : public QObject {
    Q_OBJECT

private slots:
    void cleanup() {
        auto& mgr = PluginManager::instance();
        for (const auto& name : mgr.listPlugins()) {
            mgr.unregisterPlugin(name);
        }
    }

    void testSingleton() {
        auto& a = DeviceService::instance();
        auto& b = DeviceService::instance();
        QCOMPARE(&a, &b);
    }

    void testEnumDevicesNoActivePlugin() {
        auto& svc = DeviceService::instance();
        auto result = svc.enumDevices();
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testChangeDeviceAuthNoActivePlugin() {
        auto& svc = DeviceService::instance();
        auto result = svc.changeDeviceAuth("dev", "old", "new");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testSetDeviceLabelNoActivePlugin() {
        auto& svc = DeviceService::instance();
        auto result = svc.setDeviceLabel("dev", "label");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testStartStopDeviceMonitor() {
        auto& svc = DeviceService::instance();
        QVERIFY(!svc.isMonitoring());
        svc.startDeviceMonitor();
        QVERIFY(svc.isMonitoring());
        svc.stopDeviceMonitor();
        QVERIFY(!svc.isMonitoring());
    }

    void testStopDeviceMonitorWhenNotStarted() {
        auto& svc = DeviceService::instance();
        svc.stopDeviceMonitor();
        QVERIFY(!svc.isMonitoring());
    }

    void testDeviceListChangedSignal() {
        auto& svc = DeviceService::instance();
        QSignalSpy spy(&svc, &DeviceService::deviceListChanged);
        // 无激活插件，enumDevices 失败，不应发信号
        svc.enumDevices();
        QCOMPARE(spy.count(), 0);
    }

    void testErrorContext() {
        auto& svc = DeviceService::instance();
        auto result = svc.enumDevices();
        QVERIFY(result.isErr());
        QVERIFY(result.error().context().contains("DeviceService"));
    }
};

QTEST_MAIN(TestDeviceService)
#include "test_deviceservice.moc"
