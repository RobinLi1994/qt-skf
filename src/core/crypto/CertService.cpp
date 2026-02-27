/**
 * @file CertService.cpp
 * @brief 证书与签名服务实现
 */

#include "CertService.h"

#include "plugin/PluginManager.h"

namespace wekey {

CertService& CertService::instance() {
    static CertService instance;
    return instance;
}

CertService::CertService() : QObject(nullptr) {}

Result<QByteArray> CertService::generateKeyPair(const QString& devName, const QString& appName,
                                                 const QString& containerName, const QString& keyType) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QByteArray>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::generateKeyPair"));
    }
    return plugin->generateKeyPair(devName, appName, containerName, keyType);
}

Result<QByteArray> CertService::generateCsr(const QString& devName, const QString& appName,
                                              const QString& containerName, const QVariantMap& args) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QByteArray>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::generateCsr"));
    }
    return plugin->generateCsr(devName, appName, containerName, args);
}

Result<void> CertService::importCert(const QString& devName, const QString& appName, const QString& containerName,
                                      const QByteArray& certData, bool isSignCert) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::importCert"));
    }
    return plugin->importCert(devName, appName, containerName, certData, isSignCert);
}

Result<void> CertService::importKeyCert(const QString& devName, const QString& appName, const QString& containerName,
                                         const QByteArray& sigCert, const QByteArray& encCert,
                                         const QByteArray& encPrivate, bool nonGM) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<void>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::importKeyCert"));
    }
    return plugin->importKeyCert(devName, appName, containerName, sigCert, encCert, encPrivate, nonGM);
}

Result<QByteArray> CertService::exportCert(const QString& devName, const QString& appName,
                                            const QString& containerName, bool isSignCert) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QByteArray>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::exportCert"));
    }
    return plugin->exportCert(devName, appName, containerName, isSignCert);
}

Result<CertInfo> CertService::getCertInfo(const QString& devName, const QString& appName,
                                           const QString& containerName, bool isSignCert) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<CertInfo>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::getCertInfo"));
    }
    return plugin->getCertInfo(devName, appName, containerName, isSignCert);
}

Result<QByteArray> CertService::sign(const QString& devName, const QString& appName, const QString& containerName,
                                      const QByteArray& data) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<QByteArray>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::sign"));
    }
    return plugin->sign(devName, appName, containerName, data);
}

Result<bool> CertService::verify(const QString& devName, const QString& appName, const QString& containerName,
                                  const QByteArray& data, const QByteArray& signature) {
    auto* plugin = PluginManager::instance().activePlugin();
    if (!plugin) {
        return Result<bool>::err(
            Error(Error::NoActiveModule, "驱动模块未激活", "CertService::verify"));
    }
    return plugin->verify(devName, appName, containerName, data, signature);
}

}  // namespace wekey
