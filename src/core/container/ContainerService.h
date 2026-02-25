/**
 * @file ContainerService.h
 * @brief 容器服务
 *
 * 封装容器管理操作，委托给激活的驱动插件
 */

#pragma once

#include <QObject>

#include "common/Result.h"
#include "plugin/interface/PluginTypes.h"

namespace wekey {

class ContainerService : public QObject {
    Q_OBJECT

public:
    static ContainerService& instance();

    ContainerService(const ContainerService&) = delete;
    ContainerService& operator=(const ContainerService&) = delete;

    Result<QList<ContainerInfo>> enumContainers(const QString& devName, const QString& appName);
    Result<void> createContainer(const QString& devName, const QString& appName, const QString& containerName);
    Result<void> deleteContainer(const QString& devName, const QString& appName, const QString& containerName);

private:
    ContainerService();
    ~ContainerService() override = default;
};

}  // namespace wekey
