/**
 * @file AppService.h
 * @brief 应用服务
 *
 * 封装应用管理操作，委托给激活的驱动插件
 */

#pragma once

#include <QObject>
#include <QVariantMap>

#include "common/Result.h"
#include "plugin/interface/PluginTypes.h"

namespace wekey {

class AppService : public QObject {
    Q_OBJECT

public:
    static AppService& instance();

    AppService(const AppService&) = delete;
    AppService& operator=(const AppService&) = delete;

    Result<QList<AppInfo>> enumApps(const QString& devName);
    Result<void> createApp(const QString& devName, const QString& appName, const QVariantMap& args);
    Result<void> deleteApp(const QString& devName, const QString& appName);
    Result<void> login(const QString& devName, const QString& appName, const QString& role, const QString& pin,
                       bool emitSignals = true);
    Result<void> logout(const QString& devName, const QString& appName, bool emitSignals = true);
    Result<void> changePin(const QString& devName, const QString& appName, const QString& role,
                           const QString& oldPin, const QString& newPin);
    Result<void> unlockPin(const QString& devName, const QString& appName, const QString& adminPin,
                           const QString& newUserPin, const QVariantMap& args);
    Result<int> getRetryCount(const QString& devName, const QString& appName,
                               const QString& role, const QString& pin);

signals:
    void loginStateChanged(const QString& devName, const QString& appName, bool loggedIn);
    void pinError(const QString& devName, const QString& appName, int retryCount);
    void pinLocked(const QString& devName, const QString& appName);

private:
    AppService();
    ~AppService() override = default;
};

}  // namespace wekey
