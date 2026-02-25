/**
 * @file DeviceService.h
 * @brief 设备服务
 *
 * 封装设备管理操作，委托给激活的驱动插件
 */

#pragma once

#include <QObject>
#include <QThread>
#include <atomic>

#include "common/Result.h"
#include "plugin/interface/PluginTypes.h"

namespace wekey {

class DeviceService : public QObject {
    Q_OBJECT

public:
    static DeviceService& instance();

    DeviceService(const DeviceService&) = delete;
    DeviceService& operator=(const DeviceService&) = delete;

    Result<QList<DeviceInfo>> enumDevices(bool login = false, bool emitSignals = true);
    Result<void> changeDeviceAuth(const QString& devName, const QString& oldPin, const QString& newPin);
    Result<void> setDeviceLabel(const QString& devName, const QString& label);

    void startDeviceMonitor();
    void stopDeviceMonitor();
    bool isMonitoring() const;

signals:
    void deviceInserted(const QString& devName);
    void deviceRemoved(const QString& devName);
    void deviceListChanged();

private:
    DeviceService();
    ~DeviceService() override;

    void monitorLoop();

    QThread monitorThread_;
    std::atomic<bool> monitoring_{false};
};

}  // namespace wekey
