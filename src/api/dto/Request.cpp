/**
 * @file Request.cpp
 * @brief API 请求 DTO 实现 (M4.1.4I)
 */

#include "Request.h"

namespace wekey {
namespace api {

// ==============================================================================
// 辅助函数
// ==============================================================================

static Result<void> requireNonEmpty(const QString& value, const QString& field, const QString& context) {
    if (value.isEmpty()) {
        return Result<void>::err(
            Error(Error::InvalidParam, QString("字段 '%1' 不能为空").arg(field), context));
    }
    return Result<void>::ok();
}

static Result<void> requireJsonField(const QJsonObject& json, const QString& field) {
    if (!json.contains(field) || json[field].toString().isEmpty()) {
        return Result<void>::err(
            Error(Error::InvalidParam, QString("缺少必填字段：%1").arg(field), "fromJson"));
    }
    return Result<void>::ok();
}

static Result<void> requireQueryField(const QMap<QString, QString>& query, const QString& field) {
    if (!query.contains(field) || query.value(field).isEmpty()) {
        return Result<void>::err(
            Error(Error::InvalidParam, QString("缺少必填字段：%1").arg(field), "fromQuery"));
    }
    return Result<void>::ok();
}

static constexpr int MAX_RANDOM_LENGTH = 4096;

// ==============================================================================
// LoginRequest
// ==============================================================================

Result<LoginRequest> LoginRequest::fromJson(const QJsonObject& json) {
    auto r = requireJsonField(json, "serialNumber");
    if (r.isErr()) return Result<LoginRequest>::err(r.error());
    r = requireJsonField(json, "pin");
    if (r.isErr()) return Result<LoginRequest>::err(r.error());

    LoginRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json.value("appName").toString();
    req.role = json.value("role").toString();
    req.pin = json["pin"].toString();
    return Result<LoginRequest>::ok(std::move(req));
}

Result<void> LoginRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "LoginRequest::validate");
    if (r.isErr()) return r;

    // appName 和 role 允许为空（将在 handler 中填充默认值）
    // 但如果 role 不为空，必须是合法值
    if (!role.isEmpty() && role != "user" && role != "admin") {
        return Result<void>::err(
            Error(Error::InvalidParam, "role 必须为 'user' 或 'admin'", "LoginRequest::validate"));
    }

    return requireNonEmpty(pin, "pin", "LoginRequest::validate");
}

// ==============================================================================
// LogoutRequest
// ==============================================================================

Result<LogoutRequest> LogoutRequest::fromJson(const QJsonObject& json) {
    auto r = requireJsonField(json, "serialNumber");
    if (r.isErr()) return Result<LogoutRequest>::err(r.error());

    LogoutRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json.value("appName").toString();
    return Result<LogoutRequest>::ok(std::move(req));
}

Result<void> LogoutRequest::validate() const {
    return requireNonEmpty(serialNumber, "serialNumber", "LogoutRequest::validate");
}

// ==============================================================================
// CsrRequest
// ==============================================================================

Result<CsrRequest> CsrRequest::fromJson(const QJsonObject& json) {
    auto r = requireJsonField(json, "serialNumber");
    if (r.isErr()) return Result<CsrRequest>::err(r.error());

    CsrRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json.value("appName").toString();
    req.containerName = json.value("containerName").toString();
    req.keyPairType = json.value("keyPairType").toString();
    req.renew = json.value("renew").toBool(false);
    req.cname = json.value("cname").toString();
    req.org = json.value("org").toString();
    req.unit = json.value("unit").toString();
    return Result<CsrRequest>::ok(std::move(req));
}

Result<void> CsrRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "CsrRequest::validate");
    if (r.isErr()) return r;
    // appName, containerName, cname, org, unit 在 handler 中填充默认值
    // 这里只校验填充默认值后的必填项
    r = requireNonEmpty(appName, "appName", "CsrRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(containerName, "containerName", "CsrRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(cname, "cname", "CsrRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(org, "org", "CsrRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(unit, "unit", "CsrRequest::validate");
    if (r.isErr()) return r;
    return Result<void>::ok();
}

// ==============================================================================
// ImportCertRequest
// ==============================================================================

Result<ImportCertRequest> ImportCertRequest::fromJson(const QJsonObject& json) {
    ImportCertRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.containerName = json["containerName"].toString();
    req.sigCert = json["sigCert"].toString();
    req.encCert = json["encCert"].toString();
    req.encPrivate = json["encPrivate"].toString();
    req.label = json["label"].toString();
    req.keyAlgorithm = json["keyAlgorithm"].toString();
    req.nonGM = json["nonGM"].toBool(false);
    return Result<ImportCertRequest>::ok(std::move(req));
}

Result<void> ImportCertRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "ImportCertRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "ImportCertRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(containerName, "containerName", "ImportCertRequest::validate");
    if (r.isErr()) return r;
    // sigCert、encCert、encPrivate 至少提供一个
    if (sigCert.isEmpty() && encCert.isEmpty() && encPrivate.isEmpty()) {
        return Result<void>::err(
            Error(Error::InvalidParam, "sigCert、encCert、encPrivate 至少需要提供一个",
                  "ImportCertRequest::validate"));
    }
    return Result<void>::ok();
}

// ==============================================================================
// ExportCertRequest
// ==============================================================================

Result<ExportCertRequest> ExportCertRequest::fromQuery(const QMap<QString, QString>& query) {
    auto r = requireQueryField(query, "serialNumber");
    if (r.isErr()) return Result<ExportCertRequest>::err(r.error());

    ExportCertRequest req;
    req.serialNumber = query.value("serialNumber");
    req.appName = query.value("appName");
    req.containerName = query.value("containerName");
    return Result<ExportCertRequest>::ok(std::move(req));
}

Result<void> ExportCertRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "ExportCertRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "ExportCertRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(containerName, "containerName", "ExportCertRequest::validate");
}

// ==============================================================================
// SignRequest
// ==============================================================================

Result<SignRequest> SignRequest::fromJson(const QJsonObject& json) {
    SignRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.containerName = json["containerName"].toString();
    req.data = json["data"].toString();
    return Result<SignRequest>::ok(std::move(req));
}

Result<void> SignRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "SignRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "SignRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(containerName, "containerName", "SignRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(data, "data", "SignRequest::validate");
}

// ==============================================================================
// VerifyRequest
// ==============================================================================

Result<VerifyRequest> VerifyRequest::fromJson(const QJsonObject& json) {
    VerifyRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.containerName = json["containerName"].toString();
    req.data = json["data"].toString();
    req.signature = json["signature"].toString();
    return Result<VerifyRequest>::ok(std::move(req));
}

Result<void> VerifyRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "VerifyRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "VerifyRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(containerName, "containerName", "VerifyRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(data, "data", "VerifyRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(signature, "signature", "VerifyRequest::validate");
}

// ==============================================================================
// RandomRequest
// ==============================================================================

Result<RandomRequest> RandomRequest::fromJson(const QJsonObject& json) {
    RandomRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.count = json["count"].toInt(0);
    return Result<RandomRequest>::ok(std::move(req));
}

Result<void> RandomRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "RandomRequest::validate");
    if (r.isErr()) return r;
    // count <= 0 时使用默认值，不需要校验
    if (count > MAX_RANDOM_LENGTH) {
        return Result<void>::err(
            Error(Error::InvalidParam, "count 不能超过 4096", "RandomRequest::validate"));
    }
    return Result<void>::ok();
}

// ==============================================================================
// CreateModuleRequest
// ==============================================================================

Result<CreateModuleRequest> CreateModuleRequest::fromJson(const QJsonObject& json) {
    CreateModuleRequest req;
    req.modName = json["modName"].toString();
    req.modPath = json["modPath"].toString();
    return Result<CreateModuleRequest>::ok(std::move(req));
}

Result<void> CreateModuleRequest::validate() const {
    auto r = requireNonEmpty(modName, "modName", "CreateModuleRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(modPath, "modPath", "CreateModuleRequest::validate");
}

// ==============================================================================
// ActiveModuleRequest
// ==============================================================================

Result<ActiveModuleRequest> ActiveModuleRequest::fromJson(const QJsonObject& json) {
    ActiveModuleRequest req;
    req.modName = json["modName"].toString();
    return Result<ActiveModuleRequest>::ok(std::move(req));
}

Result<void> ActiveModuleRequest::validate() const {
    return requireNonEmpty(modName, "modName", "ActiveModuleRequest::validate");
}

// ==============================================================================
// DeleteModuleRequest
// ==============================================================================

Result<DeleteModuleRequest> DeleteModuleRequest::fromJson(const QJsonObject& json) {
    DeleteModuleRequest req;
    req.modName = json["modName"].toString();
    return Result<DeleteModuleRequest>::ok(std::move(req));
}

Result<void> DeleteModuleRequest::validate() const {
    return requireNonEmpty(modName, "modName", "DeleteModuleRequest::validate");
}

// ==============================================================================
// ChangeDeviceAuthRequest
// ==============================================================================

Result<ChangeDeviceAuthRequest> ChangeDeviceAuthRequest::fromJson(const QJsonObject& json) {
    ChangeDeviceAuthRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.oldPin = json["oldPin"].toString();
    req.newPin = json["newPin"].toString();
    return Result<ChangeDeviceAuthRequest>::ok(std::move(req));
}

Result<void> ChangeDeviceAuthRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "ChangeDeviceAuthRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(oldPin, "oldPin", "ChangeDeviceAuthRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(newPin, "newPin", "ChangeDeviceAuthRequest::validate");
}

// ==============================================================================
// SetDeviceLabelRequest
// ==============================================================================

Result<SetDeviceLabelRequest> SetDeviceLabelRequest::fromJson(const QJsonObject& json) {
    SetDeviceLabelRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.label = json["label"].toString();
    return Result<SetDeviceLabelRequest>::ok(std::move(req));
}

Result<void> SetDeviceLabelRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "SetDeviceLabelRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(label, "label", "SetDeviceLabelRequest::validate");
}

// ==============================================================================
// CreateAppRequest
// ==============================================================================

Result<CreateAppRequest> CreateAppRequest::fromJson(const QJsonObject& json) {
    CreateAppRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.adminPin = json["adminPin"].toString();
    req.userPin = json["userPin"].toString();
    return Result<CreateAppRequest>::ok(std::move(req));
}

Result<void> CreateAppRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "CreateAppRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "CreateAppRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(adminPin, "adminPin", "CreateAppRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(userPin, "userPin", "CreateAppRequest::validate");
}

// ==============================================================================
// DeleteAppRequest
// ==============================================================================

Result<DeleteAppRequest> DeleteAppRequest::fromJson(const QJsonObject& json) {
    DeleteAppRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    return Result<DeleteAppRequest>::ok(std::move(req));
}

Result<void> DeleteAppRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "DeleteAppRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(appName, "appName", "DeleteAppRequest::validate");
}

// ==============================================================================
// UpdateAppPinRequest
// ==============================================================================

Result<UpdateAppPinRequest> UpdateAppPinRequest::fromJson(const QJsonObject& json) {
    UpdateAppPinRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.role = json["role"].toString();
    req.oldPin = json["oldPin"].toString();
    req.newPin = json["newPin"].toString();
    return Result<UpdateAppPinRequest>::ok(std::move(req));
}

Result<void> UpdateAppPinRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "UpdateAppPinRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "UpdateAppPinRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(role, "role", "UpdateAppPinRequest::validate");
    if (r.isErr()) return r;
    if (role != "user" && role != "admin") {
        return Result<void>::err(
            Error(Error::InvalidParam, "role 必须为 'user' 或 'admin'", "UpdateAppPinRequest::validate"));
    }
    r = requireNonEmpty(oldPin, "oldPin", "UpdateAppPinRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(newPin, "newPin", "UpdateAppPinRequest::validate");
}

// ==============================================================================
// UnblockAppRequest
// ==============================================================================

Result<UnblockAppRequest> UnblockAppRequest::fromJson(const QJsonObject& json) {
    UnblockAppRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.adminPin = json["adminPin"].toString();
    return Result<UnblockAppRequest>::ok(std::move(req));
}

Result<void> UnblockAppRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "UnblockAppRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "UnblockAppRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(adminPin, "adminPin", "UnblockAppRequest::validate");
}

// ==============================================================================
// CreateContainerRequest
// ==============================================================================

Result<CreateContainerRequest> CreateContainerRequest::fromJson(const QJsonObject& json) {
    CreateContainerRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.containerName = json["containerName"].toString();
    return Result<CreateContainerRequest>::ok(std::move(req));
}

Result<void> CreateContainerRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "CreateContainerRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "CreateContainerRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(containerName, "containerName", "CreateContainerRequest::validate");
}

// ==============================================================================
// DeleteContainerRequest
// ==============================================================================

Result<DeleteContainerRequest> DeleteContainerRequest::fromJson(const QJsonObject& json) {
    DeleteContainerRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.containerName = json["containerName"].toString();
    return Result<DeleteContainerRequest>::ok(std::move(req));
}

Result<void> DeleteContainerRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "DeleteContainerRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "DeleteContainerRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(containerName, "containerName", "DeleteContainerRequest::validate");
}

// ==============================================================================
// CreateFileRequest
// ==============================================================================

Result<CreateFileRequest> CreateFileRequest::fromJson(const QJsonObject& json) {
    CreateFileRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.fileName = json["fileName"].toString();
    req.size = json["size"].toInt(0);
    return Result<CreateFileRequest>::ok(std::move(req));
}

Result<void> CreateFileRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "CreateFileRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "CreateFileRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(fileName, "fileName", "CreateFileRequest::validate");
    if (r.isErr()) return r;
    if (size <= 0) {
        return Result<void>::err(
            Error(Error::InvalidParam, "size 必须大于 0", "CreateFileRequest::validate"));
    }
    return Result<void>::ok();
}

// ==============================================================================
// ReadFileRequest
// ==============================================================================

Result<ReadFileRequest> ReadFileRequest::fromQuery(const QMap<QString, QString>& query) {
    auto r = requireQueryField(query, "serialNumber");
    if (r.isErr()) return Result<ReadFileRequest>::err(r.error());
    r = requireQueryField(query, "appName");
    if (r.isErr()) return Result<ReadFileRequest>::err(r.error());
    r = requireQueryField(query, "fileName");
    if (r.isErr()) return Result<ReadFileRequest>::err(r.error());

    ReadFileRequest req;
    req.serialNumber = query.value("serialNumber");
    req.appName = query.value("appName");
    req.fileName = query.value("fileName");
    return Result<ReadFileRequest>::ok(std::move(req));
}

Result<void> ReadFileRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "ReadFileRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "ReadFileRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(fileName, "fileName", "ReadFileRequest::validate");
}

// ==============================================================================
// DeleteFileRequest
// ==============================================================================

Result<DeleteFileRequest> DeleteFileRequest::fromJson(const QJsonObject& json) {
    DeleteFileRequest req;
    req.serialNumber = json["serialNumber"].toString();
    req.appName = json["appName"].toString();
    req.fileName = json["fileName"].toString();
    return Result<DeleteFileRequest>::ok(std::move(req));
}

Result<void> DeleteFileRequest::validate() const {
    auto r = requireNonEmpty(serialNumber, "serialNumber", "DeleteFileRequest::validate");
    if (r.isErr()) return r;
    r = requireNonEmpty(appName, "appName", "DeleteFileRequest::validate");
    if (r.isErr()) return r;
    return requireNonEmpty(fileName, "fileName", "DeleteFileRequest::validate");
}

// ==============================================================================
// SetDefaultsRequest
// ==============================================================================

Result<SetDefaultsRequest> SetDefaultsRequest::fromJson(const QJsonObject& json) {
    SetDefaultsRequest req;
    req.appName = json["appName"].toString();
    req.containerName = json["containerName"].toString();
    req.commonName = json["commonName"].toString();
    req.organization = json["organization"].toString();
    req.unit = json["unit"].toString();
    req.role = json["role"].toString();
    return Result<SetDefaultsRequest>::ok(std::move(req));
}

Result<void> SetDefaultsRequest::validate() const {
    // SetDefaults 所有字段都是可选的，至少有一个非空即可
    return Result<void>::ok();
}

}  // namespace api
}  // namespace wekey
