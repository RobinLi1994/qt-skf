/**
 * @file DeviceService.cpp
 * @brief 设备服务实现
 */

#include "DeviceService.h"

#include "plugin/PluginManager.h"

namespace wekey {

DeviceService& DeviceService::instance() {
    static DeviceService instance;
    return instance;
}

DeviceService::DeviceService() : QObject(nullptr) {}

DeviceService::~DeviceService() {
    stopDeviceMonitor();
}

Result<QList<DeviceInfo>> DeviceService::enumDevices(bool login, bool emitSignals) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QList<DeviceInfo>>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "DeviceService::enumDevices"));
    }
    auto result = plugin->enumDevices(login);
    if (result.isOk() && emitSignals) {
        emit deviceListChanged();
    }
    return result;
}

Result<void> DeviceService::changeDeviceAuth(const QString& devName, const QString& oldPin, const QString& newPin) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "DeviceService::changeDeviceAuth"));
    }
    return plugin->changeDeviceAuth(devName, oldPin, newPin);
}

Result<void> DeviceService::setDeviceLabel(const QString& devName, const QString& label) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "DeviceService::setDeviceLabel"));
    }
    return plugin->setDeviceLabel(devName, label);
}

void DeviceService::startDeviceMonitor() {
    if (monitoring_) {
        return;
    }
    monitoring_ = true;

    QObject::connect(&monitorThread_, &QThread::started, [this]() {
        monitorLoop();
    });
    monitorThread_.start();
}

void DeviceService::stopDeviceMonitor() {
    if (!monitoring_) {
        return;
    }
    monitoring_ = false;
    monitorThread_.quit();
    monitorThread_.wait();
}

bool DeviceService::isMonitoring() const {
    return monitoring_;
}

void DeviceService::monitorLoop() {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return;
    }

    while (monitoring_) {
        auto result = plugin->waitForDeviceEvent();
        if (result.isErr()) {
            break;
        }

        int event = result.value();
        if (event == static_cast<int>(DeviceEvent::Inserted)) {
            emit deviceInserted({});
            emit deviceListChanged();
        } else if (event == static_cast<int>(DeviceEvent::Removed)) {
            emit deviceRemoved({});
            emit deviceListChanged();
        }
    }
}

}  // namespace wekey
