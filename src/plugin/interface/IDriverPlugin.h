/**
 * @file IDriverPlugin.h
 * @brief 驱动插件接口定义
 *
 * 定义设备驱动插件必须实现的接口
 */

#pragma once

#include <QList>
#include <QString>
#include <QVariantMap>
#include <QtPlugin>

#include "common/Result.h"
#include "plugin/interface/PluginTypes.h"

namespace wekey {

/**
 * @brief 驱动插件接口
 *
 * 所有设备驱动插件必须实现此接口
 */
class IDriverPlugin {
public:
    virtual ~IDriverPlugin() = default;

    //=== 设备管理 ===

    /**
     * @brief 枚举设备
     * @param login 是否尝试登录
     * @return 设备信息列表
     */
    virtual Result<QList<DeviceInfo>> enumDevices(bool login = false) = 0;

    /**
     * @brief 修改设备认证密码
     * @param devName 设备名称
     * @param oldPin 旧密码
     * @param newPin 新密码
     * @return 操作结果
     */
    virtual Result<void> changeDeviceAuth(const QString& devName, const QString& oldPin, const QString& newPin) = 0;

    /**
     * @brief 设置设备标签
     * @param devName 设备名称
     * @param label 新标签
     * @return 操作结果
     */
    virtual Result<void> setDeviceLabel(const QString& devName, const QString& label) = 0;

    /**
     * @brief 等待设备事件
     * @return 设备事件代码
     */
    virtual Result<int> waitForDeviceEvent() = 0;

    //=== 应用管理 ===

    /**
     * @brief 枚举应用
     * @param devName 设备名称
     * @return 应用信息列表
     */
    virtual Result<QList<AppInfo>> enumApps(const QString& devName) = 0;

    /**
     * @brief 创建应用
     * @param devName 设备名称
     * @param appName 应用名称
     * @param args 额外参数
     * @return 操作结果
     */
    virtual Result<void> createApp(const QString& devName, const QString& appName, const QVariantMap& args) = 0;

    /**
     * @brief 删除应用
     * @param devName 设备名称
     * @param appName 应用名称
     * @return 操作结果
     */
    virtual Result<void> deleteApp(const QString& devName, const QString& appName) = 0;

    /**
     * @brief 打开应用（登录）
     * @param devName 设备名称
     * @param appName 应用名称
     * @param role 角色（user/admin）
     * @param pin PIN 码
     * @return 操作结果
     */
    virtual Result<void> openApp(const QString& devName, const QString& appName, const QString& role,
                                 const QString& pin) = 0;

    /**
     * @brief 关闭应用（登出）
     * @param devName 设备名称
     * @param appName 应用名称
     * @return 操作结果
     */
    virtual Result<void> closeApp(const QString& devName, const QString& appName) = 0;

    /**
     * @brief 修改 PIN 码
     * @param devName 设备名称
     * @param appName 应用名称
     * @param role 角色
     * @param oldPin 旧 PIN
     * @param newPin 新 PIN
     * @return 操作结果
     */
    virtual Result<void> changePin(const QString& devName, const QString& appName, const QString& role,
                                   const QString& oldPin, const QString& newPin) = 0;

    /**
     * @brief 解锁 PIN 码
     * @param devName 设备名称
     * @param appName 应用名称
     * @param adminPin 管理员 PIN
     * @param newUserPin 新用户 PIN
     * @param args 额外参数
     * @return 操作结果
     */
    virtual Result<void> unlockPin(const QString& devName, const QString& appName, const QString& adminPin,
                                   const QString& newUserPin, const QVariantMap& args) = 0;

    /**
     * @brief 获取 PIN 重试次数
     * @param devName 设备名称
     * @param appName 应用名称
     * @param role 角色
     * @param pin PIN码（用于验证并获取剩余次数）
     * @return 剩余重试次数
     */
    virtual Result<int> getRetryCount(const QString& devName, const QString& appName,
                                      const QString& role, const QString& pin) = 0;

    //=== 容器管理 ===

    /**
     * @brief 枚举容器
     * @param devName 设备名称
     * @param appName 应用名称
     * @return 容器信息列表
     */
    virtual Result<QList<ContainerInfo>> enumContainers(const QString& devName, const QString& appName) = 0;

    /**
     * @brief 创建容器
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @return 操作结果
     */
    virtual Result<void> createContainer(const QString& devName, const QString& appName,
                                         const QString& containerName) = 0;

    /**
     * @brief 删除容器
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @return 操作结果
     */
    virtual Result<void> deleteContainer(const QString& devName, const QString& appName,
                                         const QString& containerName) = 0;

    /**
     * @brief 生成密钥对
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param keyType 密钥类型（RSA/SM2）
     * @return 公钥数据
     */
    virtual Result<QByteArray> generateKeyPair(const QString& devName, const QString& appName,
                                               const QString& containerName, const QString& keyType) = 0;

    /**
     * @brief 生成证书签名请求 (CSR)
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param args 扩展参数：
     *   - renewKey (bool): 是否重新生成密钥对，默认 false
     *   - keyType (QString): 密钥类型（RSA/SM2），默认 SM2
     *   - keySize (int): RSA 密钥长度（2048/3072/4096），默认 2048
     *   - cname (QString): 通用名称 (Common Name)
     *   - org (QString): 组织名称 (Organization)
     *   - unit (QString): 组织单位 (Organizational Unit)
     * @return DER 编码的 CSR 数据
     */
    virtual Result<QByteArray> generateCsr(const QString& devName, const QString& appName,
                                            const QString& containerName, const QVariantMap& args) = 0;

    //=== 证书管理 ===

    /**
     * @brief 导入证书
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param certData 证书数据
     * @param isSignCert 是否为签名证书
     * @return 操作结果
     */
    virtual Result<void> importCert(const QString& devName, const QString& appName, const QString& containerName,
                                    const QByteArray& certData, bool isSignCert) = 0;

    /**
     * @brief 统一导入证书和密钥（对应 Go 的 ImportCert）
     *
     * 在单个设备/容器会话中完成签名证书、加密证书、加密私钥的导入。
     * 会自动检测容器密钥类型来决定私钥导入方式（RSA/SM2）。
     *
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param sigCert 签名证书 DER 数据（可为空）
     * @param encCert 加密证书 DER 数据（可为空）
     * @param encPrivate 加密私钥数据（可为空）
     * @param nonGM 是否非国密（会与容器类型取或）
     * @return 操作结果
     */
    virtual Result<void> importKeyCert(const QString& devName, const QString& appName, const QString& containerName,
                                       const QByteArray& sigCert, const QByteArray& encCert,
                                       const QByteArray& encPrivate, bool nonGM) = 0;

    /**
     * @brief 导出证书
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param isSignCert 是否为签名证书
     * @return 证书数据
     */
    virtual Result<QByteArray> exportCert(const QString& devName, const QString& appName, const QString& containerName,
                                          bool isSignCert) = 0;

    /**
     * @brief 获取证书信息
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param isSignCert 是否为签名证书
     * @return 证书信息
     */
    virtual Result<CertInfo> getCertInfo(const QString& devName, const QString& appName, const QString& containerName,
                                         bool isSignCert) = 0;

    //=== 签名验签 ===

    /**
     * @brief 数据签名
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param data 待签名数据
     * @param algorithm 签名算法
     * @return 签名值
     */
    virtual Result<QByteArray> sign(const QString& devName, const QString& appName, const QString& containerName,
                                    const QByteArray& data) = 0;

    /**
     * @brief 验证签名
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param data 原始数据
     * @param signature 签名值
     * @param algorithm 签名算法
     * @return 验证结果
     */
    virtual Result<bool> verify(const QString& devName, const QString& appName, const QString& containerName,
                                const QByteArray& data, const QByteArray& signature) = 0;

    //=== 文件操作 ===

    /**
     * @brief 枚举文件
     * @param devName 设备名称
     * @param appName 应用名称
     * @return 文件名列表
     */
    virtual Result<QStringList> enumFiles(const QString& devName, const QString& appName) = 0;

    /**
     * @brief 读取文件
     * @param devName 设备名称
     * @param appName 应用名称
     * @param fileName 文件名
     * @return 文件内容
     */
    virtual Result<QByteArray> readFile(const QString& devName, const QString& appName, const QString& fileName) = 0;

    /**
     * @brief 写入文件（先创建文件再写入数据）
     *
     * 若文件不存在则先调用 SKF_CreateFile 创建，再调用 SKF_WriteFile 写入数据。
     * 权限值对应 SKF 规范：
     *   - SECURE_NEVER_ACCOUNT  = 0x00000000 不允许
     *   - SECURE_ADM_ACCOUNT    = 0x00000001 管理员权限
     *   - SECURE_USER_ACCOUNT   = 0x00000010 用户权限
     *   - SECURE_ANYONE_ACCOUNT = 0x000000FF 任何人
     *
     * @param devName 设备名称
     * @param appName 应用名称
     * @param fileName 文件名
     * @param data 文件数据
     * @param readRights 读权限，默认 0xFF（任何人可读）
     * @param writeRights 写权限，默认 0x01（仅管理员可写）
     * @return 操作结果
     */
    virtual Result<void> writeFile(const QString& devName, const QString& appName, const QString& fileName,
                                   const QByteArray& data, int readRights = 0xFF, int writeRights = 0x01) = 0;

    /**
     * @brief 删除文件
     * @param devName 设备名称
     * @param appName 应用名称
     * @param fileName 文件名
     * @return 操作结果
     */
    virtual Result<void> deleteFile(const QString& devName, const QString& appName, const QString& fileName) = 0;

    //=== 其他 ===

    /**
     * @brief 生成随机数
     * @param devName 设备名称
     * @param count 字节数
     * @return 随机数据
     */
    virtual Result<QByteArray> generateRandom(const QString& devName, int count) = 0;
};

}  // namespace wekey

#define IDriverPlugin_iid "com.trustasia.wekey.IDriverPlugin/1.0"
Q_DECLARE_INTERFACE(wekey::IDriverPlugin, IDriverPlugin_iid)
