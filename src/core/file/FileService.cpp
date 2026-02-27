/**
 * @file FileService.cpp
 * @brief 文件与随机数服务实现
 */

#include "FileService.h"

#include "plugin/PluginManager.h"

namespace wekey {

FileService& FileService::instance() {
    static FileService instance;
    return instance;
}

FileService::FileService() : QObject(nullptr) {}

Result<QStringList> FileService::enumFiles(const QString& devName, const QString& appName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QStringList>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "FileService::enumFiles"));
    }
    return plugin->enumFiles(devName, appName);
}

Result<QByteArray> FileService::readFile(const QString& devName, const QString& appName, const QString& fileName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QByteArray>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "FileService::readFile"));
    }
    return plugin->readFile(devName, appName, fileName);
}

Result<void> FileService::writeFile(const QString& devName, const QString& appName, const QString& fileName,
                                     const QByteArray& data, int readRights, int writeRights) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "FileService::writeFile"));
    }
    return plugin->writeFile(devName, appName, fileName, data, readRights, writeRights);
}

Result<void> FileService::deleteFile(const QString& devName, const QString& appName, const QString& fileName) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "FileService::deleteFile"));
    }
    return plugin->deleteFile(devName, appName, fileName);
}

Result<QByteArray> FileService::generateRandom(const QString& devName, int count) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QByteArray>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "FileService::generateRandom"));
    }
    return plugin->generateRandom(devName, count);
}

}  // namespace wekey
