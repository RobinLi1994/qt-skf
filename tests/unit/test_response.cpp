/**
 * @file test_response.cpp
 * @brief Response DTO 单元测试 (M4.1.3T)
 *
 * 测试 ApiResponse 模板和转换辅助函数
 */

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "api/dto/Response.h"
#include "common/Error.h"
#include "common/Result.h"
#include "plugin/interface/PluginTypes.h"

using namespace wekey;
using namespace wekey::api;

class TestResponse : public QObject {
    Q_OBJECT

private slots:
    // ApiResponse 基础测试
    void testApiResponseSuccess();
    void testApiResponseError();
    void testApiResponseToJson();
    void testApiResponseVoidSuccess();
    void testApiResponseVoidError();

    // 转换辅助函数测试
    void testDeviceInfoToJson();
    void testDeviceInfoListToJson();
    void testAppInfoToJson();
    void testAppInfoListToJson();
    void testContainerInfoToJson();
    void testContainerInfoListToJson();
    void testCertInfoToJson();
    void testCertInfoListToJson();

    // fromResult 测试
    void testFromResultSuccess();
    void testFromResultError();
};

// ==============================================================================
// ApiResponse 基础测试
// ==============================================================================

void TestResponse::testApiResponseSuccess() {
    QString data = "test data";
    auto response = ApiResponse<QString>::success(data);

    QVERIFY(response.isSuccess());
    QCOMPARE(response.code(), 0);
    QCOMPARE(response.message(), QString("success"));
    QCOMPARE(response.data(), QString("test data"));
}

void TestResponse::testApiResponseError() {
    Error error(Error::InvalidParam, "参数无效", "TestFunction");
    auto response = ApiResponse<QString>::error(error);

    QVERIFY(!response.isSuccess());
    QCOMPARE(response.code(), static_cast<int>(Error::InvalidParam));
    QVERIFY(response.message().contains("参数无效"));
}

void TestResponse::testApiResponseToJson() {
    QString data = "test value";
    auto response = ApiResponse<QString>::success(data);

    QJsonObject json = response.toJson([](const QString& str) {
        return QJsonValue(str);
    });

    QCOMPARE(json["code"].toInt(), 0);
    QCOMPARE(json["message"].toString(), QString("success"));
    QCOMPARE(json["data"].toString(), QString("test value"));
}

void TestResponse::testApiResponseVoidSuccess() {
    auto response = ApiResponse<void>::success();

    QVERIFY(response.isSuccess());
    QCOMPARE(response.code(), 0);
    QCOMPARE(response.message(), QString("success"));

    QJsonObject json = response.toJson();
    QCOMPARE(json["code"].toInt(), 0);
    QVERIFY(json["data"].isNull());
}

void TestResponse::testApiResponseVoidError() {
    Error error(Error::NotFound, "未找到", "TestFunction");
    auto response = ApiResponse<void>::error(error);

    QVERIFY(!response.isSuccess());
    QCOMPARE(response.code(), static_cast<int>(Error::NotFound));

    QJsonObject json = response.toJson();
    QCOMPARE(json["code"].toInt(), static_cast<int>(Error::NotFound));
    QVERIFY(json["data"].isNull());
}

// ==============================================================================
// 转换辅助函数测试
// ==============================================================================

void TestResponse::testDeviceInfoToJson() {
    DeviceInfo info;
    info.deviceName = "TestDevice";
    info.serialNumber = "SN12345";
    info.manufacturer = "TestManufacturer";
    info.label = "TestLabel";
    info.hardwareVersion = "1.0";
    info.firmwareVersion = "2.0";
    info.isLoggedIn = true;

    QJsonObject json = deviceInfoToJson(info);

    QCOMPARE(json["deviceName"].toString(), QString("TestDevice"));
    QCOMPARE(json["serialNumber"].toString(), QString("SN12345"));
    QCOMPARE(json["manufacturer"].toString(), QString("TestManufacturer"));
    QCOMPARE(json["label"].toString(), QString("TestLabel"));
    QCOMPARE(json["hwVersion"].toString(), QString("1.0"));
    QCOMPARE(json["firmwareVersion"].toString(), QString("2.0"));
    QCOMPARE(json["isLogin"].toBool(), true);
}

void TestResponse::testDeviceInfoListToJson() {
    QList<DeviceInfo> devices;

    DeviceInfo dev1;
    dev1.deviceName = "Device1";
    dev1.serialNumber = "SN001";
    devices.append(dev1);

    DeviceInfo dev2;
    dev2.deviceName = "Device2";
    dev2.serialNumber = "SN002";
    devices.append(dev2);

    QJsonArray json = deviceInfoListToJson(devices);

    QCOMPARE(json.size(), 2);
    QCOMPARE(json[0].toObject()["deviceName"].toString(), QString("Device1"));
    QCOMPARE(json[1].toObject()["deviceName"].toString(), QString("Device2"));
}

void TestResponse::testAppInfoToJson() {
    AppInfo info;
    info.appName = "TAGM";
    info.isLoggedIn = true;

    QJsonObject json = appInfoToJson(info);

    QCOMPARE(json["appName"].toString(), QString("TAGM"));
    QCOMPARE(json["isLogin"].toBool(), true);
}

void TestResponse::testAppInfoListToJson() {
    QList<AppInfo> apps;

    AppInfo app1;
    app1.appName = "App1";
    app1.isLoggedIn = true;
    apps.append(app1);

    AppInfo app2;
    app2.appName = "App2";
    app2.isLoggedIn = false;
    apps.append(app2);

    QJsonArray json = appInfoListToJson(apps);

    QCOMPARE(json.size(), 2);
    QCOMPARE(json[0].toObject()["appName"].toString(), QString("App1"));
    QCOMPARE(json[0].toObject()["isLogin"].toBool(), true);
    QCOMPARE(json[1].toObject()["isLogin"].toBool(), false);
}

void TestResponse::testContainerInfoToJson() {
    ContainerInfo info;
    info.containerName = "TrustAsia";
    info.keyGenerated = true;
    info.keyType = ContainerInfo::KeyType::SM2;
    info.certImported = true;

    QJsonObject json = containerInfoToJson(info);

    QCOMPARE(json["containerName"].toString(), QString("TrustAsia"));
    QCOMPARE(json["keyGenerated"].toBool(), true);
    QCOMPARE(json["keyType"].toInt(), 2);  // SM2 = 2
    QCOMPARE(json["certImported"].toBool(), true);
}

void TestResponse::testContainerInfoListToJson() {
    QList<ContainerInfo> containers;

    ContainerInfo c1;
    c1.containerName = "Container1";
    c1.keyType = ContainerInfo::KeyType::RSA;
    containers.append(c1);

    ContainerInfo c2;
    c2.containerName = "Container2";
    c2.keyType = ContainerInfo::KeyType::SM2;
    containers.append(c2);

    QJsonArray json = containerInfoListToJson(containers);

    QCOMPARE(json.size(), 2);
    QCOMPARE(json[0].toObject()["keyType"].toInt(), 1);  // RSA = 1
    QCOMPARE(json[1].toObject()["keyType"].toInt(), 2);  // SM2 = 2
}

void TestResponse::testCertInfoToJson() {
    CertInfo info;
    info.subjectDn = "CN=Test, O=TrustAsia";
    info.commonName = "Test";
    info.issuerDn = "CN=CA, O=TrustAsia";
    info.serialNumber = "123456";
    info.notBefore = QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate);
    info.notAfter = QDateTime::fromString("2025-01-01T00:00:00", Qt::ISODate);
    info.certType = 1;
    info.pubKeyHash = "abc123";
    info.cert = "base64cert";
    info.rawData = QByteArray("cert data");

    QJsonObject json = certInfoToJson(info);

    QCOMPARE(json["subjectDn"].toString(), QString("CN=Test, O=TrustAsia"));
    QCOMPARE(json["commonName"].toString(), QString("Test"));
    QCOMPARE(json["issuerDn"].toString(), QString("CN=CA, O=TrustAsia"));
    QCOMPARE(json["serialNumber"].toString(), QString("123456"));
    QCOMPARE(json["certType"].toInt(), 1);
    QVERIFY(json.contains("validity"));
}

void TestResponse::testCertInfoListToJson() {
    QList<CertInfo> certs;

    CertInfo cert1;
    cert1.subjectDn = "CN=Cert1";
    cert1.certType = 1;
    certs.append(cert1);

    CertInfo cert2;
    cert2.subjectDn = "CN=Cert2";
    cert2.certType = 2;
    certs.append(cert2);

    QJsonArray json = certInfoListToJson(certs);

    QCOMPARE(json.size(), 2);
    QCOMPARE(json[0].toObject()["subjectDn"].toString(), QString("CN=Cert1"));
    QCOMPARE(json[1].toObject()["subjectDn"].toString(), QString("CN=Cert2"));
}

// ==============================================================================
// fromResult 测试
// ==============================================================================

void TestResponse::testFromResultSuccess() {
    Result<int> result = Result<int>::ok(42);
    auto response = ApiResponse<int>::fromResult(result);

    QVERIFY(response.isSuccess());
    QCOMPARE(response.code(), 0);
    QCOMPARE(response.data(), 42);
}

void TestResponse::testFromResultError() {
    Error error(Error::NotFound, "未找到资源", "TestFunction");
    Result<int> result = Result<int>::err(error);
    auto response = ApiResponse<int>::fromResult(result);

    QVERIFY(!response.isSuccess());
    QCOMPARE(response.code(), static_cast<int>(Error::NotFound));
    QVERIFY(response.message().contains("未找到资源"));
}

QTEST_MAIN(TestResponse)
#include "test_response.moc"
