/**
 * @file SkfPlugin.h
 * @brief SKF 驱动插件实现
 *
 * 实现 IDriverPlugin 接口，封装 SKF 库调用
 */

#pragma once

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QString>
#include <memory>

#include "SkfLibrary.h"
#include "common/Result.h"
#include "plugin/interface/IDriverPlugin.h"

namespace wekey {

/**
 * @brief 登录凭据信息
 *
 * 缓存已登录应用的 PIN 和角色，用于后续操作（如创建容器）时重新验证
 */
struct LoginInfo {
    QString pin;   ///< 登录 PIN 码
    QString role;  ///< 登录角色（"admin" 或 "user"）
};

/**
 * @brief 句柄信息结构
 *
 * 跟踪设备、应用、容器的句柄状态
 */
struct HandleInfo {
    skf::DEVHANDLE devHandle = nullptr;      ///< 设备句柄
    skf::HAPPLICATION appHandle = nullptr;   ///< 应用句柄
    skf::HCONTAINER containerHandle = nullptr;  ///< 容器句柄
    bool isLoggedIn = false;                 ///< 是否已登录
};

/**
 * @brief SKF 驱动插件
 *
 * 实现 IDriverPlugin 接口，提供 SKF 设备的完整功能
 */
class SkfPlugin : public QObject, public IDriverPlugin {
    Q_OBJECT
    Q_INTERFACES(wekey::IDriverPlugin)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SkfPlugin(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     *
     * 自动关闭所有打开的句柄
     */
    ~SkfPlugin() override;

    /**
     * @brief 初始化插件
     * @param libPath SKF 库路径
     * @return 操作结果
     */
    Result<void> initialize(const QString& libPath);

    //=== IDriverPlugin 接口实现 ===

    //--- 设备管理 (4 个方法) ---

    Result<QList<DeviceInfo>> enumDevices(bool login = false) override;
    Result<void> changeDeviceAuth(const QString& devName, const QString& oldPin, const QString& newPin) override;
    Result<void> setDeviceLabel(const QString& devName, const QString& label) override;
    Result<int> waitForDeviceEvent() override;

    //--- 应用管理 (8 个方法) ---

    Result<QList<AppInfo>> enumApps(const QString& devName) override;
    Result<void> createApp(const QString& devName, const QString& appName, const QVariantMap& args) override;
    Result<void> deleteApp(const QString& devName, const QString& appName) override;
    Result<void> openApp(const QString& devName, const QString& appName, const QString& role,
                         const QString& pin) override;
    Result<void> closeApp(const QString& devName, const QString& appName) override;
    Result<void> changePin(const QString& devName, const QString& appName, const QString& role, const QString& oldPin,
                           const QString& newPin) override;
    Result<void> unlockPin(const QString& devName, const QString& appName, const QString& adminPin,
                           const QString& newUserPin, const QVariantMap& args) override;
    Result<int> getRetryCount(const QString& devName, const QString& appName,
                              const QString& role, const QString& pin) override;

    //--- 容器管理 (3 个方法) ---

    Result<QList<ContainerInfo>> enumContainers(const QString& devName, const QString& appName) override;
    Result<void> createContainer(const QString& devName, const QString& appName,
                                 const QString& containerName) override;
    Result<void> deleteContainer(const QString& devName, const QString& appName,
                                 const QString& containerName) override;

    //--- 密钥操作 (2 个方法) ---

    Result<QByteArray> generateKeyPair(const QString& devName, const QString& appName, const QString& containerName,
                                       const QString& keyType) override;
    Result<QByteArray> generateCsr(const QString& devName, const QString& appName, const QString& containerName,
                                    const QVariantMap& args) override;

    //--- 证书管理 (4 个方法) ---

    Result<void> importCert(const QString& devName, const QString& appName, const QString& containerName,
                            const QByteArray& certData, bool isSignCert) override;
    Result<void> importKeyCert(const QString& devName, const QString& appName, const QString& containerName,
                               const QByteArray& sigCert, const QByteArray& encCert,
                               const QByteArray& encPrivate, bool nonGM) override;
    Result<QByteArray> exportCert(const QString& devName, const QString& appName, const QString& containerName,
                                  bool isSignCert) override;
    Result<CertInfo> getCertInfo(const QString& devName, const QString& appName, const QString& containerName,
                                 bool isSignCert) override;

    //--- 签名验签 (2 个方法) ---

    Result<QByteArray> sign(const QString& devName, const QString& appName, const QString& containerName,
                            const QByteArray& data) override;
    Result<bool> verify(const QString& devName, const QString& appName, const QString& containerName,
                        const QByteArray& data, const QByteArray& signature) override;

    //--- 文件操作 (4 个方法) ---

    Result<QStringList> enumFiles(const QString& devName, const QString& appName) override;
    Result<QByteArray> readFile(const QString& devName, const QString& appName, const QString& fileName) override;
    Result<void> writeFile(const QString& devName, const QString& appName, const QString& fileName,
                           const QByteArray& data, int readRights = 0xFF, int writeRights = 0x01) override;
    Result<void> deleteFile(const QString& devName, const QString& appName, const QString& fileName) override;

    //--- 其他 (1 个方法) ---

    Result<QByteArray> generateRandom(const QString& devName, int count) override;

private:
    /**
     * @brief 打开设备
     * @param devName 设备名称
     * @return 设备句柄
     */
    Result<skf::DEVHANDLE> openDevice(const QString& devName);

    /**
     * @brief 关闭设备
     * @param devName 设备名称
     */
    void closeDevice(const QString& devName);

    /**
     * @brief 执行设备认证
     * @param devHandle 设备句柄
     * @param authPin 认证 PIN（默认 "1234567812345678"）
     * @return 认证结果
     */
    Result<void> performDeviceAuth(skf::DEVHANDLE devHandle, const QString& authPin = "1234567812345678");

    /**
     * @brief 打开应用
     * @param devName 设备名称
     * @param appName 应用名称
     * @return 应用句柄
     */
    Result<skf::HAPPLICATION> openAppHandle(const QString& devName, const QString& appName);

    /**
     * @brief 关闭应用句柄
     * @param devName 设备名称
     * @param appName 应用名称
     */
    void closeAppHandle(const QString& devName, const QString& appName);

    /**
     * @brief 打开容器
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @return 容器句柄
     */
    Result<skf::HCONTAINER> openContainerHandle(const QString& devName, const QString& appName,
                                                 const QString& containerName);

    /**
     * @brief 关闭容器句柄
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     */
    void closeContainerHandle(const QString& devName, const QString& appName, const QString& containerName);

    /**
     * @brief 生成句柄键
     * @param dev 设备名称
     * @param app 应用名称（可选）
     * @param container 容器名称（可选）
     * @return 复合键字符串
     */
    QString makeKey(const QString& dev, const QString& app = {}, const QString& container = {}) const;

    /**
     * @brief 解析 SKF 双空字符结尾的字符串列表
     * @param buffer 缓冲区
     * @param size 缓冲区大小
     * @return 字符串列表
     */
    QStringList parseNameList(const char* buffer, size_t size) const;

    /**
     * @brief 解析后的证书信息结构
     */
    struct ParsedCertInfo {
        QString subjectDn;     ///< 主题 DN
        QString commonName;    ///< 通用名称
        QString issuerDn;      ///< 颁发者 DN
        QString serialNumber;  ///< 序列号
        QDateTime notBefore;   ///< 有效期开始
        QDateTime notAfter;    ///< 有效期结束
    };

    /**
     * @brief 解析 DER 格式的 X.509 证书
     * @param certData 证书数据
     * @return 解析结果
     */
    Result<ParsedCertInfo> parseDerCertificate(const QByteArray& certData) const;

    std::unique_ptr<SkfLibrary> lib_;  ///< SKF 库实例
    QMap<QString, HandleInfo> handles_;  ///< 句柄映射表
    QMap<QString, LoginInfo> loginCache_;  ///< 登录凭据缓存，key = "devName/appName"
    mutable QMutex mutex_;  ///< 线程安全互斥锁
};

}  // namespace wekey
