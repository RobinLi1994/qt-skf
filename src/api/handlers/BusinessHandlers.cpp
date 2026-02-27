/**
 * @file BusinessHandlers.cpp
 * @brief 业务接口处理器实现 (M4.5)
 */

#include "BusinessHandlers.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "api/dto/HttpTypes.h"
#include "api/dto/Request.h"
#include "api/dto/Response.h"
#include "config/Config.h"
#include "core/application/AppService.h"
#include "core/container/ContainerService.h"
#include "core/crypto/CertService.h"
#include "core/device/DeviceService.h"
#include "core/file/FileService.h"

namespace wekey {
namespace api {

HttpResponse BusinessHandlers::handleEnumDev(const HttpRequest& /*request*/) {
    auto result = DeviceService::instance().enumDevices(false, false);
    HttpResponse resp;
    if (result.isErr()) {
        resp.setError(result.error());
        return resp;
    }
    resp.setSuccess(QJsonValue(deviceInfoListToJson(result.value())));
    return resp;
}

HttpResponse BusinessHandlers::handleLogin(const HttpRequest& request) {
    auto jsonResult = request.jsonBody();
    if (jsonResult.isErr()) {
        HttpResponse resp;
        resp.setError(jsonResult.error());
        return resp;
    }

    auto reqResult = LoginRequest::fromJson(jsonResult.value());
    if (reqResult.isErr()) {
        HttpResponse resp;
        resp.setError(reqResult.error());
        return resp;
    }

    auto& req = reqResult.value();

    // 填充默认值
    if (req.appName.isEmpty()) {
        req.appName = Config::instance().defaultAppName();
    }
    if (req.role.isEmpty()) {
        req.role = Config::instance().defaultRole();
    }

    auto valResult = req.validate();
    if (valResult.isErr()) {
        HttpResponse resp;
        resp.setError(valResult.error());
        return resp;
    }

    auto result = AppService::instance().login(req.serialNumber, req.appName, req.role, req.pin, false);

    HttpResponse resp;
    if (result.isErr()) {
        resp.setError(result.error());
    } else {
        resp.setSuccess();
    }
    return resp;
}

HttpResponse BusinessHandlers::handleLogout(const HttpRequest& request) {
    auto jsonResult = request.jsonBody();
    if (jsonResult.isErr()) {
        HttpResponse resp;
        resp.setError(jsonResult.error());
        return resp;
    }

    auto reqResult = LogoutRequest::fromJson(jsonResult.value());
    if (reqResult.isErr()) {
        HttpResponse resp;
        resp.setError(reqResult.error());
        return resp;
    }

    auto& req = reqResult.value();

    // 填充默认值
    if (req.appName.isEmpty()) {
        req.appName = Config::instance().defaultAppName();
    }

    auto valResult = req.validate();
    if (valResult.isErr()) {
        HttpResponse resp;
        resp.setError(valResult.error());
        return resp;
    }

    auto result = AppService::instance().logout(req.serialNumber, req.appName, false);

    HttpResponse resp;
    if (result.isErr()) {
        resp.setError(result.error());
    } else {
        resp.setSuccess();
    }
    return resp;
}

HttpResponse BusinessHandlers::handleGenCsr(const HttpRequest& request) {
    auto jsonResult = request.jsonBody();
    if (jsonResult.isErr()) {
        HttpResponse resp;
        resp.setError(jsonResult.error());
        return resp;
    }

    auto reqResult = CsrRequest::fromJson(jsonResult.value());
    if (reqResult.isErr()) {
        HttpResponse resp;
        resp.setError(reqResult.error());
        return resp;
    }

    auto& req = reqResult.value();

    // 填充默认值（与 Go 逻辑一致）
    if (req.appName.isEmpty()) {
        req.appName = Config::instance().defaultAppName();
    }
    if (req.containerName.isEmpty()) {
        req.containerName = Config::instance().defaultContainerName();
    }
    if (req.cname.isEmpty()) {
        req.cname = Config::instance().defaultCommonName();
    }
    if (req.org.isEmpty()) {
        req.org = Config::instance().defaultOrganization();
    }
    if (req.unit.isEmpty()) {
        req.unit = Config::instance().defaultUnit();
    }

    auto valResult = req.validate();
    if (valResult.isErr()) {
        HttpResponse resp;
        resp.setError(valResult.error());
        return resp;
    }

    // 检查容器是否存在，不存在则自动创建（与 Go 逻辑一致）
    auto containers = ContainerService::instance().enumContainers(req.serialNumber, req.appName);
    if (containers.isOk()) {
        bool found = false;
        for (const auto& c : containers.value()) {
            if (c.containerName == req.containerName) {
                found = true;
                break;
            }
        }
        if (!found) {
            auto createResult = ContainerService::instance().createContainer(
                req.serialNumber, req.appName, req.containerName);
            if (createResult.isErr()) {
                HttpResponse resp;
                resp.setError(createResult.error());
                return resp;
            }
        }
    }

    // 构建 generateCsr 参数
    QVariantMap args;
    args["renewKey"] = req.renew;
    args["cname"] = req.cname;
    args["org"] = req.org;
    args["unit"] = req.unit;

    // 解析 keyPairType -> keyType + keySize
    if (req.keyPairType.startsWith("RSA")) {
        args["keyType"] = "RSA";
        if (req.keyPairType.contains("4096")) {
            args["keySize"] = 4096;
        } else if (req.keyPairType.contains("3072")) {
            args["keySize"] = 3072;
        } else {
            args["keySize"] = 2048;
        }
    } else {
        // SM2_sm2p256v1 等
        args["keyType"] = "SM2";
    }

    // 生成 CSR
    auto result = CertService::instance().generateCsr(
        req.serialNumber, req.appName, req.containerName, args);

    HttpResponse resp;
    if (result.isErr()) {
        resp.setError(result.error());
        return resp;
    }

    // 将 DER 编码的 CSR 转换为 PEM 格式（与 Go 逻辑一致）
    QString pemBody = QString::fromLatin1(result.value().toBase64());
    // 每 64 字符换行
    QString formattedPem;
    for (int i = 0; i < pemBody.size(); i += 64) {
        formattedPem += pemBody.mid(i, 64) + "\n";
    }
    QString csrPem = "-----BEGIN CERTIFICATE REQUEST-----\n" + formattedPem + "-----END CERTIFICATE REQUEST-----\n";

    QJsonObject data;
    data["csr"] = csrPem;
    resp.setSuccess(data);
    return resp;
}

HttpResponse BusinessHandlers::handleImportCert(const HttpRequest& request) {
    auto jsonResult = request.jsonBody();
    if (jsonResult.isErr()) {
        HttpResponse resp;
        resp.setError(jsonResult.error());
        return resp;
    }

    auto reqResult = ImportCertRequest::fromJson(jsonResult.value());
    if (reqResult.isErr()) {
        HttpResponse resp;
        resp.setError(reqResult.error());
        return resp;
    }

    auto& req = reqResult.value();

    // 填充默认值
    if (req.appName.isEmpty()) {
        req.appName = Config::instance().defaultAppName();
    }
    if (req.containerName.isEmpty()) {
        req.containerName = Config::instance().defaultContainerName();
    }

    auto valResult = req.validate();
    if (valResult.isErr()) {
        HttpResponse resp;
        resp.setError(valResult.error());
        return resp;
    }

    qDebug() << "[handleImportCert] serialNumber:" << req.serialNumber
             << "appName:" << req.appName << "containerName:" << req.containerName
             << "nonGM:" << req.nonGM
             << "sigCert empty:" << req.sigCert.isEmpty()
             << "encCert empty:" << req.encCert.isEmpty()
             << "encPrivate empty:" << req.encPrivate.isEmpty();

    // 解析证书和私钥数据（PEM/Base64 → 二进制）
    QByteArray sigCertBytes;
    QByteArray encCertBytes;
    QByteArray encPrivateBytes;

    // 解析签名证书（支持 PEM 和 Base64 DER）
    if (!req.sigCert.isEmpty()) {
        QString trimmed = req.sigCert.trimmed();
        if (trimmed.startsWith("-----BEGIN")) {
            // PEM 格式：提取 Base64 内容并解码
            QStringList lines = trimmed.split('\n');
            QString base64Content;
            for (const auto& line : lines) {
                QString l = line.trimmed();
                if (!l.startsWith("-----")) {
                    base64Content += l;
                }
            }
            sigCertBytes = QByteArray::fromBase64(base64Content.toLatin1());
        } else {
            // 纯 Base64 DER 编码
            sigCertBytes = QByteArray::fromBase64(trimmed.toLatin1());
        }
        if (sigCertBytes.isEmpty()) {
            HttpResponse resp;
            resp.setError(Error(Error::InvalidParam, "签名证书解码失败", "handleImportCert"));
            return resp;
        }
        qDebug() << "[handleImportCert] sigCert decoded, size:" << sigCertBytes.size();
    }

    // 解析加密证书（同签名证书格式）
    if (!req.encCert.isEmpty()) {
        QString trimmed = req.encCert.trimmed();
        if (trimmed.startsWith("-----BEGIN")) {
            QStringList lines = trimmed.split('\n');
            QString base64Content;
            for (const auto& line : lines) {
                QString l = line.trimmed();
                if (!l.startsWith("-----")) {
                    base64Content += l;
                }
            }
            encCertBytes = QByteArray::fromBase64(base64Content.toLatin1());
        } else {
            encCertBytes = QByteArray::fromBase64(trimmed.toLatin1());
        }
        if (encCertBytes.isEmpty()) {
            HttpResponse resp;
            resp.setError(Error(Error::InvalidParam, "加密证书解码失败", "handleImportCert"));
            return resp;
        }
        qDebug() << "[handleImportCert] encCert decoded, size:" << encCertBytes.size();
    }

    // 解析加密私钥（Base64 编码的原始二进制数据）
    if (!req.encPrivate.isEmpty()) {
        encPrivateBytes = QByteArray::fromBase64(req.encPrivate.trimmed().toLatin1());
        if (encPrivateBytes.isEmpty()) {
            HttpResponse resp;
            resp.setError(Error(Error::InvalidParam, "加密私钥解码失败", "handleImportCert"));
            return resp;
        }
        qDebug() << "[handleImportCert] encPrivate decoded, size:" << encPrivateBytes.size();
    }

    // 统一调用 importKeyCert：在单个设备/容器会话中完成所有导入
    // 插件内部会检测容器密钥类型决定 RSA/SM2 私钥导入方式
    auto result = CertService::instance().importKeyCert(
        req.serialNumber, req.appName, req.containerName,
        sigCertBytes, encCertBytes, encPrivateBytes, req.nonGM);

    HttpResponse resp;
    if (result.isErr()) {
        resp.setError(result.error());
    } else {
        resp.setSuccess();
    }
    return resp;
}

HttpResponse BusinessHandlers::handleExportCert(const HttpRequest& request) {
    auto reqResult = ExportCertRequest::fromQuery(request.queryParams);
    if (reqResult.isErr()) {
        HttpResponse resp;
        resp.setError(reqResult.error());
        return resp;
    }

    auto& req = reqResult.value();

    // 填充默认值
    if (req.appName.isEmpty()) {
        req.appName = Config::instance().defaultAppName();
    }
    if (req.containerName.isEmpty()) {
        req.containerName = Config::instance().defaultContainerName();
    }

    auto valResult = req.validate();
    if (valResult.isErr()) {
        HttpResponse resp;
        resp.setError(valResult.error());
        return resp;
    }

    // 获取签名证书 (certType=0) 和加密证书 (certType=1)
    auto signCertResult = CertService::instance().getCertInfo(
        req.serialNumber, req.appName, req.containerName, true);  // 签名证书
    auto encCertResult = CertService::instance().getCertInfo(
        req.serialNumber, req.appName, req.containerName, false); // 加密证书

    // 构建证书数组，使用 Response.cpp 中的 certInfoToJson 工具函数
    QJsonArray dataArray;

    // 添加签名证书 (certType=0 在前)
    if (signCertResult.isOk()) {
        dataArray.append(certInfoToJson(signCertResult.value()));
    }

    // 添加加密证书 (certType=1 在后)
    if (encCertResult.isOk()) {
        dataArray.append(certInfoToJson(encCertResult.value()));
    }

    HttpResponse resp;
    if (dataArray.isEmpty()) {
        // 两个证书都获取失败，返回签名证书的错误
        auto& err = signCertResult.isErr() ? signCertResult.error() : encCertResult.error();
        resp.setError(err);
    } else {
        resp.setSuccess(QJsonValue(dataArray));
    }
    return resp;
}

HttpResponse BusinessHandlers::handleSign(const HttpRequest& request) {
    auto jsonResult = request.jsonBody();
    if (jsonResult.isErr()) {
        HttpResponse resp;
        resp.setError(jsonResult.error());
        return resp;
    }

    auto reqResult = SignRequest::fromJson(jsonResult.value());
    if (reqResult.isErr()) {
        HttpResponse resp;
        resp.setError(reqResult.error());
        return resp;
    }

    auto& req = reqResult.value();

    // 填充默认值
    if (req.appName.isEmpty()) {
        req.appName = Config::instance().defaultAppName();
    }
    if (req.containerName.isEmpty()) {
        req.containerName = Config::instance().defaultContainerName();
    }

    auto valResult = req.validate();
    if (valResult.isErr()) {
        HttpResponse resp;
        resp.setError(valResult.error());
        return resp;
    }

    auto data = req.data.toUtf8();
    // 签名算法由插件根据容器密钥类型自动选择（SM2→SM3, RSA→SHA256）
    auto result = CertService::instance().sign(
        req.serialNumber, req.appName, req.containerName, data);

    HttpResponse resp;
    if (result.isErr()) {
        resp.setError(result.error());
    } else {
        resp.setSuccess(QJsonValue(QString::fromLatin1(result.value().toBase64())));
    }
    return resp;
}


HttpResponse BusinessHandlers::handleRandom(const HttpRequest& request) {
    auto jsonResult = request.jsonBody();
    if (jsonResult.isErr()) {
        HttpResponse resp;
        resp.setError(jsonResult.error());
        return resp;
    }

    auto reqResult = RandomRequest::fromJson(jsonResult.value());
    if (reqResult.isErr()) {
        HttpResponse resp;
        resp.setError(reqResult.error());
        return resp;
    }

    auto& req = reqResult.value();
    auto valResult = req.validate();
    if (valResult.isErr()) {
        HttpResponse resp;
        resp.setError(valResult.error());
        return resp;
    }

    // count <= 0 时使用默认值
    int count = req.count;
    if (count <= 0) {
        count = Config::instance().defaultRandomLength();
    }

    auto result = FileService::instance().generateRandom(req.serialNumber, count);

    HttpResponse resp;
    if (result.isErr()) {
        resp.setError(result.error());
    } else {
        QJsonObject data;
        data["randomNum"] = QString::fromLatin1(result.value().toHex());
        resp.setSuccess(data);
    }
    return resp;
}

}  // namespace api
}  // namespace wekey
