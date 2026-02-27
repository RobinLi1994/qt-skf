/**
 * @file ContainerService.cpp
 * @brief 容器服务实现
 */

#include "ContainerService.h"

#include "plugin/PluginManager.h"

namespace wekey {

ContainerService& ContainerService::instance() {
    static ContainerService instance;
    return instance;
}

ContainerService::ContainerService() : QObject(nullptr) {}

Result<QList<ContainerInfo>> ContainerService::enumContainers(const QString& devName, const QString& appName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QList<ContainerInfo>>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "ContainerService::enumContainers"));
    }
    return plugin->enumContainers(devName, appName);
}

Result<void> ContainerService::createContainer(const QString& devName, const QString& appName,
                                                const QString& containerName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "ContainerService::createContainer"));
    }
    return plugin->createContainer(devName, appName, containerName);
}

Result<void> ContainerService::deleteContainer(const QString& devName, const QString& appName,
                                                const QString& containerName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "ContainerService::deleteContainer"));
    }
    return plugin->deleteContainer(devName, appName, containerName);
}

}  // namespace wekey
