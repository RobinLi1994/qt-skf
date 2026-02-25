/**
 * @file CertService.h
 * @brief 证书与签名服务
 *
 * 封装证书管理和签名验签操作，委托给激活的驱动插件
 */

#pragma once

#include <QObject>

#include "common/Result.h"
#include "plugin/interface/PluginTypes.h"

namespace wekey {

class CertService : public QObject {
    Q_OBJECT

public:
    static CertService& instance();

    CertService(const CertService&) = delete;
    CertService& operator=(const CertService&) = delete;

    Result<QByteArray> generateKeyPair(const QString& devName, const QString& appName,
                                       const QString& containerName, const QString& keyType);
    Result<QByteArray> generateCsr(const QString& devName, const QString& appName, const QString& containerName,
                                    const QVariantMap& args);
    Result<void> importCert(const QString& devName, const QString& appName, const QString& containerName,
                            const QByteArray& certData, bool isSignCert);
    Result<void> importKeyCert(const QString& devName, const QString& appName, const QString& containerName,
                               const QByteArray& sigCert, const QByteArray& encCert,
                               const QByteArray& encPrivate, bool nonGM);
    Result<QByteArray> exportCert(const QString& devName, const QString& appName, const QString& containerName,
                                  bool isSignCert);
    Result<CertInfo> getCertInfo(const QString& devName, const QString& appName, const QString& containerName,
                                 bool isSignCert);
    Result<QByteArray> sign(const QString& devName, const QString& appName, const QString& containerName,
                            const QByteArray& data);
    Result<bool> verify(const QString& devName, const QString& appName, const QString& containerName,
                        const QByteArray& data, const QByteArray& signature);

private:
    CertService();
    ~CertService() override = default;
};

}  // namespace wekey
