/**
 * @file AppService.cpp
 * @brief 应用服务实现
 */

#include "AppService.h"

#include "plugin/PluginManager.h"

namespace wekey {

AppService& AppService::instance() {
    static AppService instance;
    return instance;
}

AppService::AppService() : QObject(nullptr) {}

Result<QList<AppInfo>> AppService::enumApps(const QString& devName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QList<AppInfo>>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::enumApps"));
    }
    return plugin->enumApps(devName);
}

Result<void> AppService::createApp(const QString& devName, const QString& appName, const QVariantMap& args) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::createApp"));
    }
    return plugin->createApp(devName, appName, args);
}

Result<void> AppService::deleteApp(const QString& devName, const QString& appName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::deleteApp"));
    }
    return plugin->deleteApp(devName, appName);
}

Result<void> AppService::login(const QString& devName, const QString& appName, const QString& role,
                                const QString& pin, bool emitSignals) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::login"));
    }

    auto result = plugin->openApp(devName, appName, role, pin);
    if (result.isOk()) {
        if (emitSignals) {
            emit loginStateChanged(devName, appName, true);
        }
    } else {
        auto code = result.error().code();
        if (code == Error::SkfPinIncorrect) {
            auto retryResult = plugin->getRetryCount(devName, appName, role, pin);
            int retryCount = retryResult.isOk() ? retryResult.value() : -1;
            if (emitSignals) {
                emit pinError(devName, appName, retryCount);
            }
        } else if (code == Error::SkfPinLocked && emitSignals) {
            emit pinLocked(devName, appName);
        }
    }
    return result;
}

Result<void> AppService::logout(const QString& devName, const QString& appName, bool emitSignals) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::logout"));
    }
    auto result = plugin->closeApp(devName, appName);
    if (result.isOk() && emitSignals) {
        emit loginStateChanged(devName, appName, false);
    }
    return result;
}

Result<void> AppService::changePin(const QString& devName, const QString& appName, const QString& role,
                                    const QString& oldPin, const QString& newPin) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::changePin"));
    }
    return plugin->changePin(devName, appName, role, oldPin, newPin);
}

Result<void> AppService::unlockPin(const QString& devName, const QString& appName, const QString& adminPin,
                                    const QString& newUserPin, const QVariantMap& args) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::unlockPin"));
    }
    return plugin->unlockPin(devName, appName, adminPin, newUserPin, args);
}

Result<int> AppService::getRetryCount(const QString& devName, const QString& appName,
                                       const QString& role, const QString& pin) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<int>::err(
            Error(Error::NoActiveModule, "No active plugin", "AppService::getRetryCount"));
    }
    return plugin->getRetryCount(devName, appName, role, pin);
}

}  // namespace wekey
