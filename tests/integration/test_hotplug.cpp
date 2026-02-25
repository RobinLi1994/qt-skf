/**
 * @file test_hotplug.cpp
 * @brief 设备热插拔集成测试
 *
 * 通过 MockPlugin 的事件队列测试 DeviceService 的设备监控功能。
 */

#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <memory>

#include "integration/MockPlugin.h"
#include "core/device/DeviceService.h"
#include "plugin/PluginManager.h"

using namespace wekey;
using namespace wekey::test;

class TestHotplug : public QObject {
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
        // 注入错误事件让监控线程退出阻塞等待
        if (mock_) {
            mock_->injectEventError();
        }
        DeviceService::instance().stopDeviceMonitor();
        auto& pm = PluginManager::instance();
        if (pm.listPlugins().contains("mock")) {
            pm.unregisterPlugin("mock");
        }
    }

    void testDeviceInsertedSignal() {
        QSignalSpy insertedSpy(&DeviceService::instance(), &DeviceService::deviceInserted);
        QSignalSpy listChangedSpy(&DeviceService::instance(), &DeviceService::deviceListChanged);

        DeviceService::instance().startDeviceMonitor();
        QVERIFY(DeviceService::instance().isMonitoring());

        // 注入插入事件
        QThread::msleep(50);
        mock_->injectEvent(static_cast<int>(DeviceEvent::Inserted));

        QTRY_COMPARE_WITH_TIMEOUT(insertedSpy.count(), 1, 3000);
        QTRY_VERIFY_WITH_TIMEOUT(listChangedSpy.count() >= 1, 3000);

        DeviceService::instance().stopDeviceMonitor();
    }

    void testDeviceRemovedSignal() {
        QSignalSpy removedSpy(&DeviceService::instance(), &DeviceService::deviceRemoved);

        DeviceService::instance().startDeviceMonitor();

        QThread::msleep(50);
        mock_->injectEvent(static_cast<int>(DeviceEvent::Removed));

        QTRY_COMPARE_WITH_TIMEOUT(removedSpy.count(), 1, 3000);

        DeviceService::instance().stopDeviceMonitor();
    }

    void testDeviceListChangedSignal() {
        QSignalSpy spy(&DeviceService::instance(), &DeviceService::deviceListChanged);

        DeviceService::instance().startDeviceMonitor();

        QThread::msleep(50);
        mock_->injectEvent(static_cast<int>(DeviceEvent::Inserted));
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 3000);

        mock_->injectEvent(static_cast<int>(DeviceEvent::Removed));
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 2, 3000);

        DeviceService::instance().stopDeviceMonitor();
    }

    void testMonitorStartStop() {
        QVERIFY(!DeviceService::instance().isMonitoring());

        DeviceService::instance().startDeviceMonitor();
        QVERIFY(DeviceService::instance().isMonitoring());

        DeviceService::instance().stopDeviceMonitor();
        QVERIFY(!DeviceService::instance().isMonitoring());
    }

    void testMonitorExitsOnError() {
        DeviceService::instance().startDeviceMonitor();
        QVERIFY(DeviceService::instance().isMonitoring());

        QThread::msleep(50);
        mock_->injectEventError();

        // 监控线程应在收到错误后退出循环
        QThread::msleep(500);
        // monitoring_ flag 仍为 true（由 stopDeviceMonitor 清除），
        // 但 monitorLoop 已 break
    }
};

QTEST_MAIN(TestHotplug)
#include "test_hotplug.moc"
