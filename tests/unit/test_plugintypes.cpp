/**
 * @file test_plugintypes.cpp
 * @brief PluginTypes 数据类型单元测试
 *
 * 测试用例 M1.5.3T
 */

#include <QtTest>

#include "plugin/interface/PluginTypes.h"

using namespace wekey;

class TestPluginTypes : public QObject {
    Q_OBJECT

private slots:
    /**
     * @brief 测试 DeviceInfo 结构体
     */
    void testDeviceInfo() {
        DeviceInfo info;
        info.deviceName = "TestDevice";
        info.devicePath = "/dev/usb0";
        info.manufacturer = "TrustAsia";
        info.label = "MyDevice";
        info.serialNumber = "SN123456";
        info.hardwareVersion = "1.0.0";
        info.firmwareVersion = "2.0.0";
        info.isLoggedIn = true;

        QCOMPARE(info.deviceName, QString("TestDevice"));
        QCOMPARE(info.devicePath, QString("/dev/usb0"));
        QCOMPARE(info.manufacturer, QString("TrustAsia"));
        QCOMPARE(info.label, QString("MyDevice"));
        QCOMPARE(info.serialNumber, QString("SN123456"));
        QCOMPARE(info.hardwareVersion, QString("1.0.0"));
        QCOMPARE(info.firmwareVersion, QString("2.0.0"));
        QCOMPARE(info.isLoggedIn, true);
    }

    /**
     * @brief 测试 DeviceInfo 默认值
     */
    void testDeviceInfoDefaults() {
        DeviceInfo info;

        QVERIFY(info.deviceName.isEmpty());
        QVERIFY(info.devicePath.isEmpty());
        QCOMPARE(info.isLoggedIn, false);
    }

    /**
     * @brief 测试 AppInfo 结构体
     */
    void testAppInfo() {
        AppInfo info;
        info.appName = "TAGM";
        info.isLoggedIn = true;

        QCOMPARE(info.appName, QString("TAGM"));
        QCOMPARE(info.isLoggedIn, true);
    }

    /**
     * @brief 测试 AppInfo 默认值
     */
    void testAppInfoDefaults() {
        AppInfo info;

        QVERIFY(info.appName.isEmpty());
        QCOMPARE(info.isLoggedIn, false);
    }

    /**
     * @brief 测试 ContainerInfo 结构体
     */
    void testContainerInfo() {
        ContainerInfo info;
        info.containerName = "TrustAsia";
        info.keyGenerated = true;
        info.keyType = ContainerInfo::KeyType::SM2;
        info.certImported = true;

        QCOMPARE(info.containerName, QString("TrustAsia"));
        QCOMPARE(info.keyGenerated, true);
        QCOMPARE(info.keyType, ContainerInfo::KeyType::SM2);
        QCOMPARE(info.certImported, true);
    }

    /**
     * @brief 测试 ContainerInfo 默认值
     */
    void testContainerInfoDefaults() {
        ContainerInfo info;

        QVERIFY(info.containerName.isEmpty());
        QCOMPARE(info.keyGenerated, false);
        QCOMPARE(info.keyType, ContainerInfo::KeyType::Unknown);
        QCOMPARE(info.certImported, false);
    }

    /**
     * @brief 测试 CertInfo 结构体
     */
    void testCertInfo() {
        CertInfo info;
        info.subjectDn = "CN=Test, O=TrustAsia";
        info.commonName = "Test";
        info.issuerDn = "CN=CA, O=TrustAsia";
        info.serialNumber = "123456789";
        info.notBefore = QDateTime(QDate(2024, 1, 1), QTime(0, 0, 0));
        info.notAfter = QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0));
        info.certType = 1;
        info.pubKeyHash = "abc123";
        info.cert = "base64cert";

        QCOMPARE(info.subjectDn, QString("CN=Test, O=TrustAsia"));
        QCOMPARE(info.commonName, QString("Test"));
        QCOMPARE(info.issuerDn, QString("CN=CA, O=TrustAsia"));
        QCOMPARE(info.serialNumber, QString("123456789"));
        QCOMPARE(info.notBefore.date().year(), 2024);
        QCOMPARE(info.notAfter.date().year(), 2025);
        QCOMPARE(info.certType, 1);
    }

    /**
     * @brief 测试 CertInfo 默认值
     */
    void testCertInfoDefaults() {
        CertInfo info;

        QVERIFY(info.subjectDn.isEmpty());
        QVERIFY(info.issuerDn.isEmpty());
        QCOMPARE(info.certType, 0);
    }

    /**
     * @brief 测试 DeviceEvent 枚举
     */
    void testDeviceEvent() {
        QCOMPARE(static_cast<int>(DeviceEvent::None), 0);
        QCOMPARE(static_cast<int>(DeviceEvent::Inserted), 1);
        QCOMPARE(static_cast<int>(DeviceEvent::Removed), 2);
    }

    /**
     * @brief 测试 ContainerInfo::KeyType 枚举
     */
    void testKeyType() {
        QCOMPARE(static_cast<int>(ContainerInfo::KeyType::Unknown), 0);
        QCOMPARE(static_cast<int>(ContainerInfo::KeyType::RSA), 1);
        QCOMPARE(static_cast<int>(ContainerInfo::KeyType::SM2), 2);
    }

    /**
     * @brief 测试 Q_DECLARE_METATYPE 注册
     */
    void testMetaTypes() {
        // 验证类型已注册
        int deviceInfoId = qMetaTypeId<DeviceInfo>();
        int appInfoId = qMetaTypeId<AppInfo>();
        int containerInfoId = qMetaTypeId<ContainerInfo>();
        int certInfoId = qMetaTypeId<CertInfo>();

        QVERIFY(deviceInfoId != 0);
        QVERIFY(appInfoId != 0);
        QVERIFY(containerInfoId != 0);
        QVERIFY(certInfoId != 0);
    }

    /**
     * @brief 测试 DeviceInfo 拷贝
     */
    void testDeviceInfoCopy() {
        DeviceInfo info1;
        info1.deviceName = "Device1";
        info1.label = "Label1";
        info1.isLoggedIn = true;

        DeviceInfo info2 = info1;
        QCOMPARE(info2.deviceName, QString("Device1"));
        QCOMPARE(info2.label, QString("Label1"));
        QCOMPARE(info2.isLoggedIn, true);
    }

    /**
     * @brief 测试 ContainerInfo 列表
     */
    void testContainerInfoList() {
        QList<ContainerInfo> list;

        ContainerInfo c1;
        c1.containerName = "Container1";
        c1.keyType = ContainerInfo::KeyType::RSA;
        list.append(c1);

        ContainerInfo c2;
        c2.containerName = "Container2";
        c2.keyType = ContainerInfo::KeyType::SM2;
        list.append(c2);

        QCOMPARE(list.size(), 2);
        QCOMPARE(list[0].containerName, QString("Container1"));
        QCOMPARE(list[1].keyType, ContainerInfo::KeyType::SM2);
    }
};

QTEST_MAIN(TestPluginTypes)
#include "test_plugintypes.moc"
