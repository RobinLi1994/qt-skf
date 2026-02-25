/**
 * @file MockPlugin.h
 * @brief 集成测试用 Mock 驱动插件
 *
 * 实现 IDriverPlugin 全部虚方法，使用内存数据结构模拟 SKF 硬件行为。
 * Header-only 实现，供所有集成测试共享。
 */

#pragma once

#include <QList>
#include <QMap>
#include <QMutex>
#include <QWaitCondition>
#include <optional>

#include "common/Result.h"
#include "plugin/interface/IDriverPlugin.h"
#include "plugin/interface/PluginTypes.h"

namespace wekey {
namespace test {

class MockPlugin : public IDriverPlugin {
public:
    // === 预置数据 ===
    QList<DeviceInfo> devices_;
    QMap<QString, QList<AppInfo>> apps_;        // devName -> apps
    QMap<QString, QList<ContainerInfo>> containers_;  // "dev/app" -> containers
    QMap<QString, int> retryCount_;             // "dev/app/role" -> retry count
    QMap<QString, QString> pins_;               // "dev/app/role" -> pin
    QMap<QString, bool> loggedIn_;              // "dev/app" -> logged in
    QMap<QString, QByteArray> files_;           // "dev/app/file" -> data
    QMap<QString, QByteArray> certs_;           // "dev/app/container/sign|enc" -> cert data
    QMap<QString, QByteArray> keys_;            // "dev/app/container" -> public key
    QMap<QString, QByteArray> signatures_;      // last signature per container

    // === 错误注入 ===
    std::optional<Error> nextError_;

    // === 设备事件队列 ===
    QMutex eventMutex_;
    QWaitCondition eventCondition_;
    QList<int> eventQueue_;
    bool eventError_ = false;

    // === 辅助方法 ===

    void addDevice(const QString& name) {
        DeviceInfo info;
        info.deviceName = name;
        info.manufacturer = "MockVendor";
        info.serialNumber = "MOCK-" + name;
        devices_.append(info);
    }

    void addApp(const QString& devName, const QString& appName, const QString& userPin = "123456",
                const QString& adminPin = "admin123") {
        AppInfo info;
        info.appName = appName;
        apps_[devName].append(info);
        pins_[devName + "/" + appName + "/user"] = userPin;
        pins_[devName + "/" + appName + "/admin"] = adminPin;
        retryCount_[devName + "/" + appName + "/user"] = 10;
        retryCount_[devName + "/" + appName + "/admin"] = 10;
    }

    void addContainer(const QString& devName, const QString& appName, const QString& containerName) {
        ContainerInfo info;
        info.containerName = containerName;
        containers_[devName + "/" + appName].append(info);
    }

    void injectEvent(int event) {
        QMutexLocker lock(&eventMutex_);
        eventQueue_.append(event);
        eventCondition_.wakeOne();
    }

    void injectEventError() {
        QMutexLocker lock(&eventMutex_);
        eventError_ = true;
        eventCondition_.wakeOne();
    }

    // === IDriverPlugin 实现 ===

    Result<QList<DeviceInfo>> enumDevices(bool /*login*/) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QList<DeviceInfo>>::err(e); }
        return Result<QList<DeviceInfo>>::ok(devices_);
    }

    Result<void> changeDeviceAuth(const QString& /*devName*/, const QString& /*oldPin*/,
                                  const QString& /*newPin*/) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        return Result<void>::ok();
    }

    Result<void> setDeviceLabel(const QString& devName, const QString& label) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        for (auto& d : devices_) {
            if (d.deviceName == devName) { d.label = label; break; }
        }
        return Result<void>::ok();
    }

    Result<int> waitForDeviceEvent() override {
        QMutexLocker lock(&eventMutex_);
        while (eventQueue_.isEmpty() && !eventError_) {
            eventCondition_.wait(&eventMutex_, 500);
            if (eventQueue_.isEmpty() && !eventError_) {
                return Result<int>::ok(static_cast<int>(DeviceEvent::None));
            }
        }
        if (eventError_) {
            eventError_ = false;
            return Result<int>::err(Error(Error::Fail, "Device event error", "MockPlugin::waitForDeviceEvent"));
        }
        int event = eventQueue_.takeFirst();
        return Result<int>::ok(event);
    }

    Result<QList<AppInfo>> enumApps(const QString& devName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QList<AppInfo>>::err(e); }
        return Result<QList<AppInfo>>::ok(apps_.value(devName));
    }

    Result<void> createApp(const QString& devName, const QString& appName, const QVariantMap& /*args*/) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        AppInfo info;
        info.appName = appName;
        apps_[devName].append(info);
        pins_[devName + "/" + appName + "/user"] = "123456";
        pins_[devName + "/" + appName + "/admin"] = "admin123";
        retryCount_[devName + "/" + appName + "/user"] = 10;
        retryCount_[devName + "/" + appName + "/admin"] = 10;
        return Result<void>::ok();
    }

    Result<void> deleteApp(const QString& devName, const QString& appName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        auto& list = apps_[devName];
        list.erase(std::remove_if(list.begin(), list.end(),
                   [&](const AppInfo& a) { return a.appName == appName; }), list.end());
        return Result<void>::ok();
    }

    Result<void> openApp(const QString& devName, const QString& appName, const QString& role,
                         const QString& pin) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        QString key = devName + "/" + appName + "/" + role;
        int& retry = retryCount_[key];
        if (retry <= 0) {
            return Result<void>::err(Error(Error::SkfPinLocked, "PIN locked", "MockPlugin::openApp"));
        }
        if (pins_.value(key) != pin) {
            --retry;
            return Result<void>::err(Error(Error::SkfPinIncorrect, "PIN incorrect", "MockPlugin::openApp"));
        }
        retry = 10;
        loggedIn_[devName + "/" + appName] = true;
        return Result<void>::ok();
    }

    Result<void> closeApp(const QString& devName, const QString& appName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        loggedIn_[devName + "/" + appName] = false;
        return Result<void>::ok();
    }

    Result<void> changePin(const QString& devName, const QString& appName, const QString& role,
                           const QString& oldPin, const QString& newPin) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        QString key = devName + "/" + appName + "/" + role;
        if (pins_.value(key) != oldPin) {
            return Result<void>::err(Error(Error::SkfPinIncorrect, "Old PIN incorrect", "MockPlugin::changePin"));
        }
        pins_[key] = newPin;
        return Result<void>::ok();
    }

    Result<void> unlockPin(const QString& devName, const QString& appName, const QString& adminPin,
                           const QString& newUserPin, const QVariantMap& /*args*/) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        QString adminKey = devName + "/" + appName + "/admin";
        if (pins_.value(adminKey) != adminPin) {
            return Result<void>::err(Error(Error::SkfPinIncorrect, "Admin PIN incorrect", "MockPlugin::unlockPin"));
        }
        QString userKey = devName + "/" + appName + "/user";
        pins_[userKey] = newUserPin;
        retryCount_[userKey] = 10;
        return Result<void>::ok();
    }

    Result<int> getRetryCount(const QString& devName, const QString& appName, const QString& role) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<int>::err(e); }
        QString key = devName + "/" + appName + "/" + role;
        return Result<int>::ok(retryCount_.value(key, 10));
    }

    Result<QList<ContainerInfo>> enumContainers(const QString& devName, const QString& appName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QList<ContainerInfo>>::err(e); }
        return Result<QList<ContainerInfo>>::ok(containers_.value(devName + "/" + appName));
    }

    Result<void> createContainer(const QString& devName, const QString& appName,
                                 const QString& containerName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        ContainerInfo info;
        info.containerName = containerName;
        containers_[devName + "/" + appName].append(info);
        return Result<void>::ok();
    }

    Result<void> deleteContainer(const QString& devName, const QString& appName,
                                 const QString& containerName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        auto& list = containers_[devName + "/" + appName];
        list.erase(std::remove_if(list.begin(), list.end(),
                   [&](const ContainerInfo& c) { return c.containerName == containerName; }), list.end());
        return Result<void>::ok();
    }

    Result<QByteArray> generateKeyPair(const QString& devName, const QString& appName,
                                       const QString& containerName, const QString& /*keyType*/) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QByteArray>::err(e); }
        QByteArray pubKey = "MOCK_PUBKEY_" + containerName.toUtf8();
        keys_[devName + "/" + appName + "/" + containerName] = pubKey;
        return Result<QByteArray>::ok(pubKey);
    }

    Result<void> importCert(const QString& devName, const QString& appName, const QString& containerName,
                            const QByteArray& certData, bool isSignCert) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        QString suffix = isSignCert ? "/sign" : "/enc";
        certs_[devName + "/" + appName + "/" + containerName + suffix] = certData;
        return Result<void>::ok();
    }

    Result<QByteArray> exportCert(const QString& devName, const QString& appName, const QString& containerName,
                                  bool isSignCert) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QByteArray>::err(e); }
        QString suffix = isSignCert ? "/sign" : "/enc";
        QString key = devName + "/" + appName + "/" + containerName + suffix;
        if (!certs_.contains(key)) {
            return Result<QByteArray>::err(Error(Error::NotFound, "Certificate not found", "MockPlugin::exportCert"));
        }
        return Result<QByteArray>::ok(certs_.value(key));
    }

    Result<CertInfo> getCertInfo(const QString& devName, const QString& appName, const QString& containerName,
                                 bool isSignCert) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<CertInfo>::err(e); }
        QString suffix = isSignCert ? "/sign" : "/enc";
        QString key = devName + "/" + appName + "/" + containerName + suffix;
        if (!certs_.contains(key)) {
            return Result<CertInfo>::err(Error(Error::NotFound, "Certificate not found", "MockPlugin::getCertInfo"));
        }
        CertInfo info;
        info.subjectDn = "CN=Mock," + containerName;
        info.commonName = "Mock";
        info.issuerDn = "CN=MockCA";
        info.certType = isSignCert ? 0 : 1;
        info.rawData = certs_.value(key);
        return Result<CertInfo>::ok(info);
    }

    Result<QByteArray> sign(const QString& devName, const QString& appName, const QString& containerName,
                            const QByteArray& data, const QString& /*algorithm*/) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QByteArray>::err(e); }
        QByteArray sig = "SIG_" + data.toHex();
        signatures_[devName + "/" + appName + "/" + containerName] = sig;
        return Result<QByteArray>::ok(sig);
    }

    Result<bool> verify(const QString& /*devName*/, const QString& /*appName*/, const QString& /*containerName*/,
                        const QByteArray& data, const QByteArray& signature, const QString& /*algorithm*/) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<bool>::err(e); }
        QByteArray expected = "SIG_" + data.toHex();
        return Result<bool>::ok(signature == expected);
    }

    Result<QStringList> enumFiles(const QString& devName, const QString& appName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QStringList>::err(e); }
        QStringList result;
        QString prefix = devName + "/" + appName + "/";
        for (auto it = files_.begin(); it != files_.end(); ++it) {
            if (it.key().startsWith(prefix)) {
                result.append(it.key().mid(prefix.length()));
            }
        }
        return Result<QStringList>::ok(result);
    }

    Result<QByteArray> readFile(const QString& devName, const QString& appName, const QString& fileName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QByteArray>::err(e); }
        QString key = devName + "/" + appName + "/" + fileName;
        if (!files_.contains(key)) {
            return Result<QByteArray>::err(Error(Error::NotFound, "File not found", "MockPlugin::readFile"));
        }
        return Result<QByteArray>::ok(files_.value(key));
    }

    Result<void> writeFile(const QString& devName, const QString& appName, const QString& fileName,
                           const QByteArray& data) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        files_[devName + "/" + appName + "/" + fileName] = data;
        return Result<void>::ok();
    }

    Result<void> deleteFile(const QString& devName, const QString& appName, const QString& fileName) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<void>::err(e); }
        files_.remove(devName + "/" + appName + "/" + fileName);
        return Result<void>::ok();
    }

    Result<QByteArray> generateRandom(const QString& /*devName*/, int count) override {
        if (nextError_) { auto e = *nextError_; nextError_.reset(); return Result<QByteArray>::err(e); }
        QByteArray data(count, '\0');
        for (int i = 0; i < count; ++i) {
            data[i] = static_cast<char>(i % 256);
        }
        return Result<QByteArray>::ok(data);
    }
};

}  // namespace test
}  // namespace wekey
