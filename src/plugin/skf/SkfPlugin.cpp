/**
 * @file SkfPlugin.cpp
 * @brief SKF 驱动插件实现
 *
 * 实现 IDriverPlugin 接口，封装 SKF 库调用
 */

#include "SkfPlugin.h"

#include <QCryptographicHash>
#include <QMutexLocker>
#include <QTimeZone>
#include <cstring>

#include <vector>

#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>

namespace wekey {

SkfPlugin::SkfPlugin(QObject* parent) : QObject(parent) {}

SkfPlugin::~SkfPlugin() {
    QMutexLocker locker(&mutex_);

    // 关闭所有打开的句柄（逆序：容器 -> 应用 -> 设备）
    for (auto it = handles_.begin(); it != handles_.end(); ++it) {
        if (it->containerHandle && lib_ && lib_->CloseContainer) {
            lib_->CloseContainer(it->containerHandle);
        }
        if (it->appHandle && lib_ && lib_->CloseApplication) {
            lib_->CloseApplication(it->appHandle);
        }
        if (it->devHandle && lib_ && lib_->DisConnectDev) {
            lib_->DisConnectDev(it->devHandle);
        }
    }
    handles_.clear();
}

Result<void> SkfPlugin::initialize(const QString& libPath) {
    QMutexLocker locker(&mutex_);

    lib_ = std::make_unique<SkfLibrary>(libPath);
    if (!lib_->isLoaded()) {
        auto errMsg = lib_->errorString();
        lib_.reset();
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF 库加载失败：" + errMsg, "SkfPlugin::initialize"));
    }

    return Result<void>::ok();
}

//=== 辅助方法 ===

QString SkfPlugin::makeKey(const QString& dev, const QString& app, const QString& container) const {
    if (!container.isEmpty()) {
        return dev + "/" + app + "/" + container;
    }
    if (!app.isEmpty()) {
        return dev + "/" + app;
    }
    return dev;
}

QStringList SkfPlugin::parseNameList(const char* buffer, size_t size) const {
    QStringList result;
    if (!buffer || size == 0) {
        return result;
    }

    const char* p = buffer;
    const char* end = buffer + size;

    while (p < end) {
        if (*p == '\0') {
            break;
        }
        QString name = QString::fromLocal8Bit(p);
        result.append(name);
        p += std::strlen(p) + 1;
    }

    return result;
}

Result<skf::DEVHANDLE> SkfPlugin::openDevice(const QString& devName) {
    QString key = makeKey(devName);

    // 复用已有句柄
    if (handles_.contains(key) && handles_[key].devHandle) {
        return Result<skf::DEVHANDLE>::ok(handles_[key].devHandle);
    }

    if (!lib_ || !lib_->ConnectDev) {
        return Result<skf::DEVHANDLE>::err(
            Error(Error::PluginLoadFailed, "SKF 库未加载", "SkfPlugin::openDevice"));
    }

    skf::DEVHANDLE hDev = nullptr;
    QByteArray nameBytes = devName.toLocal8Bit();
    skf::ULONG ret = lib_->ConnectDev(nameBytes.constData(), &hDev);

    if (ret != skf::SAR_OK) {
        return Result<skf::DEVHANDLE>::err(Error::fromSkf(ret, "SKF_ConnectDev"));
    }

    HandleInfo info;
    info.devHandle = hDev;
    handles_[key] = info;

    return Result<skf::DEVHANDLE>::ok(hDev);
}

void SkfPlugin::closeDevice(const QString& devName) {
    QString devKey = makeKey(devName);

    // 级联清理：先关闭所有依赖此设备的子级句柄
    // 第一步：关闭所有容器句柄 (devName/app/container)
    QStringList keysToRemove;
    for (auto it = handles_.begin(); it != handles_.end(); ++it) {
        const QString& key = it.key();
        // 找到所有以 "devName/" 开头且包含2个斜杠的 key (容器层)
        if (key.startsWith(devKey + "/") && key.count('/') == 2) {
            keysToRemove.append(key);
        }
    }

    for (const QString& key : keysToRemove) {
        auto& info = handles_[key];
        if (info.containerHandle && lib_ && lib_->CloseContainer) {
            lib_->CloseContainer(info.containerHandle);
        }
        handles_.remove(key);
    }

    // 第二步：关闭所有应用句柄 (devName/app)
    keysToRemove.clear();
    for (auto it = handles_.begin(); it != handles_.end(); ++it) {
        const QString& key = it.key();
        // 找到所有以 "devName/" 开头且包含1个斜杠的 key (应用层)
        if (key.startsWith(devKey + "/") && key.count('/') == 1) {
            keysToRemove.append(key);
        }
    }

    for (const QString& key : keysToRemove) {
        auto& info = handles_[key];
        if (info.appHandle && lib_ && lib_->CloseApplication) {
            lib_->CloseApplication(info.appHandle);
        }
        handles_.remove(key);
    }

    // 第三步：关闭设备句柄
    if (!handles_.contains(devKey)) {
        return;
    }

    auto& info = handles_[devKey];
    if (info.devHandle && lib_ && lib_->DisConnectDev) {
        lib_->DisConnectDev(info.devHandle);
    }
    handles_.remove(devKey);
}

Result<void> SkfPlugin::performDeviceAuth(skf::DEVHANDLE devHandle, const QString& authPin) {
    // SKF 设备认证流程：GenRandom → SetSymmKey → EncryptInit → Encrypt → DevAuth

    if (!lib_->GenRandom || !lib_->SetSymmKey || !lib_->EncryptInit || !lib_->Encrypt || !lib_->DevAuth) {
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "设备认证所需接口不完整", "SkfPlugin::performDeviceAuth"));
    }

    // 1. 生成 16 字节随机数
    constexpr int kRandLen = 16;
    skf::BYTE randBytes[kRandLen] = {};
    skf::ULONG ret = lib_->GenRandom(devHandle, randBytes, kRandLen);
    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_GenRandom"));
    }

    // 2. 用 authPIN 设置对称密钥（SM4 ECB 模式）
    QByteArray authPinBytes = authPin.toUtf8();
    skf::HANDLE hKey = nullptr;
    ret = lib_->SetSymmKey(devHandle,
                           reinterpret_cast<skf::BYTE*>(authPinBytes.data()),
                           skf::SGD_SM4_ECB, &hKey);
    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_SetSymmKey"));
    }

    // 3. 初始化加密（ECB 模式无需 IV）
    skf::BLOCKCIPHERPARAM bp = {};
    ret = lib_->EncryptInit(hKey, bp);
    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_EncryptInit"));
    }

    // 4. 加密随机数
    constexpr int kEncBufLen = 256;
    skf::BYTE encryptBytes[kEncBufLen] = {};
    skf::ULONG encryptLen = kEncBufLen;
    ret = lib_->Encrypt(hKey, randBytes, kRandLen, encryptBytes, &encryptLen);
    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_Encrypt"));
    }

    // 5. 设备认证
    ret = lib_->DevAuth(devHandle, encryptBytes, encryptLen);
    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_DevAuth"));
    }

    return Result<void>::ok();
}

Result<skf::HAPPLICATION> SkfPlugin::openAppHandle(const QString& devName, const QString& appName) {
    QString key = makeKey(devName, appName);

    // 复用已有句柄
    if (handles_.contains(key) && handles_[key].appHandle) {
        return Result<skf::HAPPLICATION>::ok(handles_[key].appHandle);
    }

    // 先确保设备已连接
    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<skf::HAPPLICATION>::err(devResult.error());
    }

    if (!lib_->OpenApplication) {
        return Result<skf::HAPPLICATION>::err(
            Error(Error::PluginLoadFailed, "SKF_OpenApplication 函数不可用", "SkfPlugin::openAppHandle"));
    }

    skf::HAPPLICATION hApp = nullptr;
    QByteArray appBytes = appName.toLocal8Bit();
    skf::ULONG ret = lib_->OpenApplication(devResult.value(), appBytes.constData(), &hApp);

    if (ret != skf::SAR_OK) {
        return Result<skf::HAPPLICATION>::err(Error::fromSkf(ret, "SKF_OpenApplication"));
    }

    HandleInfo info;
    info.appHandle = hApp;
    handles_[key] = info;

    return Result<skf::HAPPLICATION>::ok(hApp);
}

void SkfPlugin::closeAppHandle(const QString& devName, const QString& appName) {
    QString appKey = makeKey(devName, appName);

    // 如果应用已登录，保留应用句柄以维持认证会话
    // 只关闭容器句柄，不关闭应用句柄
    QString loginKey = devName + "/" + appName;
    bool isLoggedIn = loginCache_.contains(loginKey);

    // 级联清理：先关闭所有依赖此应用的容器句柄
    QStringList keysToRemove;
    for (auto it = handles_.begin(); it != handles_.end(); ++it) {
        const QString& key = it.key();
        // 找到所有以 "devName/appName/" 开头的 key (容器层)
        if (key.startsWith(appKey + "/")) {
            keysToRemove.append(key);
        }
    }

    for (const QString& key : keysToRemove) {
        auto& info = handles_[key];
        if (info.containerHandle && lib_ && lib_->CloseContainer) {
            lib_->CloseContainer(info.containerHandle);
        }
        handles_.remove(key);
    }

    // 已登录时保留应用句柄，避免丢失 PIN 认证状态
    if (isLoggedIn) {
        qDebug() << "[closeAppHandle] 应用已登录，保留句柄:" << appKey;
        return;
    }

    // 关闭应用句柄
    if (!handles_.contains(appKey)) {
        return;
    }

    auto& info = handles_[appKey];
    if (info.appHandle && lib_ && lib_->CloseApplication) {
        lib_->CloseApplication(info.appHandle);
    }
    handles_.remove(appKey);
}

Result<skf::HCONTAINER> SkfPlugin::openContainerHandle(const QString& devName, const QString& appName,
                                                        const QString& containerName) {
    QString key = makeKey(devName, appName, containerName);

    // 复用已有句柄
    if (handles_.contains(key) && handles_[key].containerHandle) {
        return Result<skf::HCONTAINER>::ok(handles_[key].containerHandle);
    }

    // 先确保应用已打开
    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<skf::HCONTAINER>::err(appResult.error());
    }

    if (!lib_->OpenContainer) {
        return Result<skf::HCONTAINER>::err(
            Error(Error::PluginLoadFailed, "SKF_OpenContainer 函数不可用", "SkfPlugin::openContainerHandle"));
    }

    skf::HCONTAINER hContainer = nullptr;
    QByteArray containerBytes = containerName.toLocal8Bit();
    skf::ULONG ret = lib_->OpenContainer(appResult.value(), containerBytes.constData(), &hContainer);

    if (ret != skf::SAR_OK) {
        return Result<skf::HCONTAINER>::err(Error::fromSkf(ret, "SKF_OpenContainer"));
    }

    HandleInfo info;
    info.containerHandle = hContainer;
    handles_[key] = info;

    return Result<skf::HCONTAINER>::ok(hContainer);
}

void SkfPlugin::closeContainerHandle(const QString& devName, const QString& appName, const QString& containerName) {
    QString key = makeKey(devName, appName, containerName);

    if (!handles_.contains(key)) {
        return;
    }

    auto& info = handles_[key];
    if (info.containerHandle && lib_ && lib_->CloseContainer) {
        lib_->CloseContainer(info.containerHandle);
    }
    handles_.remove(key);
}

//=== 设备管理 ===

Result<QList<DeviceInfo>> SkfPlugin::enumDevices(bool /*login*/) {
    QMutexLocker locker(&mutex_);

    if (!lib_ || !lib_->EnumDev) {
        return Result<QList<DeviceInfo>>::err(
            Error(Error::PluginLoadFailed, "SKF 库未加载", "SkfPlugin::enumDevices"));
    }

    // 先用预分配缓冲区尝试一次调用获取设备列表，避免双次 EnumDev（减少 USB 扫描开销）
    skf::ULONG size = 4096;
    QByteArray buffer(static_cast<int>(size), '\0');
    skf::ULONG ret = lib_->EnumDev(1, buffer.data(), &size);
    if (ret != skf::SAR_OK) {
        // 缓冲区不够，回退到双次调用
        size = 0;
        ret = lib_->EnumDev(1, nullptr, &size);
        if (ret != skf::SAR_OK) {
            return Result<QList<DeviceInfo>>::err(Error::fromSkf(ret, "SKF_EnumDev"));
        }
        if (size == 0) {
            devInfoCache_.clear();
            return Result<QList<DeviceInfo>>::ok({});
        }
        buffer.resize(static_cast<int>(size));
        buffer.fill('\0');
        ret = lib_->EnumDev(1, buffer.data(), &size);
        if (ret != skf::SAR_OK) {
            return Result<QList<DeviceInfo>>::err(Error::fromSkf(ret, "SKF_EnumDev"));
        }
    }

    if (size == 0) {
        devInfoCache_.clear();
        return Result<QList<DeviceInfo>>::ok({});
    }

    QStringList devNames = parseNameList(buffer.constData(), size);
    QSet<QString> currentDevs(devNames.begin(), devNames.end());

    // 清理已拔出设备的缓存
    for (auto it = devInfoCache_.begin(); it != devInfoCache_.end(); ) {
        if (!currentDevs.contains(it.key())) {
            it = devInfoCache_.erase(it);
        } else {
            ++it;
        }
    }

    QList<DeviceInfo> devices;

    for (const auto& name : devNames) {
        // 优先使用缓存的设备信息，避免重复 ConnectDev/DisConnectDev
        if (devInfoCache_.contains(name)) {
            DeviceInfo info = devInfoCache_[name];
            // 刷新登录状态（可能在两次枚举之间变化）
            info.isLoggedIn = false;
            if (!info.serialNumber.isEmpty()) {
                for (const auto& cacheKey : loginCache_.keys()) {
                    if (cacheKey.startsWith(info.serialNumber + "/")) {
                        info.isLoggedIn = true;
                        break;
                    }
                }
            }
            devices.append(info);
            continue;
        }

        DeviceInfo info;
        info.deviceName = name;

        // 首次发现的设备：调用 ConnectDev/GetDevInfo/DisConnectDev 获取信息
        if (lib_->ConnectDev && lib_->GetDevInfo && lib_->DisConnectDev) {
            skf::DEVHANDLE hDev = nullptr;
            QByteArray nameBytes = name.toLocal8Bit();
            ret = lib_->ConnectDev(nameBytes.constData(), &hDev);
            if (ret == skf::SAR_OK && hDev) {
                skf::DEVINFO devInfo;
                std::memset(&devInfo, 0, sizeof(devInfo));
                ret = lib_->GetDevInfo(hDev, &devInfo);
                if (ret == skf::SAR_OK) {
                    info.manufacturer = QString::fromLocal8Bit(devInfo.manufacturer);
                    info.label = QString::fromLocal8Bit(devInfo.label);
                    info.serialNumber = QString::fromLocal8Bit(devInfo.serialNumber);
                    info.hardwareVersion =
                        QString("%1.%2").arg(devInfo.hwVersion.major).arg(devInfo.hwVersion.minor);
                    info.firmwareVersion =
                        QString("%1.%2").arg(devInfo.firmwareVersion.major).arg(devInfo.firmwareVersion.minor);
                }
                lib_->DisConnectDev(hDev);
            }
        }

        // 缓存设备信息
        devInfoCache_[name] = info;

        // 从独立的登录缓存中检查是否有已登录的应用
        if (!info.serialNumber.isEmpty()) {
            for (const auto& cacheKey : loginCache_.keys()) {
                if (cacheKey.startsWith(info.serialNumber + "/")) {
                    info.isLoggedIn = true;
                    break;
                }
            }
        }

        devices.append(info);
    }

    return Result<QList<DeviceInfo>>::ok(devices);
}

Result<void> SkfPlugin::changeDeviceAuth(const QString& devName, const QString& oldPin, const QString& newPin) {
    QMutexLocker locker(&mutex_);

    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<void>::err(devResult.error());
    }

    if (!lib_->DevAuth || !lib_->ChangeDevAuthKey) {
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "DevAuth/ChangeDevAuthKey 函数不可用", "SkfPlugin::changeDeviceAuth"));
    }

    // 先用旧密钥认证
    QByteArray oldPinBytes = oldPin.toUtf8();
    skf::ULONG ret =
        lib_->DevAuth(devResult.value(), reinterpret_cast<skf::BYTE*>(oldPinBytes.data()), oldPinBytes.size());
    if (ret != skf::SAR_OK) {
        closeDevice(devName);
        return Result<void>::err(Error::fromSkf(ret, "SKF_DevAuth"));
    }

    // 设置新密钥
    QByteArray newPinBytes = newPin.toUtf8();
    ret = lib_->ChangeDevAuthKey(devResult.value(), reinterpret_cast<skf::BYTE*>(newPinBytes.data()),
                                 newPinBytes.size());
    closeDevice(devName);

    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_ChangeDevAuthKey"));
    }

    return Result<void>::ok();
}

Result<void> SkfPlugin::setDeviceLabel(const QString& devName, const QString& label) {
    QMutexLocker locker(&mutex_);

    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<void>::err(devResult.error());
    }

    if (!lib_->SetLabel) {
        closeDevice(devName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_SetLabel 函数不可用", "SkfPlugin::setDeviceLabel"));
    }

    QByteArray labelBytes = label.toLocal8Bit();
    skf::ULONG ret = lib_->SetLabel(devResult.value(), labelBytes.constData());
    closeDevice(devName);

    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_SetLabel"));
    }

    // 标签已变更，清除缓存以便下次枚举时重新获取
    devInfoCache_.remove(devName);

    return Result<void>::ok();
}

Result<int> SkfPlugin::waitForDeviceEvent() {
    // 注意：不加 mutex_ 锁！
    // SKF_WaitForDevEvent 是阻塞调用，会长时间等待设备插拔事件。
    // 如果持有 mutex_，会导致其他所有 SKF 操作（GUI 和 API）全部阻塞等待。
    // 此函数只读取 lib_ 指针（初始化后不变），不访问 handles_ 等共享状态，无需加锁。

    if (!lib_ || !lib_->WaitForDevEvent) {
        return Result<int>::err(
            Error(Error::PluginLoadFailed, "SKF 库未加载", "SkfPlugin::waitForDeviceEvent"));
    }

    char devName[256] = {};
    skf::ULONG devNameLen = sizeof(devName);
    skf::ULONG event = 0;

    skf::ULONG ret = lib_->WaitForDevEvent(devName, &devNameLen, &event);
    if (ret != skf::SAR_OK) {
        return Result<int>::err(Error::fromSkf(ret, "SKF_WaitForDevEvent"));
    }

    qDebug() << "[waitForDeviceEvent] 设备事件:" << event << "devName:" << devName;
    return Result<int>::ok(static_cast<int>(event));
}

//=== 应用管理 ===

Result<QList<AppInfo>> SkfPlugin::enumApps(const QString& devName) {
    QMutexLocker locker(&mutex_);

    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<QList<AppInfo>>::err(devResult.error());
    }

    if (!lib_->EnumApplication) {
        closeDevice(devName);
        return Result<QList<AppInfo>>::err(
            Error(Error::PluginLoadFailed, "SKF_EnumApplication 函数不可用", "SkfPlugin::enumApps"));
    }

    // 获取缓冲区大小
    skf::ULONG size = 0;
    skf::ULONG ret = lib_->EnumApplication(devResult.value(), nullptr, &size);
    if (ret != skf::SAR_OK) {
        closeDevice(devName);
        return Result<QList<AppInfo>>::err(Error::fromSkf(ret, "SKF_EnumApplication"));
    }

    if (size == 0) {
        closeDevice(devName);
        return Result<QList<AppInfo>>::ok({});
    }

    QByteArray buffer(static_cast<int>(size), '\0');
    ret = lib_->EnumApplication(devResult.value(), buffer.data(), &size);
    closeDevice(devName);

    if (ret != skf::SAR_OK) {
        return Result<QList<AppInfo>>::err(Error::fromSkf(ret, "SKF_EnumApplication"));
    }

    QStringList appNames = parseNameList(buffer.constData(), size);
    QList<AppInfo> apps;
    for (const auto& name : appNames) {
        AppInfo info;
        info.appName = name;
        // 从登录缓存检查应用是否已登录
        QString loginKey = devName + "/" + name;
        info.isLoggedIn = loginCache_.contains(loginKey);
        qDebug() << "[enumApps] app:" << name << "isLoggedIn:" << info.isLoggedIn;
        apps.append(info);
    }

    return Result<QList<AppInfo>>::ok(apps);
}

Result<void> SkfPlugin::createApp(const QString& devName, const QString& appName, const QVariantMap& args) {
    QMutexLocker locker(&mutex_);

    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<void>::err(devResult.error());
    }

    // 参数解析（参考 Go 实现默认值）
    QString authPin = args.value("authPin", "1234567812345678").toString();
    QString adminPin = args.value("adminPin", "12345678").toString();
    int adminRetry = args.value("adminRetry", 3).toInt();
    QString userPin = args.value("userPin", "12345678").toString();
    int userRetry = args.value("userRetry", 3).toInt();
    int fileRights = args.value("fileRights", 255).toInt();

    // 步骤1：设备认证
    qDebug() << "[createApp] 开始设备认证, devName:" << devName;
    auto authResult = performDeviceAuth(devResult.value(), authPin);
    if (authResult.isErr()) {
        closeDevice(devName);
        qWarning() << "[createApp] 设备认证失败:" << authResult.error().message();
        return Result<void>::err(authResult.error());
    }
    qDebug() << "[createApp] 设备认证成功";

    // 步骤2：创建应用
    if (!lib_->CreateApplication) {
        closeDevice(devName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_CreateApplication 函数不可用", "SkfPlugin::createApp"));
    }

    QByteArray appBytes = appName.toLocal8Bit();
    QByteArray adminPinBytes = adminPin.toLocal8Bit();
    QByteArray userPinBytes = userPin.toLocal8Bit();

    skf::HAPPLICATION hApp = nullptr;
    qDebug() << "[createApp] 创建应用:" << appName
             << "adminRetry:" << adminRetry << "userRetry:" << userRetry
             << "fileRights:" << fileRights;
    skf::ULONG ret = lib_->CreateApplication(devResult.value(), appBytes.constData(), adminPinBytes.constData(),
                                  static_cast<skf::DWORD>(adminRetry), userPinBytes.constData(),
                                  static_cast<skf::DWORD>(userRetry),
                                  static_cast<skf::DWORD>(fileRights), &hApp);

    if (hApp && lib_->CloseApplication) {
        lib_->CloseApplication(hApp);
    }
    closeDevice(devName);

    if (ret != skf::SAR_OK) {
        qWarning() << "[createApp] 创建应用失败, ret:" << QString::number(ret, 16);
        return Result<void>::err(Error::fromSkf(ret, "SKF_CreateApplication"));
    }

    qDebug() << "[createApp] 创建应用成功:" << appName;
    return Result<void>::ok();
}

Result<void> SkfPlugin::deleteApp(const QString& devName, const QString& appName) {
    QMutexLocker locker(&mutex_);

    // 步骤1：先关闭该应用的句柄（如果已打开）
    // SKF 规范要求：删除应用前必须先关闭应用句柄
    closeAppHandle(devName, appName);

    // 步骤2：打开设备句柄
    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<void>::err(devResult.error());
    }

    if (!lib_->DeleteApplication) {
        closeDevice(devName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_DeleteApplication 函数不可用", "SkfPlugin::deleteApp"));
    }

    // 步骤3：设备认证（删除应用需要设备认证）
    auto authResult = performDeviceAuth(devResult.value());
    if (authResult.isErr()) {
        closeDevice(devName);
        return authResult;
    }

    // 步骤4：删除应用
    QByteArray appBytes = appName.toLocal8Bit();
    skf::ULONG ret = lib_->DeleteApplication(devResult.value(), appBytes.constData());
    closeDevice(devName);

    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_DeleteApplication"));
    }

    // 步骤5：清理登录缓存
    QString loginKey = devName + "/" + appName;
    loginCache_.remove(loginKey);

    return Result<void>::ok();
}

Result<void> SkfPlugin::openApp(const QString& devName, const QString& appName, const QString& role,
                                 const QString& pin) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<void>::err(appResult.error());
    }

    if (!lib_->VerifyPIN) {
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_VerifyPIN 函数不可用", "SkfPlugin::openApp"));
    }

    // 0 = admin, 1 = user
    skf::ULONG pinType = (role.toLower() == "admin") ? 0 : 1;
    QByteArray pinBytes = pin.toLocal8Bit();
    skf::ULONG retryCount = 0;

    skf::ULONG ret = lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
    if (ret != skf::SAR_OK) {
        closeAppHandle(devName, appName);
        return Result<void>::err(Error::fromSkf(ret, "SKF_VerifyPIN"));
    }

    // 写入登录缓存（保存 PIN 和角色，供后续操作验证使用）
    LoginInfo loginInfo;
    loginInfo.pin = pin;
    loginInfo.role = role;
    loginCache_.insert(devName + "/" + appName, loginInfo);

    return Result<void>::ok();
}

Result<void> SkfPlugin::closeApp(const QString& devName, const QString& appName) {
    QMutexLocker locker(&mutex_);

    // 从登录缓存中删除
    loginCache_.remove(devName + "/" + appName);

    closeAppHandle(devName, appName);
    return Result<void>::ok();
}

Result<void> SkfPlugin::changePin(const QString& devName, const QString& appName, const QString& role,
                                   const QString& oldPin, const QString& newPin) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<void>::err(appResult.error());
    }

    if (!lib_->ChangePIN) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_ChangePIN 函数不可用", "SkfPlugin::changePin"));
    }

    skf::ULONG pinType = (role.toLower() == "admin") ? 0 : 1;
    QByteArray oldPinBytes = oldPin.toLocal8Bit();
    QByteArray newPinBytes = newPin.toLocal8Bit();
    skf::ULONG retryCount = 0;

    skf::ULONG ret = lib_->ChangePIN(appResult.value(), pinType, oldPinBytes.constData(),
                                      newPinBytes.constData(), &retryCount);
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_ChangePIN"));
    }

    return Result<void>::ok();
}

Result<void> SkfPlugin::unlockPin(const QString& devName, const QString& appName, const QString& adminPin,
                                   const QString& newUserPin, const QVariantMap& /*args*/) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<void>::err(appResult.error());
    }

    if (!lib_->UnblockPIN) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_UnblockPIN 函数不可用", "SkfPlugin::unlockPin"));
    }

    QByteArray adminPinBytes = adminPin.toLocal8Bit();
    QByteArray newUserPinBytes = newUserPin.toLocal8Bit();
    skf::ULONG retryCount = 0;

    skf::ULONG ret = lib_->UnblockPIN(appResult.value(), adminPinBytes.constData(),
                                       newUserPinBytes.constData(), &retryCount);
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_UnblockPIN"));
    }

    return Result<void>::ok();
}

Result<int> SkfPlugin::getRetryCount(const QString& devName, const QString& appName,
                                      const QString& role, const QString& pin) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<int>::err(appResult.error());
    }

    if (!lib_->VerifyPIN) {
        closeAppHandle(devName, appName);
        return Result<int>::err(
            Error(Error::PluginLoadFailed, "SKF_VerifyPIN 函数不可用", "SkfPlugin::getRetryCount"));
    }

    skf::ULONG pinType = (role.toLower() == "admin") ? 0 : 1;
    skf::ULONG retryCount = 0;

    // 用传入的 PIN 调用 VerifyPIN 获取剩余次数（会失败但返回 retryCount）
    QByteArray pinBytes = pin.toLocal8Bit();
    lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
    qDebug() << "[getRetryCount] role:" << role << "retryCount:" << retryCount;
    closeAppHandle(devName, appName);

    return Result<int>::ok(static_cast<int>(retryCount));
}

//=== 容器管理 ===

Result<QList<ContainerInfo>> SkfPlugin::enumContainers(const QString& devName, const QString& appName) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<QList<ContainerInfo>>::err(appResult.error());
    }

    if (!lib_->EnumContainer) {
        closeAppHandle(devName, appName);
        return Result<QList<ContainerInfo>>::err(
            Error(Error::PluginLoadFailed, "SKF_EnumContainer 函数不可用", "SkfPlugin::enumContainers"));
    }

    skf::ULONG size = 0;
    skf::ULONG ret = lib_->EnumContainer(appResult.value(), nullptr, &size);
    if (ret != skf::SAR_OK) {
        closeAppHandle(devName, appName);
        return Result<QList<ContainerInfo>>::err(Error::fromSkf(ret, "SKF_EnumContainer"));
    }

    if (size == 0) {
        closeAppHandle(devName, appName);
        return Result<QList<ContainerInfo>>::ok({});
    }

    QByteArray buffer(static_cast<int>(size), '\0');
    ret = lib_->EnumContainer(appResult.value(), buffer.data(), &size);

    if (ret != skf::SAR_OK) {
        closeAppHandle(devName, appName);
        return Result<QList<ContainerInfo>>::err(Error::fromSkf(ret, "SKF_EnumContainer"));
    }

    QStringList containerNames = parseNameList(buffer.constData(), size);
    QList<ContainerInfo> containers;

    for (const auto& name : containerNames) {
        ContainerInfo info;
        info.containerName = name;

        // 尝试获取容器类型和证书导入状态
        if (lib_->OpenContainer && lib_->GetContainerType && lib_->CloseContainer) {
            skf::HCONTAINER hContainer = nullptr;
            QByteArray nameBytes = name.toLocal8Bit();
            if (lib_->OpenContainer(appResult.value(), nameBytes.constData(), &hContainer) == skf::SAR_OK) {
                skf::ULONG containerType = 0;
                if (lib_->GetContainerType(hContainer, &containerType) == skf::SAR_OK) {
                    info.keyGenerated = true;
                    if (containerType == 1) {
                        info.keyType = ContainerInfo::KeyType::RSA;
                    } else if (containerType == 2) {
                        info.keyType = ContainerInfo::KeyType::SM2;
                    }
                }

                // 检查是否已导入证书（尝试导出签名证书，成功且长度>0则已导入）
                if (lib_->ExportCertificate) {
                    skf::ULONG certLen = 0;
                    skf::ULONG certRet = lib_->ExportCertificate(hContainer, 1, nullptr, &certLen);
                    if (certRet == skf::SAR_OK && certLen > 0) {
                        info.certImported = true;
                    } else {
                        // 签名证书不存在，再检查加密证书
                        certLen = 0;
                        certRet = lib_->ExportCertificate(hContainer, 0, nullptr, &certLen);
                        if (certRet == skf::SAR_OK && certLen > 0) {
                            info.certImported = true;
                        }
                    }
                }

                lib_->CloseContainer(hContainer);
            }
        }

        containers.append(info);
    }

    closeAppHandle(devName, appName);
    return Result<QList<ContainerInfo>>::ok(containers);
}

Result<void> SkfPlugin::createContainer(const QString& devName, const QString& appName,
                                         const QString& containerName) {
    QMutexLocker locker(&mutex_);

    qDebug() << "[createContainer] 开始创建容器, devName:" << devName
             << "appName:" << appName << "containerName:" << containerName;

    // 步骤1：检查登录状态（创建容器需要先登录应用验证PIN）
    QString loginKey = devName + "/" + appName;
    if (!loginCache_.contains(loginKey)) {
        qWarning() << "[createContainer] 应用未登录, devName:" << devName << "appName:" << appName;
        return Result<void>::err(
            Error(Error::NotLoggedIn, "应用未登录，请先登录应用", "SkfPlugin::createContainer"));
    }

    // 步骤2：打开应用句柄
    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        qWarning() << "[createContainer] 打开应用失败:" << appResult.error().message();
        return Result<void>::err(appResult.error());
    }

    // 步骤3：使用缓存的凭据验证 PIN（参考 Go 实现，每次操作前重新验证）
    const LoginInfo& cached = loginCache_[loginKey];
    if (!lib_->VerifyPIN) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_VerifyPIN 函数不可用", "SkfPlugin::createContainer"));
    }
    skf::ULONG pinType = (cached.role.toLower() == "admin") ? 0 : 1;
    QByteArray pinBytes = cached.pin.toLocal8Bit();
    skf::ULONG retryCount = 0;
    skf::ULONG verifyRet = lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
    if (verifyRet != skf::SAR_OK) {
        qWarning() << "[createContainer] VerifyPIN 失败, ret:" << QString::number(verifyRet, 16);
        closeAppHandle(devName, appName);
        return Result<void>::err(Error::fromSkf(verifyRet, "SKF_VerifyPIN"));
    }
    qDebug() << "[createContainer] VerifyPIN 成功, role:" << cached.role;

    if (!lib_->CreateContainer) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_CreateContainer 函数不可用", "SkfPlugin::createContainer"));
    }

    // 步骤4：创建容器
    skf::HCONTAINER hContainer = nullptr;
    QByteArray containerBytes = containerName.toLocal8Bit();
    skf::ULONG ret = lib_->CreateContainer(appResult.value(), containerBytes.constData(), &hContainer);

    // 关闭容器句柄（创建后不需要保持打开）
    if (hContainer && lib_->CloseContainer) {
        qDebug() << "[createContainer] 关闭容器句柄";
        lib_->CloseContainer(hContainer);
    }
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        qWarning() << "[createContainer] SKF_CreateContainer 失败, ret:" << QString::number(ret, 16);
        return Result<void>::err(Error::fromSkf(ret, "SKF_CreateContainer"));
    }

    qDebug() << "[createContainer] 创建容器成功:" << containerName;
    return Result<void>::ok();
}

Result<void> SkfPlugin::deleteContainer(const QString& devName, const QString& appName,
                                         const QString& containerName) {
    QMutexLocker locker(&mutex_);

    qDebug() << "[deleteContainer] 开始删除容器, devName:" << devName
             << "appName:" << appName << "containerName:" << containerName;

    // 步骤1：检查登录状态（删除容器需要先登录应用验证PIN）
    QString loginKey = devName + "/" + appName;
    if (!loginCache_.contains(loginKey)) {
        qWarning() << "[deleteContainer] 应用未登录, devName:" << devName << "appName:" << appName;
        return Result<void>::err(
            Error(Error::NotLoggedIn, "应用未登录，请先登录应用", "SkfPlugin::deleteContainer"));
    }

    // 步骤2：关闭目标容器句柄（如果已打开）
    QString containerKey = makeKey(devName, appName, containerName);
    if (handles_.contains(containerKey)) {
        auto& info = handles_[containerKey];
        if (info.containerHandle && lib_ && lib_->CloseContainer) {
            qDebug() << "[deleteContainer] 关闭容器句柄:" << containerKey;
            lib_->CloseContainer(info.containerHandle);
        }
        handles_.remove(containerKey);
    }

    // 步骤3：打开应用句柄
    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        qWarning() << "[deleteContainer] 打开应用失败:" << appResult.error().message();
        return Result<void>::err(appResult.error());
    }

    // 步骤4：使用缓存的凭据验证 PIN
    const LoginInfo& cached = loginCache_[loginKey];
    if (!lib_->VerifyPIN) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_VerifyPIN 函数不可用", "SkfPlugin::deleteContainer"));
    }
    skf::ULONG pinType = (cached.role.toLower() == "admin") ? 0 : 1;
    QByteArray pinBytes = cached.pin.toLocal8Bit();
    skf::ULONG retryCount = 0;
    skf::ULONG verifyRet = lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
    if (verifyRet != skf::SAR_OK) {
        qWarning() << "[deleteContainer] VerifyPIN 失败, ret:" << QString::number(verifyRet, 16);
        closeAppHandle(devName, appName);
        return Result<void>::err(Error::fromSkf(verifyRet, "SKF_VerifyPIN"));
    }
    qDebug() << "[deleteContainer] VerifyPIN 成功, role:" << cached.role;

    if (!lib_->DeleteContainer) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_DeleteContainer 函数不可用", "SkfPlugin::deleteContainer"));
    }

    // 步骤5：删除容器
    QByteArray containerBytes = containerName.toLocal8Bit();
    skf::ULONG ret = lib_->DeleteContainer(appResult.value(), containerBytes.constData());
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        qWarning() << "[deleteContainer] SKF_DeleteContainer 失败, ret:" << QString::number(ret, 16);
        return Result<void>::err(Error::fromSkf(ret, "SKF_DeleteContainer"));
    }

    qDebug() << "[deleteContainer] 删除容器成功:" << containerName;
    return Result<void>::ok();
}

//=== 密钥操作 ===

Result<QByteArray> SkfPlugin::generateKeyPair(const QString& devName, const QString& appName,
                                               const QString& containerName, const QString& keyType) {
    QMutexLocker locker(&mutex_);

    auto containerResult = openContainerHandle(devName, appName, containerName);
    if (containerResult.isErr()) {
        return Result<QByteArray>::err(containerResult.error());
    }

    if (keyType.toUpper() == "SM2") {
        if (!lib_->GenECCKeyPair) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::PluginLoadFailed, "SKF_GenECCKeyPair 函数不可用", "SkfPlugin::generateKeyPair"));
        }

        skf::ECCPUBLICKEYBLOB pubKey;
        std::memset(&pubKey, 0, sizeof(pubKey));
        skf::ULONG ret = lib_->GenECCKeyPair(containerResult.value(), skf::SGD_SM2_1, &pubKey);
        closeContainerHandle(devName, appName, containerName);

        if (ret != skf::SAR_OK) {
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_GenECCKeyPair"));
        }

        QByteArray pubKeyData(reinterpret_cast<const char*>(&pubKey), sizeof(pubKey));
        return Result<QByteArray>::ok(pubKeyData);

    } else {
        // RSA
        if (!lib_->GenRSAKeyPair) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::PluginLoadFailed, "SKF_GenRSAKeyPair 函数不可用", "SkfPlugin::generateKeyPair"));
        }

        skf::ULONG bitsLen = 2048;
        if (keyType.contains("3072")) {
            bitsLen = 3072;
        } else if (keyType.contains("4096")) {
            bitsLen = 4096;
        }

        skf::RSAPUBLICKEYBLOB pubKey;
        std::memset(&pubKey, 0, sizeof(pubKey));
        skf::ULONG ret = lib_->GenRSAKeyPair(containerResult.value(), bitsLen, &pubKey);
        closeContainerHandle(devName, appName, containerName);

        if (ret != skf::SAR_OK) {
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_GenRSAKeyPair"));
        }

        QByteArray pubKeyData(reinterpret_cast<const char*>(&pubKey), sizeof(pubKey));
        return Result<QByteArray>::ok(pubKeyData);
    }
}

//=== CSR 生成辅助函数（基于 OpenSSL）===

namespace {

/**
 * @brief RAII 封装 OpenSSL EVP_PKEY，自动释放
 */
struct EvpPKeyGuard {
    EVP_PKEY* pkey = nullptr;
    ~EvpPKeyGuard() {
        if (pkey) EVP_PKEY_free(pkey);
    }
};

/**
 * @brief 从 SKF ECCPUBLICKEYBLOB 创建 OpenSSL EVP_PKEY（SM2）
 * 使用 EVP_PKEY_fromdata (OpenSSL 3.0+) 避免弃用警告
 */
EVP_PKEY* createSm2EvpPKey(const skf::ECCPUBLICKEYBLOB& blob) {
    qDebug() << "[createSm2EvpPKey] bitLen:" << blob.bitLen;

    // 构建 SM2 SubjectPublicKeyInfo DER 编码，使用 d2i_PUBKEY 解析
    // 格式: SEQUENCE { SEQUENCE { OID ecPublicKey, OID SM2 }, BIT STRING { 04 || x || y } }
    // 固定头部 27 字节 + x(32) + y(32) = 91 字节
    static const unsigned char spkiHeader[27] = {
        0x30, 0x59,                                                 // SEQUENCE (89 bytes)
        0x30, 0x13,                                                 // SEQUENCE (19 bytes) - AlgorithmIdentifier
        0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,     // OID 1.2.840.10045.2.1 (ecPublicKey)
        0x06, 0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x82, 0x2D, // OID 1.2.156.10197.1.301 (SM2)
        0x03, 0x42,                                                 // BIT STRING (66 bytes)
        0x00,                                                       // unused bits = 0
        0x04                                                        // 未压缩点标记
    };

    unsigned char spki[91];
    std::memcpy(spki, spkiHeader, sizeof(spkiHeader));
    std::memcpy(spki + 27, blob.xCoordinate + 32, 32);  // x 坐标
    std::memcpy(spki + 59, blob.yCoordinate + 32, 32);  // y 坐标

    qDebug() << "[createSm2EvpPKey] SPKI(hex):"
             << QByteArray(reinterpret_cast<const char*>(spki), 91).toHex();

    const unsigned char* p = spki;
    EVP_PKEY* pkey = d2i_PUBKEY(nullptr, &p, sizeof(spki));
    if (!pkey) {
        qWarning() << "[createSm2EvpPKey] d2i_PUBKEY failed:";
        unsigned long err;
        while ((err = ERR_get_error()) != 0) {
            qWarning() << "  OpenSSL error:" << ERR_error_string(err, nullptr);
        }
        return nullptr;
    }

    qDebug() << "[createSm2EvpPKey] EVP_PKEY created successfully, type:" << EVP_PKEY_id(pkey);
    return pkey;
}

/**
 * @brief 从 SKF RSAPUBLICKEYBLOB 创建 OpenSSL EVP_PKEY（RSA）
 * 使用 EVP_PKEY_fromdata (OpenSSL 3.0+) 避免弃用警告
 */
EVP_PKEY* createRsaEvpPKey(const skf::RSAPUBLICKEYBLOB& blob) {

    if (blob.bitLen == 0 || blob.bitLen > 4096) {
        qWarning() << "[createRsaEvpPKey] 非法 BitLen:" << blob.bitLen;
        return nullptr;
    }

    int modulusLen = static_cast<int>(blob.bitLen / 8);

    // blob.modulus 是小端存储，BN_bin2bn 需要大端，无条件反转
    std::vector<unsigned char> modulusBE(static_cast<size_t>(modulusLen));
    for (int i = 0; i < modulusLen; ++i) {
        modulusBE[i] = blob.modulus[modulusLen - 1 - i];
    }
    BIGNUM* bn_n = BN_bin2bn(modulusBE.data(), modulusLen, nullptr);

    // publicExponent 同样是小端，00 01 00 01 → 反转 → 01 00 01 00 → 跳过尾部00 → 01 00 01
    // 注意：反转后跳过的是"前导零"（反转前的尾部零）
    int expBufLen = static_cast<int>(sizeof(blob.publicExponent));
    std::vector<unsigned char> expBE(static_cast<size_t>(expBufLen));
    for (int i = 0; i < expBufLen; ++i) {
        expBE[i] = blob.publicExponent[expBufLen - 1 - i];
    }
    const unsigned char* expPtr = expBE.data();
    int expLen = expBufLen;
    while (expLen > 1 && *expPtr == 0x00) { ++expPtr; --expLen; }
    BIGNUM* bn_e = BN_bin2bn(expPtr, expLen, nullptr);

    if (!bn_n || !bn_e) {
        BN_free(bn_n); BN_free(bn_e);
        return nullptr;
    }
    int nLen = BN_num_bytes(bn_n);
    int eLen = BN_num_bytes(bn_e);
    std::vector<unsigned char> nBuf(static_cast<size_t>(nLen));
    std::vector<unsigned char> eBuf(static_cast<size_t>(eLen));
    BN_bn2bin(bn_n, nBuf.data());
    BN_bn2bin(bn_e, eBuf.data());
    BN_free(bn_n);
    BN_free(bn_e);

    qDebug() << "[createRsaEvpPKey] modulus(BE) 首字节:" << QString::number(nBuf[0], 16)
             << "末字节:" << QString::number(nBuf[nLen-1], 16)
             << "exponent:" << QByteArray(reinterpret_cast<const char*>(eBuf.data()), eLen).toHex();
    // 期望输出：首字节=cf，末字节=b5（奇数），exponent=010001

    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_BN("n", nBuf.data(), static_cast<size_t>(nLen)),
        OSSL_PARAM_construct_BN("e", eBuf.data(), static_cast<size_t>(eLen)),
        OSSL_PARAM_END
    };

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr);
    if (!ctx) return nullptr;

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_fromdata_init(ctx) != 1 ||
        EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) != 1) {
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

/**
 * @brief 使用 OpenSSL 构建 CSR 的 CertificationRequestInfo (TBS) 部分
 * @return TBS 的 DER 编码，失败返回空 QByteArray
 */
QByteArray buildCsrTbs(EVP_PKEY* pkey, const QString& cname, const QString& org,
                       const QString& unit, bool /*isSm2*/) {
    X509_REQ* req = X509_REQ_new();
    if (!req) return {};

    // 设置版本 v1 (值为 0)
    X509_REQ_set_version(req, 0);

    // 设置 Subject DN：C=CN, O=org, OU=unit, CN=cname
    X509_NAME* subj = X509_REQ_get_subject_name(req);
    X509_NAME_add_entry_by_txt(subj, "C", MBSTRING_ASC,
        reinterpret_cast<const unsigned char*>("CN"), -1, -1, 0);
    X509_NAME_add_entry_by_txt(subj, "O", MBSTRING_UTF8,
        reinterpret_cast<const unsigned char*>(org.toUtf8().constData()), -1, -1, 0);
    X509_NAME_add_entry_by_txt(subj, "OU", MBSTRING_UTF8,
        reinterpret_cast<const unsigned char*>(unit.toUtf8().constData()), -1, -1, 0);
    X509_NAME_add_entry_by_txt(subj, "CN", MBSTRING_UTF8,
        reinterpret_cast<const unsigned char*>(cname.toUtf8().constData()), -1, -1, 0);

    // 设置公钥
    X509_REQ_set_pubkey(req, pkey);

    // 序列化 TBS (CertificationRequestInfo)
    unsigned char* tbsDer = nullptr;
    int tbsLen = i2d_re_X509_REQ_tbs(req, &tbsDer);
    X509_REQ_free(req);

    if (tbsLen <= 0 || !tbsDer) {
        return {};
    }

    QByteArray result(reinterpret_cast<const char*>(tbsDer), tbsLen);
    OPENSSL_free(tbsDer);
    return result;
}

/**
 * @brief 将 ECCSIGNATUREBLOB 编码为 DER 格式的 ECDSA-Sig-Value
 * ECDSA-Sig-Value ::= SEQUENCE { r INTEGER, s INTEGER }
 */
QByteArray encodeEccSignatureDer(const skf::ECCSIGNATUREBLOB& eccSig) {
    // 从 64 字节数组的后 32 字节提取 r 和 s
    BIGNUM* r = BN_bin2bn(eccSig.r + 32, 32, nullptr);
    BIGNUM* s = BN_bin2bn(eccSig.s + 32, 32, nullptr);
    if (!r || !s) {
        BN_free(r);
        BN_free(s);
        return {};
    }

    // 使用 OpenSSL ECDSA_SIG 进行 DER 编码
    ECDSA_SIG* sig = ECDSA_SIG_new();
    // ECDSA_SIG_set0 接管 BIGNUM 所有权
    ECDSA_SIG_set0(sig, r, s);

    unsigned char* derBuf = nullptr;
    int derLen = i2d_ECDSA_SIG(sig, &derBuf);
    ECDSA_SIG_free(sig);

    if (derLen <= 0 || !derBuf) {
        return {};
    }

    QByteArray result(reinterpret_cast<const char*>(derBuf), derLen);
    OPENSSL_free(derBuf);
    return result;
}

/**
 * @brief 编码 DER 长度字段
 */
QByteArray derEncodeLength(int length) {
    QByteArray result;
    if (length < 0x80) {
        result.append(static_cast<char>(length));
    } else if (length < 0x100) {
        result.append('\x81');
        result.append(static_cast<char>(length));
    } else {
        result.append('\x82');
        result.append(static_cast<char>((length >> 8) & 0xFF));
        result.append(static_cast<char>(length & 0xFF));
    }
    return result;
}

/**
 * @brief 组装最终的 CSR DER 编码
 * CertificationRequest ::= SEQUENCE { TBS, signatureAlgorithm, signatureValue BIT STRING }
 */
QByteArray assembleCsrDer(const QByteArray& tbsDer, const QByteArray& signatureValue, bool isSm2) {
    // 签名算法标识 DER（预编码的固定字节）
    // SM2-with-SM3: SEQUENCE { OID 1.2.156.10197.1.501 }
    static const QByteArray SM2_WITH_SM3_ALG_ID(
        "\x30\x0A\x06\x08\x2A\x81\x1C\xCF\x55\x01\x83\x75", 12);
    // SHA256-with-RSA: SEQUENCE { OID 1.2.840.113549.1.1.11, NULL }
    static const QByteArray SHA256_WITH_RSA_ALG_ID(
        "\x30\x0D\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x01\x0B\x05\x00", 15);

    const QByteArray& sigAlgDer = isSm2 ? SM2_WITH_SM3_ALG_ID : SHA256_WITH_RSA_ALG_ID;

    // BIT STRING 封装签名值
    int bitStringContentLen = signatureValue.size() + 1;  // +1 for unused bits byte
    QByteArray bitString;
    bitString.append('\x03');
    bitString.append(derEncodeLength(bitStringContentLen));
    bitString.append('\x00');  // unused bits = 0
    bitString.append(signatureValue);

    // 最外层 SEQUENCE 封装
    QByteArray content;
    content.append(tbsDer);
    content.append(sigAlgDer);
    content.append(bitString);

    QByteArray csrDer;
    csrDer.append('\x30');
    csrDer.append(derEncodeLength(content.size()));
    csrDer.append(content);

    return csrDer;
}

}  // namespace

Result<QByteArray> SkfPlugin::generateCsr(const QString& devName, const QString& appName,
                                            const QString& containerName, const QVariantMap& args) {
    QMutexLocker locker(&mutex_);

    // 解析参数
    bool renewKey = args.value("renewKey", false).toBool();
    QString keyType = args.value("keyType", "SM2").toString().toUpper();
    int keySize = args.value("keySize", 2048).toInt();
    QString cname = args.value("cname", "SKFTool").toString();
    QString org = args.value("org", "TrustAsia").toString();
    QString unit = args.value("unit", "TrustAsia").toString();

    bool isSm2 = (keyType == "SM2");

    // 检查登录状态（密钥生成和签名操作需要先登录应用）
    QString loginKey = devName + "/" + appName;
    if (!loginCache_.contains(loginKey)) {
        qWarning() << "[generateCsr] 应用未登录, devName:" << devName << "appName:" << appName;
        return Result<QByteArray>::err(
            Error(Error::NotLoggedIn, "应用未登录，请先登录", "SkfPlugin::generateCsr"));
    }

    // 使用缓存的凭据验证 PIN（每次操作前重新验证）
    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        qWarning() << "[generateCsr] 打开应用失败:" << appResult.error().message();
        return Result<QByteArray>::err(appResult.error());
    }
    const LoginInfo& cached = loginCache_[loginKey];
    if (!lib_->VerifyPIN) {
        closeAppHandle(devName, appName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_VerifyPIN 函数不可用", "SkfPlugin::generateCsr"));
    }
    skf::ULONG pinType = (cached.role.toLower() == "admin") ? 0 : 1;
    QByteArray pinBytes = cached.pin.toLocal8Bit();
    skf::ULONG retryCount = 0;
    skf::ULONG verifyRet = lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
    if (verifyRet != skf::SAR_OK) {
        qWarning() << "[generateCsr] VerifyPIN 失败, ret:" << QString::number(verifyRet, 16);
        closeAppHandle(devName, appName);
        return Result<QByteArray>::err(Error::fromSkf(verifyRet, "SKF_VerifyPIN"));
    }
    qDebug() << "[generateCsr] VerifyPIN 成功, role:" << cached.role;

    // 打开容器
    auto containerResult = openContainerHandle(devName, appName, containerName);
    if (containerResult.isErr()) {
        return Result<QByteArray>::err(containerResult.error());
    }

    // 步骤 1: 如果 renew=true，重新生成密钥对
    if (renewKey) {
        if (isSm2) {
            if (!lib_->GenECCKeyPair) {
                closeContainerHandle(devName, appName, containerName);
                return Result<QByteArray>::err(
                    Error(Error::PluginLoadFailed, "SKF_GenECCKeyPair 函数不可用", "SkfPlugin::generateCsr"));
            }
            skf::ECCPUBLICKEYBLOB tmpPubKey;
            std::memset(&tmpPubKey, 0, sizeof(tmpPubKey));
            skf::ULONG ret = lib_->GenECCKeyPair(containerResult.value(), skf::SGD_SM2_1, &tmpPubKey);
            if (ret != skf::SAR_OK) {
                closeContainerHandle(devName, appName, containerName);
                return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_GenECCKeyPair"));
            }
        } else {
            if (!lib_->GenRSAKeyPair) {
                closeContainerHandle(devName, appName, containerName);
                return Result<QByteArray>::err(
                    Error(Error::PluginLoadFailed, "SKF_GenRSAKeyPair 函数不可用", "SkfPlugin::generateCsr"));
            }
            skf::RSAPUBLICKEYBLOB tmpPubKey;
            std::memset(&tmpPubKey, 0, sizeof(tmpPubKey));
            skf::ULONG ret = lib_->GenRSAKeyPair(containerResult.value(), static_cast<skf::ULONG>(keySize), &tmpPubKey);
            if (ret != skf::SAR_OK) {
                closeContainerHandle(devName, appName, containerName);
                return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_GenRSAKeyPair"));
            }
        }
    }

    // 步骤 2: 导出签名公钥并创建 OpenSSL EVP_PKEY
    if (!lib_->ExportPublicKey) {
        closeContainerHandle(devName, appName, containerName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_ExportPublicKey 函数不可用", "SkfPlugin::generateCsr"));
    }

    EvpPKeyGuard pkeyGuard;
    skf::ECCPUBLICKEYBLOB eccPubKey;
    std::memset(&eccPubKey, 0, sizeof(eccPubKey));
    skf::RSAPUBLICKEYBLOB rsaPubKey;
    std::memset(&rsaPubKey, 0, sizeof(rsaPubKey));

    if (isSm2) {
        skf::ULONG pubKeyLen = sizeof(eccPubKey);
        skf::ULONG ret = lib_->ExportPublicKey(containerResult.value(), true,
            reinterpret_cast<skf::BYTE*>(&eccPubKey), &pubKeyLen);
        if (ret != skf::SAR_OK) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_ExportPublicKey"));
        }
        pkeyGuard.pkey = createSm2EvpPKey(eccPubKey);
    } else {
        skf::ULONG pubKeyLen = sizeof(rsaPubKey);
        skf::ULONG ret = lib_->ExportPublicKey(containerResult.value(), true,
            reinterpret_cast<skf::BYTE*>(&rsaPubKey), &pubKeyLen);
        if (ret != skf::SAR_OK) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_ExportPublicKey"));
        }
        pkeyGuard.pkey = createRsaEvpPKey(rsaPubKey);
    }

    if (!pkeyGuard.pkey) {
        closeContainerHandle(devName, appName, containerName);
        return Result<QByteArray>::err(
            Error(Error::Fail, "从 SKF 公钥创建 EVP_PKEY 失败", "SkfPlugin::generateCsr"));
    }

    // 步骤 3: 使用 OpenSSL 构建 CertificationRequestInfo (TBS)
    QByteArray certReqInfoDer = buildCsrTbs(pkeyGuard.pkey, cname, org, unit, isSm2);
    if (certReqInfoDer.isEmpty()) {
        closeContainerHandle(devName, appName, containerName);
        return Result<QByteArray>::err(
            Error(Error::Fail, "使用 OpenSSL 构建 CSR TBS 失败", "SkfPlugin::generateCsr"));
    }

    // 步骤 4: 使用 SKF 硬件对 TBS 进行签名
    QByteArray signatureValue;

    if (isSm2) {
        // SM2 签名：先计算 SM3 哈希（含 SM2 预处理），再签名
        if (!lib_->DigestInit || !lib_->Digest || !lib_->ECCSignData) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::PluginLoadFailed, "SM2 签名函数不可用", "SkfPlugin::generateCsr"));
        }

        // 获取设备句柄用于哈希
        auto devResult = openDevice(devName);
        if (devResult.isErr()) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(devResult.error());
        }

        // SM3 哈希初始化（含 SM2 预处理）
        skf::HANDLE hHash = nullptr;
        const char* defaultId = "1234567812345678";
        skf::ULONG idLen = 16;
        skf::ULONG ret = lib_->DigestInit(
            devResult.value(), skf::SGD_SM3,
            &eccPubKey,
            const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(defaultId)),
            idLen, &hHash);
        if (ret != skf::SAR_OK || !hHash) {
            closeDevice(devName);
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_DigestInit"));
        }

        // 计算哈希
        QByteArray digest(32, 0);
        skf::ULONG digestLen = 32;
        ret = lib_->Digest(hHash,
            const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(certReqInfoDer.constData())),
            static_cast<skf::ULONG>(certReqInfoDer.size()),
            reinterpret_cast<skf::BYTE*>(digest.data()), &digestLen);
        if (ret != skf::SAR_OK) {
            closeDevice(devName);
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_Digest"));
        }

        // ECC 签名
        skf::ECCSIGNATUREBLOB eccSig;
        std::memset(&eccSig, 0, sizeof(eccSig));
        ret = lib_->ECCSignData(containerResult.value(),
            reinterpret_cast<skf::BYTE*>(digest.data()), 32, &eccSig);
        if (ret != skf::SAR_OK) {
            closeDevice(devName);
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_ECCSignData"));
        }

        // 使用 OpenSSL ECDSA_SIG 进行 DER 编码
        signatureValue = encodeEccSignatureDer(eccSig);
        if (signatureValue.isEmpty()) {
            closeDevice(devName);
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::Fail, "ECC 签名编码失败", "SkfPlugin::generateCsr"));
        }

        // SM2 路径完成，关闭设备句柄
        closeDevice(devName);
    } else {


        // RSA 签名：OpenSSL 软件 SHA-256 哈希（与 Java MessageDigest 对齐）+ PKCS#1 v1.5 DigestInfo + RSASignData
        if (!lib_->RSASignData) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::PluginLoadFailed, "SKF_RSASignData 函数不可用", "SkfPlugin::generateCsr"));
        }

        // 使用 OpenSSL 软件计算 SHA-256（SHA-256 是公开计算，无需走硬件）
        unsigned char sha256Hash[32];
        unsigned int sha256Len = 0;
        EVP_MD_CTX* mdCtx = EVP_MD_CTX_new();
        if (!mdCtx ||
            EVP_DigestInit_ex(mdCtx, EVP_sha256(), nullptr) != 1 ||
            EVP_DigestUpdate(mdCtx, certReqInfoDer.constData(), static_cast<size_t>(certReqInfoDer.size())) != 1 ||
            EVP_DigestFinal_ex(mdCtx, sha256Hash, &sha256Len) != 1) {
            EVP_MD_CTX_free(mdCtx);
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::Fail, "OpenSSL SHA-256 摘要计算失败", "SkfPlugin::generateCsr"));
        }
        EVP_MD_CTX_free(mdCtx);

        QByteArray digest(reinterpret_cast<const char*>(sha256Hash), static_cast<int>(sha256Len));
        qDebug() << "[generateCsr] RSA SHA-256 digest(hex):" << digest.toHex();

        // 构建 PKCS#1 v1.5 DigestInfo（使用与 sign 方法和 Java 端相同的硬编码前缀）
        // DigestInfo ::= SEQUENCE { AlgorithmIdentifier { OID sha256, NULL }, OCTET STRING hash }
        static const unsigned char SHA256_DIGEST_INFO_PREFIX[] = {
            0x30, 0x31,                                     // SEQUENCE (49 bytes)
            0x30, 0x0D,                                     // SEQUENCE (13 bytes) - AlgorithmIdentifier
            0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65,      // OID 2.16.840.1.101.3.4.2.1 (SHA-256)
            0x03, 0x04, 0x02, 0x01,
            0x05, 0x00,                                     // NULL
            0x04, 0x20                                      // OCTET STRING (32 bytes)
        };

        QByteArray digestInfo;
        digestInfo.append(reinterpret_cast<const char*>(SHA256_DIGEST_INFO_PREFIX),
                         sizeof(SHA256_DIGEST_INFO_PREFIX));
        digestInfo.append(digest);

        qDebug() << "[generateCsr] RSA DigestInfo length:" << digestInfo.size();

        // RSA 硬件签名
        // 第一次：获取签名长度
        skf::ULONG rsaSigLen = 0;
        skf::ULONG ret = lib_->RSASignData(containerResult.value(),
            reinterpret_cast<skf::BYTE*>(digestInfo.data()),
            static_cast<skf::ULONG>(digestInfo.size()),
            nullptr, &rsaSigLen);
        if (ret != skf::SAR_OK) {
            qWarning() << "[generateCsr] RSASignData get length failed, ret:" << QString::number(ret, 16);
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_RSASignData(getLen)"));
        }
        if (rsaSigLen == 0) {
            qWarning() << "[generateCsr] RSASignData returned zero length";
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error(Error::Fail, "RSASignData 返回长度为零", "generateCsr"));
        }
        qDebug() << "[generateCsr] RSA expected sig length:" << rsaSigLen;

        // 第二次：真正签名
        QByteArray rsaSig(static_cast<int>(rsaSigLen), 0);
        ret = lib_->RSASignData(containerResult.value(),
            reinterpret_cast<skf::BYTE*>(digestInfo.data()),
            static_cast<skf::ULONG>(digestInfo.size()),
            reinterpret_cast<skf::BYTE*>(rsaSig.data()), &rsaSigLen);
        if (ret != skf::SAR_OK) {
            qWarning() << "[generateCsr] RSASignData sign failed, ret:" << QString::number(ret, 16);
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_RSASignData(sign)"));
        }
        rsaSig.resize(static_cast<int>(rsaSigLen));
        signatureValue = rsaSig;
        qDebug() << "[generateCsr] RSA signature length:" << signatureValue.size();
        qDebug() << "[generateCsr] RSA signature(hex):" << signatureValue.toHex();

        qDebug() << "[generateCsr] RSA signature length:" << signatureValue.size();

    }

    closeContainerHandle(devName, appName, containerName);

    // 步骤 5: 组装最终的 CertificationRequest DER
    QByteArray csrDer = assembleCsrDer(certReqInfoDer, signatureValue, isSm2);
    return Result<QByteArray>::ok(csrDer);
}

//=== 证书管理 ===

Result<void> SkfPlugin::importCert(const QString& devName, const QString& appName, const QString& containerName,
                                    const QByteArray& certData, bool isSignCert) {
    QMutexLocker locker(&mutex_);

    auto containerResult = openContainerHandle(devName, appName, containerName);
    if (containerResult.isErr()) {
        return Result<void>::err(containerResult.error());
    }

    if (!lib_->ImportCertificate) {
        closeContainerHandle(devName, appName, containerName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_ImportCertificate 函数不可用", "SkfPlugin::importCert"));
    }

    skf::ULONG ret = lib_->ImportCertificate(
        containerResult.value(), isSignCert ? 1 : 0,
        const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(certData.constData())),
        static_cast<skf::ULONG>(certData.size()));
    closeContainerHandle(devName, appName, containerName);

    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_ImportCertificate"));
    }

    return Result<void>::ok();
}

/**
 * @brief 将 GMT-0009 ASN.1 格式的 SM2 加密密钥转换为 GMT-0016 ENVELOPEDKEYBLOB 二进制格式
 *
 * GMT-0009 ASN.1 结构:
 * SEQUENCE {
 *   cipherEncPriv  OCTET STRING,   -- 加密的私钥（32 bytes）
 *   encPub         BIT STRING,     -- 加密公钥（65 bytes: 04+X+Y）
 *   cipherSymKey   SEQUENCE {      -- SM2 加密的对称密钥
 *     x            INTEGER,
 *     y            INTEGER,
 *     hash         OCTET STRING,   -- 32 bytes SM3 哈希
 *     cipherTxt    OCTET STRING    -- 密文
 *   }
 * }
 *
 * @param keyData 原始密钥数据
 * @return 成功返回 1024 字节的 ENVELOPEDKEYBLOB 缓冲区，失败返回错误
 */
static Result<QByteArray> parseGmt0009ToEnvelopedKeyBlob(const QByteArray& keyData) {
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(keyData.constData());
    const unsigned char* p = raw;
    long len = keyData.size();

    // 解析外层 SEQUENCE
    long totalLen = 0;
    int tag = 0, cls = 0;
    ASN1_get_object(&p, &totalLen, &tag, &cls, len);
    if (tag != V_ASN1_SEQUENCE) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：外层 SEQUENCE 无效", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    const unsigned char* seqEnd = p + totalLen;

    // 1. cipherEncPriv: OCTET STRING（加密的私钥）
    long objLen = 0;
    ASN1_get_object(&p, &objLen, &tag, &cls, seqEnd - p);
    if (tag != V_ASN1_OCTET_STRING) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：cipherEncPriv 不是 OCTET STRING", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    QByteArray cipherEncPriv(reinterpret_cast<const char*>(p), static_cast<int>(objLen));
    p += objLen;

    // 2. encPub: BIT STRING（加密公钥，04+X+Y）
    ASN1_get_object(&p, &objLen, &tag, &cls, seqEnd - p);
    if (tag != V_ASN1_BIT_STRING) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：encPub 不是 BIT STRING", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    const unsigned char* pubData = p + 1;  // 跳过 unused bits byte
    long pubLen = objLen - 1;
    p += objLen;

    // 3. cipherSymKey: SEQUENCE { x INTEGER, y INTEGER, hash OCTET STRING, cipherTxt OCTET STRING }
    ASN1_get_object(&p, &objLen, &tag, &cls, seqEnd - p);
    if (tag != V_ASN1_SEQUENCE) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：cipherSymKey 不是 SEQUENCE", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    const unsigned char* symSeqEnd = p + objLen;

    // 3.1 x: INTEGER
    ASN1_get_object(&p, &objLen, &tag, &cls, symSeqEnd - p);
    if (tag != V_ASN1_INTEGER) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：x 不是 INTEGER", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    const unsigned char* xData = p;
    long xLen = objLen;
    if (xLen > 0 && xData[0] == 0x00) { xData++; xLen--; }  // 跳过前导 0x00
    p += objLen;

    // 3.2 y: INTEGER
    ASN1_get_object(&p, &objLen, &tag, &cls, symSeqEnd - p);
    if (tag != V_ASN1_INTEGER) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：y 不是 INTEGER", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    const unsigned char* yData = p;
    long yLen = objLen;
    if (yLen > 0 && yData[0] == 0x00) { yData++; yLen--; }
    p += objLen;

    // 3.3 hash: OCTET STRING（32 bytes）
    ASN1_get_object(&p, &objLen, &tag, &cls, symSeqEnd - p);
    if (tag != V_ASN1_OCTET_STRING) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：hash 不是 OCTET STRING", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    QByteArray hashData(reinterpret_cast<const char*>(p), static_cast<int>(objLen));
    p += objLen;

    // 3.4 cipherTxt: OCTET STRING
    ASN1_get_object(&p, &objLen, &tag, &cls, symSeqEnd - p);
    if (tag != V_ASN1_OCTET_STRING) {
        return Result<QByteArray>::err(
            Error(Error::InvalidParam, "SM2 密钥：cipherTxt 不是 OCTET STRING", "parseGmt0009ToEnvelopedKeyBlob"));
    }
    QByteArray cipherTxt(reinterpret_cast<const char*>(p), static_cast<int>(objLen));

    qDebug() << "[parseGmt0009ToEnvelopedKeyBlob] ASN.1 parsed:"
             << "cipherEncPriv:" << cipherEncPriv.size()
             << "pubLen:" << pubLen
             << "x:" << xLen << "y:" << yLen
             << "hash:" << hashData.size()
             << "cipherTxt:" << cipherTxt.size();

    // 构造 GMT-0016 ENVELOPEDKEYBLOB（匹配 Go 的转换逻辑）
    QByteArray blob(1024, 0);
    auto* buf = reinterpret_cast<uint8_t*>(blob.data());

    // offset 0: Version = 1（小端）
    buf[0] = 0x01;
    // offset 4: SymAlgID = SGD_SM4_ECB = 0x00000401（小端）
    buf[4] = 0x01; buf[5] = 0x04;
    // offset 8: ulBits = 256（小端）
    buf[8] = 0x00; buf[9] = 0x01;

    // offset 12+32: cbEncryptedPriKey[64] 后半段放加密私钥密文
    std::memcpy(buf + 44, cipherEncPriv.constData(), qMin(cipherEncPriv.size(), 32));

    // offset 76: ECCPUBLICKEYBLOB.BitLen = 256
    buf[76] = 0x00; buf[77] = 0x01;

    // offset 80+32: xCoordinate, offset 144+32: yCoordinate
    if (pubLen >= 33 && pubData[0] == 0x04) {
        std::memcpy(buf + 112, pubData + 1, 32);     // X
        if (pubLen >= 65) {
            std::memcpy(buf + 176, pubData + 33, 32); // Y
        }
    }

    // offset 208: ECCCIPHERBLOB
    // xCoordinate[64]: 右对齐
    if (xLen > 0) {
        int n = qMin(static_cast<int>(xLen), 32);
        std::memcpy(buf + 240 + (32 - n), xData, n);
    }
    // yCoordinate[64]: 右对齐
    if (yLen > 0) {
        int n = qMin(static_cast<int>(yLen), 32);
        std::memcpy(buf + 304 + (32 - n), yData, n);
    }
    // hash[32]
    std::memcpy(buf + 336, hashData.constData(), qMin(hashData.size(), 32));
    // cipherLen（小端 4 字节）
    uint32_t ctLen = static_cast<uint32_t>(cipherTxt.size());
    buf[368] = ctLen & 0xFF;
    buf[369] = (ctLen >> 8) & 0xFF;
    buf[370] = (ctLen >> 16) & 0xFF;
    buf[371] = (ctLen >> 24) & 0xFF;
    // cipherData
    std::memcpy(buf + 372, cipherTxt.constData(), cipherTxt.size());

    return Result<QByteArray>::ok(blob);
}

Result<void> SkfPlugin::importKeyCert(const QString& devName, const QString& appName, const QString& containerName,
                                       const QByteArray& sigCert, const QByteArray& encCert,
                                       const QByteArray& encPrivate, bool nonGM) {
    QMutexLocker locker(&mutex_);

    qDebug() << "[importKeyCert] devName:" << devName << "appName:" << appName
             << "containerName:" << containerName << "nonGM:" << nonGM
             << "sigCert size:" << sigCert.size()
             << "encCert size:" << encCert.size()
             << "encPrivate size:" << encPrivate.size();

    // PIN 验证（ImportECCKeyPair/ImportRSAKeyPair 需要先验证 PIN）
    QString loginKey = devName + "/" + appName;
    if (!loginCache_.contains(loginKey)) {
        qWarning() << "[importKeyCert] 应用未登录, devName:" << devName << "appName:" << appName;
        return Result<void>::err(
            Error(Error::NotLoggedIn, "应用未登录，请先登录", "SkfPlugin::importKeyCert"));
    }
    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        qWarning() << "[importKeyCert] 打开应用失败:" << appResult.error().message();
        return Result<void>::err(appResult.error());
    }
    const LoginInfo& cached = loginCache_[loginKey];
    if (lib_->VerifyPIN) {
        skf::ULONG pinType = (cached.role.toLower() == "admin") ? 0 : 1;
        QByteArray pinBytes = cached.pin.toLocal8Bit();
        skf::ULONG retryCount = 0;
        skf::ULONG verifyRet = lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
        if (verifyRet != skf::SAR_OK) {
            qWarning() << "[importKeyCert] VerifyPIN 失败, ret:" << QString::number(verifyRet, 16);
            return Result<void>::err(Error::fromSkf(verifyRet, "SKF_VerifyPIN"));
        }
        qDebug() << "[importKeyCert] VerifyPIN 成功, role:" << cached.role;
    }

    // 打开容器（内部会打开设备和应用）
    auto containerResult = openContainerHandle(devName, appName, containerName);
    if (containerResult.isErr()) {
        return Result<void>::err(containerResult.error());
    }
    skf::HCONTAINER hContainer = containerResult.value();

    // 获取容器密钥类型：1=RSA, 2=SM2
    // Go: keyType, _ := s.publicKeyType(hcon, container); nonGM = nonGM || keyType == 2
    skf::ULONG containerType = 0;
    if (lib_->GetContainerType) {
        skf::ULONG ret = lib_->GetContainerType(hContainer, &containerType);
        if (ret != skf::SAR_OK) {
            qWarning() << "[importKeyCert] GetContainerType failed, ret:" << QString::number(ret, 16);
            // 获取失败不阻塞，使用请求参数的 nonGM
        } else {
            qDebug() << "[importKeyCert] containerType:" << containerType << "(1=RSA, 2=SM2)";
            // Go: nonGM = nonGM || keyType == 2  (注意：Go 里 keyType==2 对应非国密/RSA)
            nonGM = nonGM || (containerType == 1);
        }
    }
    qDebug() << "[importKeyCert] final nonGM:" << nonGM;

    // === 导入签名证书 ===
    if (!sigCert.isEmpty()) {
        if (!lib_->ImportCertificate) {
            closeContainerHandle(devName, appName, containerName);
            return Result<void>::err(
                Error(Error::PluginLoadFailed, "SKF_ImportCertificate 函数不可用", "SkfPlugin::importKeyCert"));
        }
        skf::ULONG ret = lib_->ImportCertificate(
            hContainer, 1,
            const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(sigCert.constData())),
            static_cast<skf::ULONG>(sigCert.size()));
        if (ret != skf::SAR_OK) {
            qWarning() << "[importKeyCert] import sigCert failed, ret:" << QString::number(ret, 16);
            closeContainerHandle(devName, appName, containerName);
            return Result<void>::err(Error::fromSkf(ret, "SKF_ImportCertificate(sigCert)"));
        }
        qDebug() << "[importKeyCert] sigCert imported successfully";
    }

    // === 导入加密证书 ===
    if (!encCert.isEmpty()) {
        if (!lib_->ImportCertificate) {
            closeContainerHandle(devName, appName, containerName);
            return Result<void>::err(
                Error(Error::PluginLoadFailed, "SKF_ImportCertificate 函数不可用", "SkfPlugin::importKeyCert"));
        }
        skf::ULONG ret = lib_->ImportCertificate(
            hContainer, 0,
            const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(encCert.constData())),
            static_cast<skf::ULONG>(encCert.size()));
        if (ret != skf::SAR_OK) {
            qWarning() << "[importKeyCert] import encCert failed, ret:" << QString::number(ret, 16);
            closeContainerHandle(devName, appName, containerName);
            return Result<void>::err(Error::fromSkf(ret, "SKF_ImportCertificate(encCert)"));
        }
        qDebug() << "[importKeyCert] encCert imported successfully";
    }

    // === 导入加密私钥 ===
    if (!encPrivate.isEmpty()) {
        if (nonGM) {
            // --- RSA 密钥对导入 ---
            // Go: parseRSAEnvelopedKeyBlob 格式: 小端 4字节 symAlgId + 4字节 wrappedKeyLen + wrappedKey + encryptedData
            if (!lib_->ImportRSAKeyPair) {
                closeContainerHandle(devName, appName, containerName);
                return Result<void>::err(
                    Error(Error::PluginLoadFailed, "SKF_ImportRSAKeyPair 函数不可用", "SkfPlugin::importKeyCert"));
            }

            if (encPrivate.size() < 8) {
                closeContainerHandle(devName, appName, containerName);
                return Result<void>::err(
                    Error(Error::InvalidParam, "RSA 密钥数据过短", "SkfPlugin::importKeyCert"));
            }

            const auto* raw = reinterpret_cast<const uint8_t*>(encPrivate.constData());
            // 小端读取 symAlgId
            skf::ULONG symAlgId = raw[0] | (raw[1] << 8) | (raw[2] << 16) | (raw[3] << 24);
            // 小端读取 wrappedKeyLen
            skf::ULONG wrappedKeyLen = raw[4] | (raw[5] << 8) | (raw[6] << 16) | (raw[7] << 24);

            if (static_cast<int>(8 + wrappedKeyLen) > encPrivate.size()) {
                closeContainerHandle(devName, appName, containerName);
                return Result<void>::err(
                    Error(Error::InvalidParam, "RSA 封装密钥长度溢出", "SkfPlugin::importKeyCert"));
            }

            skf::BYTE* pbWrappedKey = const_cast<skf::BYTE*>(raw + 8);
            skf::ULONG encDataLen = static_cast<skf::ULONG>(encPrivate.size()) - 8 - wrappedKeyLen;
            skf::BYTE* pbEncData = const_cast<skf::BYTE*>(raw + 8 + wrappedKeyLen);

            qDebug() << "[importKeyCert] RSA symAlgId:" << symAlgId
                     << "wrappedKeyLen:" << wrappedKeyLen
                     << "encDataLen:" << encDataLen;

            skf::ULONG ret = lib_->ImportRSAKeyPair(
                hContainer, symAlgId,
                pbWrappedKey, wrappedKeyLen,
                pbEncData, encDataLen);
            if (ret != skf::SAR_OK) {
                qWarning() << "[importKeyCert] SKF_ImportRSAKeyPair failed, ret:" << QString::number(ret, 16);
                closeContainerHandle(devName, appName, containerName);
                return Result<void>::err(Error::fromSkf(ret, "SKF_ImportRSAKeyPair"));
            }
            qDebug() << "[importKeyCert] RSA key pair imported successfully";
        } else {
            // --- SM2 密钥对导入 ---
            // Go: 先尝试 ASN.1 解码 (GMT-0009)，再尝试直接 GMT-0016 小端格式
            if (!lib_->ImportECCKeyPair) {
                closeContainerHandle(devName, appName, containerName);
                return Result<void>::err(
                    Error(Error::PluginLoadFailed, "SKF_ImportECCKeyPair 函数不可用", "SkfPlugin::importKeyCert"));
            }

            // 检查是否为 GMT-0016 小端格式（Go: bytes.HasPrefix(key, {0x01,0x00,0x00,0x00,0x01,0x04,0x00,0x00})）
            static const uint8_t GMT0016_PREFIX[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00};
            const auto* raw = reinterpret_cast<const uint8_t*>(encPrivate.constData());
            bool isGmt0016 = (encPrivate.size() >= 8 &&
                              std::memcmp(raw, GMT0016_PREFIX, sizeof(GMT0016_PREFIX)) == 0);

            QByteArray evpKeyBuf;
            skf::ENVELOPEDKEYBLOB* pEvpKey = nullptr;

            if (isGmt0016) {
                // 直接使用 GMT-0016 格式，复制一份可写副本避免 const_cast 问题
                qDebug() << "[importKeyCert] SM2 key: GMT-0016 format detected, data size:" << encPrivate.size();
                evpKeyBuf = encPrivate;
                pEvpKey = reinterpret_cast<skf::ENVELOPEDKEYBLOB*>(evpKeyBuf.data());
            } else {
                // GMT-0009 ASN.1 → GMT-0016 ENVELOPEDKEYBLOB 转换
                qDebug() << "[importKeyCert] SM2 key: trying ASN.1 (GMT-0009) decode";
                auto blobResult = parseGmt0009ToEnvelopedKeyBlob(encPrivate);
                if (blobResult.isErr()) {
                    closeContainerHandle(devName, appName, containerName);
                    return Result<void>::err(blobResult.error());
                }
                evpKeyBuf = blobResult.value();
                pEvpKey = reinterpret_cast<skf::ENVELOPEDKEYBLOB*>(evpKeyBuf.data());
            }

            qDebug() << "[importKeyCert] SM2 ENVELOPEDKEYBLOB version:" << pEvpKey->version
                     << "symAlgId:" << pEvpKey->ulSymAlgId
                     << "bits:" << pEvpKey->ulBits
                     << "pubKey.bitLen:" << pEvpKey->pubKey.bitLen
                     << "eccCipherBlob.cipherLen:" << pEvpKey->eccCipherBlob.cipherLen;

            skf::ULONG ret = lib_->ImportECCKeyPair(hContainer, pEvpKey);
            if (ret != skf::SAR_OK) {
                qWarning() << "[importKeyCert] SKF_ImportECCKeyPair failed, ret:" << QString::number(ret, 16);
                closeContainerHandle(devName, appName, containerName);
                return Result<void>::err(Error::fromSkf(ret, "SKF_ImportECCKeyPair"));
            }
            qDebug() << "[importKeyCert] SM2 key pair imported successfully";
        }
    }

    closeContainerHandle(devName, appName, containerName);
    return Result<void>::ok();
}

Result<QByteArray> SkfPlugin::exportCert(const QString& devName, const QString& appName,
                                          const QString& containerName, bool isSignCert) {
    QMutexLocker locker(&mutex_);

    auto containerResult = openContainerHandle(devName, appName, containerName);
    if (containerResult.isErr()) {
        return Result<QByteArray>::err(containerResult.error());
    }

    if (!lib_->ExportCertificate) {
        closeContainerHandle(devName, appName, containerName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_ExportCertificate 函数不可用", "SkfPlugin::exportCert"));
    }

    // 获取证书大小
    skf::ULONG certLen = 0;
    skf::ULONG ret = lib_->ExportCertificate(containerResult.value(), isSignCert ? 1 : 0, nullptr, &certLen);
    if (ret != skf::SAR_OK) {
        closeContainerHandle(devName, appName, containerName);
        return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_ExportCertificate"));
    }

    QByteArray certData(static_cast<int>(certLen), '\0');
    ret = lib_->ExportCertificate(containerResult.value(), isSignCert ? 1 : 0,
                                   reinterpret_cast<skf::BYTE*>(certData.data()), &certLen);
    closeContainerHandle(devName, appName, containerName);

    if (ret != skf::SAR_OK) {
        return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_ExportCertificate"));
    }

    certData.resize(static_cast<int>(certLen));
    return Result<QByteArray>::ok(certData);
}

Result<CertInfo> SkfPlugin::getCertInfo(const QString& devName, const QString& appName,
                                         const QString& containerName, bool isSignCert) {
    // 先导出证书数据
    auto certResult = exportCert(devName, appName, containerName, isSignCert);
    if (certResult.isErr()) {
        return Result<CertInfo>::err(certResult.error());
    }

    const QByteArray& certData = certResult.value();
    CertInfo info;
    info.rawData = certData;
    
    // 转换为 PEM 格式
    QString base64Cert = QString::fromLatin1(certData.toBase64());
    // 每 64 字符换行
    QString pemBody;
    for (int i = 0; i < base64Cert.length(); i += 64) {
        pemBody += base64Cert.mid(i, 64) + "\n";
    }
    info.cert = "-----BEGIN CERTIFICATE-----\n" + pemBody + "-----END CERTIFICATE-----\n";
    

    info.certType = isSignCert ? 0 : 1;

    // 解析 X.509 DER 格式证书
    if (certData.size() < 10) {
        return Result<CertInfo>::err(
            Error(Error::InvalidParam, "证书数据过短", "SkfPlugin::getCertInfo"));
    }

    // 计算公钥哈希 (SHA1, 40字符 hex)
    QByteArray hash = QCryptographicHash::hash(certData, QCryptographicHash::Sha1);
    info.pubKeyHash = QString::fromLatin1(hash.toHex());

    // 简化的 ASN.1 DER 解析
    // 实际生产环境应使用 OpenSSL 或专业的 ASN.1 解析库
    auto parseResult = parseDerCertificate(certData);
    if (parseResult.isOk()) {
        const auto& parsed = parseResult.value();
        info.subjectDn = parsed.subjectDn;
        info.commonName = parsed.commonName;
        info.issuerDn = parsed.issuerDn;
        info.serialNumber = parsed.serialNumber;
        info.notBefore = parsed.notBefore;
        info.notAfter = parsed.notAfter;
    } else {
        // 解析失败时使用默认值
        info.subjectDn = "";
        info.commonName = "";
        info.issuerDn = "";
        info.serialNumber = "";
    }

    return Result<CertInfo>::ok(info);
}

//=== 签名验签 ===

Result<QByteArray> SkfPlugin::sign(const QString& devName, const QString& appName, const QString& containerName,
                                    const QByteArray& data) {

    QMutexLocker locker(&mutex_);

    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<QByteArray>::err(devResult.error());
    }

    // 检查登录状态（签名操作需要先登录应用）
    QString loginKey = devName + "/" + appName;
    if (!loginCache_.contains(loginKey)) {
        qWarning() << "[sign] 应用未登录, devName:" << devName << "appName:" << appName;
        return Result<QByteArray>::err(
            Error(Error::NotLoggedIn, "应用未登录，请先登录", "SkfPlugin::sign"));
    }

    // 使用缓存的凭据验证 PIN（每次操作前重新验证）
    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        qWarning() << "[sign] 打开应用失败:" << appResult.error().message();
        return Result<QByteArray>::err(appResult.error());
    }
    const LoginInfo& cached = loginCache_[loginKey];
    if (!lib_->VerifyPIN) {
        closeAppHandle(devName, appName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_VerifyPIN 函数不可用", "SkfPlugin::sign"));
    }
    skf::ULONG pinType = (cached.role.toLower() == "admin") ? 0 : 1;
    QByteArray pinBytes = cached.pin.toLocal8Bit();
    skf::ULONG retryCount = 0;
    skf::ULONG verifyRet = lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
    if (verifyRet != skf::SAR_OK) {
        qWarning() << "[sign] VerifyPIN 失败, ret:" << QString::number(verifyRet, 16);
        closeAppHandle(devName, appName);
        return Result<QByteArray>::err(Error::fromSkf(verifyRet, "SKF_VerifyPIN"));
    }
    qDebug() << "[sign] VerifyPIN 成功, role:" << cached.role;

    // 打开容器（内部会打开设备和应用）
    auto containerResult = openContainerHandle(devName, appName, containerName);
    if (containerResult.isErr()) {
        return Result<QByteArray>::err(containerResult.error());
    }

    // 获取容器类型：1=RSA, 2=SM2
    skf::ULONG containerType = 0;
    if (lib_->GetContainerType) {
        skf::ULONG ret = lib_->GetContainerType(containerResult.value(), &containerType);
        if (ret != skf::SAR_OK) {
            qWarning() << "[sign] GetContainerType failed, ret:" << ret;
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_GetContainerType"));
        }
    } else {
        closeContainerHandle(devName, appName, containerName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_GetContainerType 函数不可用", "SkfPlugin::sign"));
    }

    qDebug() << "[sign] containerType:" << containerType << "(1=RSA, 2=SM2)";

    bool isSm2 = (containerType == 2);

    if (!lib_->DigestInit || !lib_->Digest) {
        closeContainerHandle(devName, appName, containerName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_DigestInit/Digest 函数不可用", "SkfPlugin::sign"));
    }

    if (isSm2) {
        // === SM2 签名：SM3 哈希（含 SM2 预处理）+ ECCSignData ===
        if (!lib_->ECCSignData) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::PluginLoadFailed, "SKF_ECCSignData 函数不可用", "SkfPlugin::sign"));
        }

        // 导出签名公钥用于 SM2 预处理
        skf::ECCPUBLICKEYBLOB pubKey;
        std::memset(&pubKey, 0, sizeof(pubKey));
        skf::ULONG pubKeyLen = sizeof(pubKey);

        if (lib_->ExportPublicKey) {
            skf::ULONG ret = lib_->ExportPublicKey(containerResult.value(), true,
                reinterpret_cast<skf::BYTE*>(&pubKey), &pubKeyLen);
            if (ret != skf::SAR_OK) {
                std::memset(&pubKey, 0, sizeof(pubKey));
            }
        }

        // SM3 哈希初始化（含 SM2 预处理：公钥 + 默认 ID）
        skf::HANDLE hHash = nullptr;
        const char* defaultId = "1234567812345678";
        skf::ULONG idLen = 16;

        skf::ULONG ret = lib_->DigestInit(
            devResult.value(), skf::SGD_SM3,
            pubKey.bitLen > 0 ? &pubKey : nullptr,
            const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(defaultId)),
            idLen, &hHash);
        if (ret != skf::SAR_OK || !hHash) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_DigestInit(SM3)"));
        }

        // 计算 SM3 哈希
        QByteArray digest(32, 0);
        skf::ULONG digestLen = 32;
        ret = lib_->Digest(hHash,
            const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(data.constData())),
            static_cast<skf::ULONG>(data.size()),
            reinterpret_cast<skf::BYTE*>(digest.data()), &digestLen);
        if (ret != skf::SAR_OK) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_Digest(SM3)"));
        }

        // ECC 签名
        skf::ECCSIGNATUREBLOB eccSig;
        std::memset(&eccSig, 0, sizeof(eccSig));
        ret = lib_->ECCSignData(containerResult.value(),
            reinterpret_cast<skf::BYTE*>(digest.data()), 32, &eccSig);
        closeContainerHandle(devName, appName, containerName);

        if (ret != skf::SAR_OK) {
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_ECCSignData"));
        }

        // 使用 OpenSSL ECDSA_SIG 编码为 DER 格式
        QByteArray derSig = encodeEccSignatureDer(eccSig);
        if (derSig.isEmpty()) {
            return Result<QByteArray>::err(
                Error(Error::Fail, "ECC 签名 DER 编码失败", "SkfPlugin::sign"));
        }
        return Result<QByteArray>::ok(derSig);

    } else {
        // === RSA 签名：SKF 硬件 SHA-256 哈希 + PKCS#1 v1.5 DigestInfo + RSASignData ===
        if (!lib_->RSASignData) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(
                Error(Error::PluginLoadFailed, "SKF_RSASignData 函数不可用", "SkfPlugin::sign"));
        }

        // 使用 SKF 硬件计算 SHA-256 哈希（RSA 不需要公钥和 ID）
        skf::HANDLE hHash = nullptr;
        skf::ULONG ret = lib_->DigestInit(
            devResult.value(), skf::SGD_SHA256,
            nullptr,   // RSA 不需要公钥
            nullptr,   // RSA 不需要 ID
            0, &hHash);
        if (ret != skf::SAR_OK || !hHash) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_DigestInit(SHA256)"));
        }

        // 计算 SHA-256 哈希
        QByteArray digest(32, 0);
        skf::ULONG digestLen = 32;
        ret = lib_->Digest(hHash,
            const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(data.constData())),
            static_cast<skf::ULONG>(data.size()),
            reinterpret_cast<skf::BYTE*>(digest.data()), &digestLen);
        if (ret != skf::SAR_OK) {
            closeContainerHandle(devName, appName, containerName);
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_Digest(SHA256)"));
        }
        digest.resize(static_cast<int>(digestLen));

        qDebug() << "[sign] RSA SHA-256 digest(hex):" << digest.toHex();

        // 构建 PKCS#1 v1.5 DigestInfo
        // DigestInfo ::= SEQUENCE { AlgorithmIdentifier { OID sha256, NULL }, OCTET STRING hash }
        static const unsigned char SHA256_DIGEST_INFO_PREFIX[] = {
            0x30, 0x31,                                     // SEQUENCE (49 bytes)
            0x30, 0x0D,                                     // SEQUENCE (13 bytes) - AlgorithmIdentifier
            0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65,      // OID 2.16.840.1.101.3.4.2.1 (SHA-256)
            0x03, 0x04, 0x02, 0x01,
            0x05, 0x00,                                     // NULL
            0x04, 0x20                                      // OCTET STRING (32 bytes)
        };

        QByteArray digestInfo;
        digestInfo.append(reinterpret_cast<const char*>(SHA256_DIGEST_INFO_PREFIX),
                         sizeof(SHA256_DIGEST_INFO_PREFIX));
        digestInfo.append(digest);

        qDebug() << "[sign] RSA DigestInfo length:" << digestInfo.size();

        // RSA 硬件签名
        QByteArray rsaSig(512, 0);  // 最大 4096 位
        skf::ULONG rsaSigLen = static_cast<skf::ULONG>(rsaSig.size());
        ret = lib_->RSASignData(containerResult.value(),
            reinterpret_cast<skf::BYTE*>(digestInfo.data()),
            static_cast<skf::ULONG>(digestInfo.size()),
            reinterpret_cast<skf::BYTE*>(rsaSig.data()), &rsaSigLen);
        closeContainerHandle(devName, appName, containerName);

        if (ret != skf::SAR_OK) {
            return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_RSASignData"));
        }
        rsaSig.resize(static_cast<int>(rsaSigLen));

        qDebug() << "[sign] RSA signature length:" << rsaSigLen;
        return Result<QByteArray>::ok(rsaSig);
    }
}

Result<bool> SkfPlugin::verify(const QString& devName, const QString& appName, const QString& containerName,
                                const QByteArray& data, const QByteArray& signature) {
    QMutexLocker locker(&mutex_);

    // 需要导出公钥来验签
    auto containerResult = openContainerHandle(devName, appName, containerName);
    if (containerResult.isErr()) {
        return Result<bool>::err(containerResult.error());
    }

    if (!lib_->ECCVerify) {
        closeContainerHandle(devName, appName, containerName);
        return Result<bool>::err(
            Error(Error::PluginLoadFailed, "SKF_ECCVerify 函数不可用", "SkfPlugin::verify"));
    }

    // 需要设备句柄和公钥进行验签
    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        closeContainerHandle(devName, appName, containerName);
        return Result<bool>::err(devResult.error());
    }

    // 从签名数据中恢复签名结构
    skf::ECCSIGNATUREBLOB sigBlob;
    std::memset(&sigBlob, 0, sizeof(sigBlob));
    if (static_cast<size_t>(signature.size()) >= sizeof(sigBlob)) {
        std::memcpy(&sigBlob, signature.constData(), sizeof(sigBlob));
    }

    // 注意：实际使用需要先导出公钥，这里简化处理
    skf::ECCPUBLICKEYBLOB pubKey;
    std::memset(&pubKey, 0, sizeof(pubKey));

    skf::ULONG ret = lib_->ECCVerify(
        devResult.value(), &pubKey,
        const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(data.constData())),
        static_cast<skf::ULONG>(data.size()), &sigBlob);

    closeContainerHandle(devName, appName, containerName);
    closeDevice(devName);

    if (ret != skf::SAR_OK) {
        return Result<bool>::ok(false);
    }

    return Result<bool>::ok(true);
}

//=== 文件操作 ===

Result<QStringList> SkfPlugin::enumFiles(const QString& devName, const QString& appName) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<QStringList>::err(appResult.error());
    }

    if (!lib_->EnumFiles) {
        closeAppHandle(devName, appName);
        return Result<QStringList>::err(
            Error(Error::PluginLoadFailed, "SKF_EnumFiles 函数不可用", "SkfPlugin::enumFiles"));
    }

    skf::ULONG size = 0;
    skf::ULONG ret = lib_->EnumFiles(appResult.value(), nullptr, &size);
    if (ret != skf::SAR_OK) {
        closeAppHandle(devName, appName);
        return Result<QStringList>::err(Error::fromSkf(ret, "SKF_EnumFiles"));
    }

    if (size == 0) {
        closeAppHandle(devName, appName);
        return Result<QStringList>::ok({});
    }

    QByteArray buffer(static_cast<int>(size), '\0');
    ret = lib_->EnumFiles(appResult.value(), buffer.data(), &size);
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        return Result<QStringList>::err(Error::fromSkf(ret, "SKF_EnumFiles"));
    }

    return Result<QStringList>::ok(parseNameList(buffer.constData(), size));
}

Result<QByteArray> SkfPlugin::readFile(const QString& devName, const QString& appName, const QString& fileName) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<QByteArray>::err(appResult.error());
    }

    if (!lib_->ReadFile) {
        closeAppHandle(devName, appName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_ReadFile 函数不可用", "SkfPlugin::readFile"));
    }

    QByteArray fileBytes = fileName.toLocal8Bit();

    // 先读取获取大小（用较大缓冲区）
    constexpr skf::ULONG maxSize = 65536;
    QByteArray buffer(maxSize, '\0');
    skf::ULONG outLen = maxSize;

    skf::ULONG ret = lib_->ReadFile(appResult.value(), fileBytes.constData(), 0, maxSize,
                                     reinterpret_cast<skf::BYTE*>(buffer.data()), &outLen);
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_ReadFile"));
    }

    buffer.resize(static_cast<int>(outLen));
    return Result<QByteArray>::ok(buffer);
}

Result<void> SkfPlugin::writeFile(const QString& devName, const QString& appName, const QString& fileName,
                                   const QByteArray& data, int readRights, int writeRights) {
    QMutexLocker locker(&mutex_);

    qDebug() << "[writeFile] devName:" << devName << "appName:" << appName
             << "fileName:" << fileName << "dataSize:" << data.size()
             << "readRights:" << Qt::hex << readRights << "writeRights:" << Qt::hex << writeRights;

    // 检查登录状态（写文件需要先登录应用验证 PIN）
    QString loginKey = devName + "/" + appName;
    if (!loginCache_.contains(loginKey)) {
        qWarning() << "[writeFile] 应用未登录, devName:" << devName << "appName:" << appName;
        return Result<void>::err(
            Error(Error::NotLoggedIn, "应用未登录，请先登录应用", "SkfPlugin::writeFile"));
    }

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<void>::err(appResult.error());
    }

    // 验证 PIN（与 importKeyCert/createContainer 保持一致）
    const LoginInfo& cached = loginCache_[loginKey];
    if (lib_->VerifyPIN) {
        skf::ULONG pinType = (cached.role.toLower() == "admin") ? 0 : 1;
        QByteArray pinBytes = cached.pin.toLocal8Bit();
        skf::ULONG retryCount = 0;
        skf::ULONG verifyRet = lib_->VerifyPIN(appResult.value(), pinType, pinBytes.constData(), &retryCount);
        if (verifyRet != skf::SAR_OK) {
            closeAppHandle(devName, appName);
            qWarning() << "[writeFile] VerifyPIN 失败, ret:" << Qt::hex << verifyRet
                       << "retryCount:" << retryCount;
            return Result<void>::err(Error::fromSkf(verifyRet, "SKF_VerifyPIN"));
        }
        qDebug() << "[writeFile] VerifyPIN 成功, role:" << cached.role;
    }

    if (!lib_->WriteFile) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_WriteFile 函数不可用", "SkfPlugin::writeFile"));
    }

    QByteArray fileBytes = fileName.toLocal8Bit();

    // 先尝试创建文件（若文件已存在则会返回 SAR_FILE_ALREADY_EXIST，此时直接覆盖写入）
    if (lib_->CreateFile) {
        skf::ULONG fileSize = static_cast<skf::ULONG>(data.size());
        // 至少分配 256 字节，避免 0 大小的文件
        if (fileSize < 256) fileSize = 256;

        skf::ULONG createRet = lib_->CreateFile(
            appResult.value(), fileBytes.constData(),
            fileSize,
            static_cast<skf::ULONG>(readRights),
            static_cast<skf::ULONG>(writeRights));
        if (createRet == skf::SAR_OK) {
            qDebug() << "[writeFile] SKF_CreateFile 成功, fileName:" << fileName;
        } else if (createRet == skf::SAR_FILE_ALREADY_EXIST) {
            qDebug() << "[writeFile] 文件已存在，直接覆盖写入, fileName:" << fileName;
        } else {
            closeAppHandle(devName, appName);
            qWarning() << "[writeFile] SKF_CreateFile 失败, ret:" << Qt::hex << createRet;
            return Result<void>::err(Error::fromSkf(createRet, "SKF_CreateFile"));
        }
    } else {
        qWarning() << "[writeFile] SKF_CreateFile 不可用，直接尝试写入";
    }

    // 写入数据
    skf::ULONG ret = lib_->WriteFile(
        appResult.value(), fileBytes.constData(), 0,
        const_cast<skf::BYTE*>(reinterpret_cast<const skf::BYTE*>(data.constData())),
        static_cast<skf::ULONG>(data.size()));
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        qWarning() << "[writeFile] SKF_WriteFile 失败, ret:" << Qt::hex << ret;
        return Result<void>::err(Error::fromSkf(ret, "SKF_WriteFile"));
    }

    qDebug() << "[writeFile] 写入成功, fileName:" << fileName;
    return Result<void>::ok();
}

Result<void> SkfPlugin::deleteFile(const QString& devName, const QString& appName, const QString& fileName) {
    QMutexLocker locker(&mutex_);

    auto appResult = openAppHandle(devName, appName);
    if (appResult.isErr()) {
        return Result<void>::err(appResult.error());
    }

    if (!lib_->DeleteFile) {
        closeAppHandle(devName, appName);
        return Result<void>::err(
            Error(Error::PluginLoadFailed, "SKF_DeleteFile 函数不可用", "SkfPlugin::deleteFile"));
    }

    QByteArray fileBytes = fileName.toLocal8Bit();
    skf::ULONG ret = lib_->DeleteFile(appResult.value(), fileBytes.constData());
    closeAppHandle(devName, appName);

    if (ret != skf::SAR_OK) {
        return Result<void>::err(Error::fromSkf(ret, "SKF_DeleteFile"));
    }

    return Result<void>::ok();
}

//=== 其他 ===

Result<QByteArray> SkfPlugin::generateRandom(const QString& devName, int count) {
    QMutexLocker locker(&mutex_);

    auto devResult = openDevice(devName);
    if (devResult.isErr()) {
        return Result<QByteArray>::err(devResult.error());
    }

    if (!lib_->GenRandom) {
        closeDevice(devName);
        return Result<QByteArray>::err(
            Error(Error::PluginLoadFailed, "SKF_GenRandom 函数不可用", "SkfPlugin::generateRandom"));
    }

    QByteArray buffer(count, '\0');
    skf::ULONG ret = lib_->GenRandom(devResult.value(), reinterpret_cast<skf::BYTE*>(buffer.data()),
                                      static_cast<skf::ULONG>(count));
    closeDevice(devName);

    if (ret != skf::SAR_OK) {
        return Result<QByteArray>::err(Error::fromSkf(ret, "SKF_GenRandom"));
    }

    return Result<QByteArray>::ok(buffer);
}

//=== 证书解析辅助方法 ===

/**
 * @brief 将 X509_NAME 转为可读的 DN 字符串
 * @param name X509_NAME 指针
 * @return DN 字符串，格式如 "CN=xxx, O=yyy, C=zzz"
 */
static QString x509NameToString(X509_NAME* name) {
    if (!name) return {};
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) return {};
    // XN_FLAG_SEP_CPLUS_SPC: 用 ", " 分隔; XN_FLAG_DN_REV: 不反转顺序
    X509_NAME_print_ex(bio, name, 0, XN_FLAG_SEP_CPLUS_SPC & ~XN_FLAG_DN_REV);
    char* buf = nullptr;
    long len = BIO_get_mem_data(bio, &buf);
    QString result = QString::fromUtf8(buf, static_cast<int>(len));
    BIO_free(bio);
    return result;
}

/**
 * @brief 将 ASN1_TIME 转为 QDateTime
 * @param asn1Time ASN1_TIME 指针
 * @return QDateTime（UTC）
 */
static QDateTime asn1TimeToQDateTime(const ASN1_TIME* asn1Time) {
    if (!asn1Time) return {};
    struct tm t;
    std::memset(&t, 0, sizeof(t));
    if (ASN1_TIME_to_tm(asn1Time, &t) != 1) return {};
    // mktime 期望本地时间，这里用 timegm/mkgmtime 处理 UTC
    QDate date(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    QTime time(t.tm_hour, t.tm_min, t.tm_sec);
    return QDateTime(date, time, QTimeZone::utc());
}

Result<SkfPlugin::ParsedCertInfo> SkfPlugin::parseDerCertificate(const QByteArray& certData) const {
    // 使用 OpenSSL d2i_X509 解析 DER 格式证书
    const unsigned char* p = reinterpret_cast<const unsigned char*>(certData.constData());
    X509* x509 = d2i_X509(nullptr, &p, certData.size());
    if (!x509) {
        qWarning() << "[parseDerCertificate] d2i_X509 failed:" << ERR_error_string(ERR_get_error(), nullptr);
        return Result<ParsedCertInfo>::err(
            Error(Error::InvalidParam, "X.509 证书解析失败", "parseDerCertificate"));
    }

    ParsedCertInfo info;

    // 序列号
    const ASN1_INTEGER* sn = X509_get0_serialNumber(x509);
    if (sn) {
        BIGNUM* bnSn = ASN1_INTEGER_to_BN(sn, nullptr);
        if (bnSn) {
            char* hex = BN_bn2hex(bnSn);
            if (hex) {
                info.serialNumber = QString::fromLatin1(hex).toLower();
                OPENSSL_free(hex);
            }
            BN_free(bnSn);
        }
    }

    // 颁发者 DN
    info.issuerDn = x509NameToString(X509_get_issuer_name(x509));

    // 主题 DN 和 CN
    X509_NAME* subject = X509_get_subject_name(x509);
    info.subjectDn = x509NameToString(subject);
    if (subject) {
        int cnIdx = X509_NAME_get_index_by_NID(subject, NID_commonName, -1);
        if (cnIdx >= 0) {
            X509_NAME_ENTRY* entry = X509_NAME_get_entry(subject, cnIdx);
            if (entry) {
                ASN1_STRING* cnAsn1 = X509_NAME_ENTRY_get_data(entry);
                if (cnAsn1) {
                    unsigned char* utf8 = nullptr;
                    int utf8Len = ASN1_STRING_to_UTF8(&utf8, cnAsn1);
                    if (utf8Len > 0 && utf8) {
                        info.commonName = QString::fromUtf8(reinterpret_cast<const char*>(utf8), utf8Len);
                        OPENSSL_free(utf8);
                    }
                }
            }
        }
    }

    // 有效期
    info.notBefore = asn1TimeToQDateTime(X509_get0_notBefore(x509));
    info.notAfter = asn1TimeToQDateTime(X509_get0_notAfter(x509));

    X509_free(x509);

    qDebug() << "[parseDerCertificate] subject:" << info.subjectDn
             << "CN:" << info.commonName
             << "issuer:" << info.issuerDn
             << "serial:" << info.serialNumber
             << "notBefore:" << info.notBefore.toString(Qt::ISODate)
             << "notAfter:" << info.notAfter.toString(Qt::ISODate);

    return Result<ParsedCertInfo>::ok(info);
}

}  // namespace wekey
