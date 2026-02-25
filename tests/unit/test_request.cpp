/**
 * @file test_request.cpp
 * @brief Request DTO 单元测试 (M4.1.4T)
 *
 * 测试所有请求 DTO 的 fromJson() 和 validate() 方法
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "api/dto/Request.h"
#include "common/Error.h"

using namespace wekey;
using namespace wekey::api;

class TestRequest : public QObject {
    Q_OBJECT

private slots:
    // LoginRequest 测试
    void testLoginRequestFromJson();
    void testLoginRequestFromJsonMissingField();
    void testLoginRequestValidate();
    void testLoginRequestValidateInvalidRole();

    // CsrRequest 测试
    void testCsrRequestFromJson();
    void testCsrRequestFromJsonMissingField();
    void testCsrRequestValidate();
    void testCsrRequestValidateInvalidKeyType();

    // ImportCertRequest 测试
    void testImportCertRequestFromJson();
    void testImportCertRequestFromJsonWithEncCert();
    void testImportCertRequestValidate();

    // ExportCertRequest 测试
    void testExportCertRequestFromQuery();
    void testExportCertRequestFromQueryMissing();

    // SignRequest 测试
    void testSignRequestFromJson();
    void testSignRequestValidate();

    // VerifyRequest 测试
    void testVerifyRequestFromJson();
    void testVerifyRequestValidate();

    // RandomRequest 测试
    void testRandomRequestFromJson();
    void testRandomRequestValidateLength();

    // CreateModuleRequest 测试
    void testCreateModuleRequestFromJson();
    void testCreateModuleRequestValidate();

    // ActiveModuleRequest 测试
    void testActiveModuleRequestFromJson();

    // DeleteModuleRequest 测试
    void testDeleteModuleRequestFromJson();

    // ChangeDeviceAuthRequest 测试
    void testChangeDeviceAuthRequestFromJson();

    // SetDeviceLabelRequest 测试
    void testSetDeviceLabelRequestFromJson();

    // CreateAppRequest 测试
    void testCreateAppRequestFromJson();

    // DeleteAppRequest 测试
    void testDeleteAppRequestFromJson();

    // UpdateAppPinRequest 测试
    void testUpdateAppPinRequestFromJson();

    // UnblockAppRequest 测试
    void testUnblockAppRequestFromJson();

    // CreateContainerRequest 测试
    void testCreateContainerRequestFromJson();

    // DeleteContainerRequest 测试
    void testDeleteContainerRequestFromJson();

    // CreateFileRequest 测试
    void testCreateFileRequestFromJson();

    // ReadFileRequest 测试
    void testReadFileRequestFromQuery();

    // DeleteFileRequest 测试
    void testDeleteFileRequestFromJson();

    // SetDefaultsRequest 测试
    void testSetDefaultsRequestFromJson();
};

// ==============================================================================
// LoginRequest 测试
// ==============================================================================

void TestRequest::testLoginRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["role"] = "user";
    json["pin"] = "123456";

    auto result = LoginRequest::fromJson(json);
    QVERIFY(result.isOk());

    LoginRequest req = result.value();
    QCOMPARE(req.serialNumber, QString("SN12345"));
    QCOMPARE(req.appName, QString("TAGM"));
    QCOMPARE(req.role, QString("user"));
    QCOMPARE(req.pin, QString("123456"));
}

void TestRequest::testLoginRequestFromJsonMissingField() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    // 缺少 appName

    auto result = LoginRequest::fromJson(json);
    QVERIFY(result.isErr());
    QCOMPARE(result.error().code(), Error::InvalidParam);
}

void TestRequest::testLoginRequestValidate() {
    LoginRequest req;
    req.serialNumber = "SN12345";
    req.appName = "TAGM";
    req.role = "user";
    req.pin = "123456";

    auto result = req.validate();
    QVERIFY(result.isOk());
}

void TestRequest::testLoginRequestValidateInvalidRole() {
    LoginRequest req;
    req.serialNumber = "SN12345";
    req.appName = "TAGM";
    req.role = "invalid_role";  // 无效角色
    req.pin = "123456";

    auto result = req.validate();
    QVERIFY(result.isErr());
    QCOMPARE(result.error().code(), Error::InvalidParam);
}

// ==============================================================================
// CsrRequest 测试
// ==============================================================================

void TestRequest::testCsrRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";
    json["keyPairType"] = "SM2_sm2p256v1";
    json["cname"] = "Common Name";
    json["org"] = "Organization";
    json["unit"] = "Unit";

    auto result = CsrRequest::fromJson(json);
    QVERIFY(result.isOk());

    CsrRequest req = result.value();
    QCOMPARE(req.serialNumber, QString("SN12345"));
    QCOMPARE(req.containerName, QString("TrustAsia"));
    QCOMPARE(req.keyPairType, QString("SM2_sm2p256v1"));
    QCOMPARE(req.cname, QString("Common Name"));
}

void TestRequest::testCsrRequestFromJsonMissingField() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    // 缺少 containerName

    auto result = CsrRequest::fromJson(json);
    QVERIFY(result.isErr());
}

void TestRequest::testCsrRequestValidate() {
    CsrRequest req;
    req.serialNumber = "SN12345";
    req.appName = "TAGM";
    req.containerName = "TrustAsia";
    req.keyPairType = "SM2_sm2p256v1";
    req.cname = "CN";
    req.org = "Org";
    req.unit = "Unit";

    auto result = req.validate();
    QVERIFY(result.isOk());
}

void TestRequest::testCsrRequestValidateInvalidKeyType() {
    CsrRequest req;
    req.serialNumber = "SN12345";
    req.appName = "TAGM";
    req.containerName = "TrustAsia";
    req.keyPairType = "INVALID_TYPE";  // 无效密钥类型
    req.cname = "CN";
    req.org = "Org";
    req.unit = "Unit";

    auto result = req.validate();
    QVERIFY(result.isErr());
}

// ==============================================================================
// ImportCertRequest 测试
// ==============================================================================

void TestRequest::testImportCertRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";
    json["sigCert"] = "-----BEGIN CERTIFICATE-----\ndata\n-----END CERTIFICATE-----";

    auto result = ImportCertRequest::fromJson(json);
    QVERIFY(result.isOk());

    ImportCertRequest req = result.value();
    QCOMPARE(req.serialNumber, QString("SN12345"));
    QVERIFY(req.sigCert.contains("BEGIN CERTIFICATE"));
    QVERIFY(req.encCert.isEmpty());
}

void TestRequest::testImportCertRequestFromJsonWithEncCert() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";
    json["sigCert"] = "-----BEGIN CERTIFICATE-----\ndata\n-----END CERTIFICATE-----";
    json["encCert"] = "-----BEGIN CERTIFICATE-----\nenc_data\n-----END CERTIFICATE-----";
    json["nonGM"] = false;

    auto result = ImportCertRequest::fromJson(json);
    QVERIFY(result.isOk());

    ImportCertRequest req = result.value();
    QVERIFY(req.encCert.contains("enc_data"));
    QCOMPARE(req.nonGM, false);
}

void TestRequest::testImportCertRequestValidate() {
    ImportCertRequest req;
    req.serialNumber = "SN12345";
    req.appName = "TAGM";
    req.containerName = "TrustAsia";
    req.sigCert = "-----BEGIN CERTIFICATE-----\ndata\n-----END CERTIFICATE-----";

    auto result = req.validate();
    QVERIFY(result.isOk());

    // 三个字段都为空时应返回错误
    ImportCertRequest emptyReq;
    emptyReq.serialNumber = "SN12345";
    emptyReq.appName = "TAGM";
    emptyReq.containerName = "TrustAsia";
    auto emptyResult = emptyReq.validate();
    QVERIFY(emptyResult.isErr());
}

// ==============================================================================
// ExportCertRequest 测试
// ==============================================================================

void TestRequest::testExportCertRequestFromQuery() {
    QMap<QString, QString> query;
    query["serialNumber"] = "SN12345";
    query["appName"] = "TAGM";
    query["containerName"] = "TrustAsia";

    auto result = ExportCertRequest::fromQuery(query);
    QVERIFY(result.isOk());

    ExportCertRequest req = result.value();
    QCOMPARE(req.serialNumber, QString("SN12345"));
    QCOMPARE(req.appName, QString("TAGM"));
    QCOMPARE(req.containerName, QString("TrustAsia"));
}

void TestRequest::testExportCertRequestFromQueryMissing() {
    QMap<QString, QString> query;
    query["serialNumber"] = "SN12345";
    // 缺少 appName

    auto result = ExportCertRequest::fromQuery(query);
    QVERIFY(result.isErr());
}

// ==============================================================================
// SignRequest 测试
// ==============================================================================

void TestRequest::testSignRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";
    json["data"] = "SGVsbG8gV29ybGQ=";  // "Hello World" in base64

    auto result = SignRequest::fromJson(json);
    QVERIFY(result.isOk());

    SignRequest req = result.value();
    QCOMPARE(req.serialNumber, QString("SN12345"));
    QCOMPARE(req.data, QString("SGVsbG8gV29ybGQ="));
}

void TestRequest::testSignRequestValidate() {
    SignRequest req;
    req.serialNumber = "SN12345";
    req.appName = "TAGM";
    req.containerName = "TrustAsia";
    req.data = "SGVsbG8gV29ybGQ=";

    auto result = req.validate();
    QVERIFY(result.isOk());
}

// ==============================================================================
// VerifyRequest 测试
// ==============================================================================

void TestRequest::testVerifyRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";
    json["data"] = "SGVsbG8gV29ybGQ=";
    json["signature"] = "3045022100abcd...";

    auto result = VerifyRequest::fromJson(json);
    QVERIFY(result.isOk());

    VerifyRequest req = result.value();
    QCOMPARE(req.data, QString("SGVsbG8gV29ybGQ="));
    QCOMPARE(req.signature, QString("3045022100abcd..."));
}

void TestRequest::testVerifyRequestValidate() {
    VerifyRequest req;
    req.serialNumber = "SN12345";
    req.appName = "TAGM";
    req.containerName = "TrustAsia";
    req.data = "SGVsbG8gV29ybGQ=";
    req.signature = "3045022100abcd...";

    auto result = req.validate();
    QVERIFY(result.isOk());
}

// ==============================================================================
// RandomRequest 测试
// ==============================================================================

void TestRequest::testRandomRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["count"] = 32;

    auto result = RandomRequest::fromJson(json);
    QVERIFY(result.isOk());

    RandomRequest req = result.value();
    QCOMPARE(req.serialNumber, QString("SN12345"));
    QCOMPARE(req.count, 32);
}

void TestRequest::testRandomRequestValidateLength() {
    // 测试有效长度
    RandomRequest req1;
    req1.serialNumber = "SN12345";
    req1.count = 32;
    QVERIFY(req1.validate().isOk());

    // 测试 count <= 0 使用默认值，不报错
    RandomRequest req2;
    req2.serialNumber = "SN12345";
    req2.count = 0;
    QVERIFY(req2.validate().isOk());

    // 测试无效长度（太大）
    RandomRequest req3;
    req3.serialNumber = "SN12345";
    req3.count = 10000;
    QVERIFY(req3.validate().isErr());
}

// ==============================================================================
// Admin Request 测试
// ==============================================================================

void TestRequest::testCreateModuleRequestFromJson() {
    QJsonObject json;
    json["modName"] = "TestModule";
    json["modPath"] = "/path/to/module.so";

    auto result = CreateModuleRequest::fromJson(json);
    QVERIFY(result.isOk());

    CreateModuleRequest req = result.value();
    QCOMPARE(req.modName, QString("TestModule"));
    QCOMPARE(req.modPath, QString("/path/to/module.so"));
}

void TestRequest::testCreateModuleRequestValidate() {
    CreateModuleRequest req;
    req.modName = "TestModule";
    req.modPath = "/path/to/module.so";

    auto result = req.validate();
    QVERIFY(result.isOk());
}

void TestRequest::testActiveModuleRequestFromJson() {
    QJsonObject json;
    json["modName"] = "TestModule";

    auto result = ActiveModuleRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().modName, QString("TestModule"));
}

void TestRequest::testDeleteModuleRequestFromJson() {
    QJsonObject json;
    json["modName"] = "TestModule";

    auto result = DeleteModuleRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().modName, QString("TestModule"));
}

void TestRequest::testChangeDeviceAuthRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["oldPin"] = "123456";
    json["newPin"] = "654321";

    auto result = ChangeDeviceAuthRequest::fromJson(json);
    QVERIFY(result.isOk());

    ChangeDeviceAuthRequest req = result.value();
    QCOMPARE(req.oldPin, QString("123456"));
    QCOMPARE(req.newPin, QString("654321"));
}

void TestRequest::testSetDeviceLabelRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["label"] = "MyDevice";

    auto result = SetDeviceLabelRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().label, QString("MyDevice"));
}

void TestRequest::testCreateAppRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["adminPin"] = "12345678";
    json["userPin"] = "123456";

    auto result = CreateAppRequest::fromJson(json);
    QVERIFY(result.isOk());

    CreateAppRequest req = result.value();
    QCOMPARE(req.appName, QString("TAGM"));
    QCOMPARE(req.adminPin, QString("12345678"));
    QCOMPARE(req.userPin, QString("123456"));
}

void TestRequest::testDeleteAppRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";

    auto result = DeleteAppRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().appName, QString("TAGM"));
}

void TestRequest::testUpdateAppPinRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["role"] = "user";
    json["oldPin"] = "123456";
    json["newPin"] = "654321";

    auto result = UpdateAppPinRequest::fromJson(json);
    QVERIFY(result.isOk());

    UpdateAppPinRequest req = result.value();
    QCOMPARE(req.role, QString("user"));
    QCOMPARE(req.oldPin, QString("123456"));
    QCOMPARE(req.newPin, QString("654321"));
}

void TestRequest::testUnblockAppRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["adminPin"] = "12345678";

    auto result = UnblockAppRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().adminPin, QString("12345678"));
}

void TestRequest::testCreateContainerRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";

    auto result = CreateContainerRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().containerName, QString("TrustAsia"));
}

void TestRequest::testDeleteContainerRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";

    auto result = DeleteContainerRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().containerName, QString("TrustAsia"));
}

void TestRequest::testCreateFileRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["fileName"] = "test.dat";
    json["size"] = 1024;

    auto result = CreateFileRequest::fromJson(json);
    QVERIFY(result.isOk());

    CreateFileRequest req = result.value();
    QCOMPARE(req.fileName, QString("test.dat"));
    QCOMPARE(req.size, 1024);
}

void TestRequest::testReadFileRequestFromQuery() {
    QMap<QString, QString> query;
    query["serialNumber"] = "SN12345";
    query["appName"] = "TAGM";
    query["fileName"] = "test.dat";

    auto result = ReadFileRequest::fromQuery(query);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().fileName, QString("test.dat"));
}

void TestRequest::testDeleteFileRequestFromJson() {
    QJsonObject json;
    json["serialNumber"] = "SN12345";
    json["appName"] = "TAGM";
    json["fileName"] = "test.dat";

    auto result = DeleteFileRequest::fromJson(json);
    QVERIFY(result.isOk());
    QCOMPARE(result.value().fileName, QString("test.dat"));
}

void TestRequest::testSetDefaultsRequestFromJson() {
    QJsonObject json;
    json["appName"] = "TAGM";
    json["containerName"] = "TrustAsia";
    json["commonName"] = "CN";
    json["organization"] = "Org";
    json["unit"] = "Unit";
    json["role"] = "user";

    auto result = SetDefaultsRequest::fromJson(json);
    QVERIFY(result.isOk());

    SetDefaultsRequest req = result.value();
    QCOMPARE(req.appName, QString("TAGM"));
    QCOMPARE(req.containerName, QString("TrustAsia"));
    QCOMPARE(req.role, QString("user"));
}

QTEST_MAIN(TestRequest)
#include "test_request.moc"
