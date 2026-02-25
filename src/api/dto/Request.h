/**
 * @file Request.h
 * @brief API 请求 DTO 定义 (M4.1.4I)
 *
 * 定义所有 API 请求的数据传输对象
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QString>

#include "common/Error.h"
#include "common/Result.h"

namespace wekey {
namespace api {

// ==============================================================================
// 业务接口请求 DTO
// ==============================================================================

/**
 * @brief 登录请求
 * POST /api/v1/login
 */
struct LoginRequest {
    QString serialNumber;
    QString appName;
    QString role;  // "user" 或 "admin"
    QString pin;

    static Result<LoginRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 登出请求
 * POST /api/v1/logout
 */
struct LogoutRequest {
    QString serialNumber;
    QString appName;  // 可选，为空时使用默认值

    static Result<LogoutRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 生成 CSR 请求
 * POST /api/v1/csr
 */
struct CsrRequest {
    QString serialNumber;
    QString appName;
    QString containerName;
    QString keyPairType;  // "RSA_2048", "SM2_sm2p256v1" 等
    bool renew = false;   // 是否重新生成密钥对
    QString cname;        // Common Name
    QString org;          // Organization
    QString unit;         // Organizational Unit

    static Result<CsrRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 导入证书及密钥请求
 * POST /api/v1/import-cert
 *
 * 对应 Go 的 onImportKeyCert 接口，支持导入签名证书、加密证书、加密私钥
 */
struct ImportCertRequest {
    QString serialNumber;
    QString appName;
    QString containerName;
    QString sigCert;             // 签名证书（PEM/Base64）
    QString encCert;             // 加密证书（PEM/Base64）
    QString encPrivate;          // 加密私钥（Base64 编码的原始数据）
    QString label;               // 设备标签（可选）
    QString keyAlgorithm;        // 密钥算法（可选）
    bool nonGM = false;          // 是否非国密（影响私钥导入方式）

    static Result<ImportCertRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 导出证书请求
 * GET /api/v1/export-cert
 */
struct ExportCertRequest {
    QString serialNumber;
    QString appName;
    QString containerName;

    static Result<ExportCertRequest> fromQuery(const QMap<QString, QString>& query);
    Result<void> validate() const;
};

/**
 * @brief 签名请求
 * POST /api/v1/sign
 */
struct SignRequest {
    QString serialNumber;
    QString appName;
    QString containerName;
    QString data;

    static Result<SignRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 验签请求
 * POST /api/v1/verify
 */
struct VerifyRequest {
    QString serialNumber;
    QString appName;
    QString containerName;
    QString data;       // base64 编码的原始数据
    QString signature;  // hex 编码的签名

    static Result<VerifyRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 生成随机数请求
 * POST /api/v1/random
 */
struct RandomRequest {
    QString serialNumber;
    int count = 0;  // 随机数长度（字节），<= 0 时使用默认值

    static Result<RandomRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

// ==============================================================================
// 管理接口请求 DTO - 模块管理
// ==============================================================================

/**
 * @brief 创建模块请求
 * POST /admin/mod/create
 */
struct CreateModuleRequest {
    QString modName;
    QString modPath;

    static Result<CreateModuleRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 激活模块请求
 * POST /admin/mod/active
 */
struct ActiveModuleRequest {
    QString modName;

    static Result<ActiveModuleRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 删除模块请求
 * DELETE /admin/mod/delete
 */
struct DeleteModuleRequest {
    QString modName;

    static Result<DeleteModuleRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

// ==============================================================================
// 管理接口请求 DTO - 设备管理
// ==============================================================================

/**
 * @brief 修改设备认证密钥请求
 * POST /admin/dev/change-auth
 */
struct ChangeDeviceAuthRequest {
    QString serialNumber;
    QString oldPin;
    QString newPin;

    static Result<ChangeDeviceAuthRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 设置设备标签请求
 * POST /admin/dev/set-label
 */
struct SetDeviceLabelRequest {
    QString serialNumber;
    QString label;

    static Result<SetDeviceLabelRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

// ==============================================================================
// 管理接口请求 DTO - 应用管理
// ==============================================================================

/**
 * @brief 创建应用请求
 * POST /admin/app/create
 */
struct CreateAppRequest {
    QString serialNumber;
    QString appName;
    QString adminPin;
    QString userPin;

    static Result<CreateAppRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 删除应用请求
 * DELETE /admin/app/delete
 */
struct DeleteAppRequest {
    QString serialNumber;
    QString appName;

    static Result<DeleteAppRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 修改应用 PIN 请求
 * POST /admin/app/update-pin
 */
struct UpdateAppPinRequest {
    QString serialNumber;
    QString appName;
    QString role;  // "user" 或 "admin"
    QString oldPin;
    QString newPin;

    static Result<UpdateAppPinRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 解锁应用请求
 * POST /admin/app/unblock
 */
struct UnblockAppRequest {
    QString serialNumber;
    QString appName;
    QString adminPin;

    static Result<UnblockAppRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

// ==============================================================================
// 管理接口请求 DTO - 容器管理
// ==============================================================================

/**
 * @brief 创建容器请求
 * POST /admin/container/create
 */
struct CreateContainerRequest {
    QString serialNumber;
    QString appName;
    QString containerName;

    static Result<CreateContainerRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 删除容器请求
 * DELETE /admin/container/delete
 */
struct DeleteContainerRequest {
    QString serialNumber;
    QString appName;
    QString containerName;

    static Result<DeleteContainerRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

// ==============================================================================
// 管理接口请求 DTO - 文件管理
// ==============================================================================

/**
 * @brief 创建文件请求
 * POST /admin/file/create
 */
struct CreateFileRequest {
    QString serialNumber;
    QString appName;
    QString fileName;
    int size;

    static Result<CreateFileRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

/**
 * @brief 读取文件请求
 * GET /admin/file/read
 */
struct ReadFileRequest {
    QString serialNumber;
    QString appName;
    QString fileName;

    static Result<ReadFileRequest> fromQuery(const QMap<QString, QString>& query);
    Result<void> validate() const;
};

/**
 * @brief 删除文件请求
 * DELETE /admin/file/delete
 */
struct DeleteFileRequest {
    QString serialNumber;
    QString appName;
    QString fileName;

    static Result<DeleteFileRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

// ==============================================================================
// 管理接口请求 DTO - 配置管理
// ==============================================================================

/**
 * @brief 设置默认值请求
 * POST /admin/settings/defaults
 */
struct SetDefaultsRequest {
    QString appName;
    QString containerName;
    QString commonName;
    QString organization;
    QString unit;
    QString role;

    static Result<SetDefaultsRequest> fromJson(const QJsonObject& json);
    Result<void> validate() const;
};

}  // namespace api
}  // namespace wekey
