/**
 * @file Response.cpp
 * @brief API 响应 DTO 实现 (M4.1.3I)
 */

#include "Response.h"

namespace wekey {
namespace api {

// ==============================================================================
// DeviceInfo 转换
// ==============================================================================

QJsonObject deviceInfoToJson(const DeviceInfo& info) {
    QJsonObject obj;
    obj["deviceName"] = info.deviceName;
    obj["serialNumber"] = info.serialNumber;
    obj["manufacturer"] = info.manufacturer;
    obj["label"] = info.label;
    obj["hwVersion"] = info.hardwareVersion;
    obj["firmwareVersion"] = info.firmwareVersion;
    obj["isLogin"] = info.isLoggedIn;
    return obj;
}

QJsonArray deviceInfoListToJson(const QList<DeviceInfo>& devices) {
    QJsonArray arr;
    for (const auto& device : devices) {
        arr.append(deviceInfoToJson(device));
    }
    return arr;
}

// ==============================================================================
// AppInfo 转换
// ==============================================================================

QJsonObject appInfoToJson(const AppInfo& info) {
    QJsonObject obj;
    obj["appName"] = info.appName;
    obj["isLogin"] = info.isLoggedIn;
    return obj;
}

QJsonArray appInfoListToJson(const QList<AppInfo>& apps) {
    QJsonArray arr;
    for (const auto& app : apps) {
        arr.append(appInfoToJson(app));
    }
    return arr;
}

// ==============================================================================
// ContainerInfo 转换
// ==============================================================================

QJsonObject containerInfoToJson(const ContainerInfo& info) {
    QJsonObject obj;
    obj["containerName"] = info.containerName;
    obj["keyGenerated"] = info.keyGenerated;
    obj["keyType"] = static_cast<int>(info.keyType);
    obj["certImported"] = info.certImported;
    return obj;
}

QJsonArray containerInfoListToJson(const QList<ContainerInfo>& containers) {
    QJsonArray arr;
    for (const auto& container : containers) {
        arr.append(containerInfoToJson(container));
    }
    return arr;
}

// ==============================================================================
// CertInfo 转换
// ==============================================================================

QJsonObject certInfoToJson(const CertInfo& info) {
    QJsonObject obj;
    obj["subjectDn"] = info.subjectDn;
    obj["commonName"] = info.commonName;
    obj["issuerDn"] = info.issuerDn;
    obj["serialNumber"] = info.serialNumber;
    obj["certType"] = info.certType;
    obj["pubKeyHash"] = info.pubKeyHash;
    obj["cert"] = info.cert;

    // validity 数组 [开始时间, 结束时间]
    QJsonArray validity;
    validity.append(info.notBefore.toString("yyyy-MM-dd HH:mm:ss"));
    validity.append(info.notAfter.toString("yyyy-MM-dd HH:mm:ss"));
    obj["validity"] = validity;

    return obj;
}

QJsonArray certInfoListToJson(const QList<CertInfo>& certs) {
    QJsonArray arr;
    for (const auto& cert : certs) {
        arr.append(certInfoToJson(cert));
    }
    return arr;
}

}  // namespace api
}  // namespace wekey
