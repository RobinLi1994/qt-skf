/**
 * @file PluginTypes.h
 * @brief 插件系统数据类型定义
 *
 * 定义设备、应用、容器、证书等数据结构
 */

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QMetaType>
#include <QString>

namespace wekey {

/**
 * @brief 设备信息结构体
 */
struct DeviceInfo {
    QString deviceName;       ///< 设备名称
    QString devicePath;       ///< 设备路径
    QString manufacturer;     ///< 制造商
    QString label;            ///< 设备标签
    QString serialNumber;     ///< 序列号
    QString hardwareVersion;  ///< 硬件版本
    QString firmwareVersion;  ///< 固件版本
    bool isLoggedIn = false;  ///< 是否已登录
};

/**
 * @brief 应用信息结构体
 */
struct AppInfo {
    QString appName;          ///< 应用名称
    bool isLoggedIn = false;  ///< 是否已登录
};

/**
 * @brief 容器信息结构体
 */
struct ContainerInfo {
    /**
     * @brief 密钥类型枚举
     */
    enum class KeyType { Unknown = 0, RSA = 1, SM2 = 2 };

    QString containerName;         ///< 容器名称
    bool keyGenerated = false;     ///< 是否已生成密钥
    KeyType keyType = KeyType::Unknown;  ///< 密钥类型
    bool certImported = false;     ///< 是否已导入证书
};

/**
 * @brief 证书信息结构体
 */
struct CertInfo {
    QString subjectDn;     ///< 主题 DN
    QString commonName;    ///< 通用名称 (CN)
    QString issuerDn;      ///< 颁发者 DN
    QString serialNumber;  ///< 证书序列号
    QDateTime notBefore;   ///< 有效期开始
    QDateTime notAfter;    ///< 有效期结束
    int certType = 0;      ///< 证书类型 (0=签名证书, 1=加密证书)
    QString pubKeyHash;    ///< 公钥哈希
    QString cert;          ///< Base64 编码的证书内容
    QByteArray rawData;    ///< 原始证书数据
};

/**
 * @brief 设备事件枚举
 */
enum class DeviceEvent {
    None = 0,     ///< 无事件
    Inserted = 1, ///< 设备插入
    Removed = 2   ///< 设备移除
};

}  // namespace wekey

Q_DECLARE_METATYPE(wekey::DeviceInfo)
Q_DECLARE_METATYPE(wekey::AppInfo)
Q_DECLARE_METATYPE(wekey::ContainerInfo)
Q_DECLARE_METATYPE(wekey::CertInfo)
