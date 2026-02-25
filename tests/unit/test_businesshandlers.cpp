/**
 * @file test_businesshandlers.cpp
 * @brief 业务接口处理器单元测试 (M4.5)
 *
 * 测试所有业务接口：设备枚举、登录、CSR、证书、签名、验签、随机数
 * 由于无真实设备，测试无激活模块时返回错误
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "api/handlers/BusinessHandlers.h"

using namespace wekey;
using namespace wekey::api;

class TestBusinessHandlers : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /**
     * @brief GET /api/v1/enum-dev 无激活模块返回错误
     */
    void testEnumDevNoModule() {
        HttpRequest req;
        req.method = HttpMethod::GET;
        req.path = "/api/v1/enum-dev";

        auto resp = BusinessHandlers::handleEnumDev(req);
        QCOMPARE(resp.statusCode, 200);

        auto doc = QJsonDocument::fromJson(resp.body.toUtf8());
        auto json = doc.object();
        QCOMPARE(json["code"].toInt(), static_cast<int>(Error::NoActiveModule));
    }

    /**
     * @brief POST /api/v1/login 无激活模块返回错误
     */
    void testLoginNoModule() {
        QJsonObject body;
        body["serialNumber"] = "SN12345";
        body["appName"] = "TAGM";
        body["role"] = "user";
        body["pin"] = "123456";

        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/api/v1/login";
        req.body = QString::fromUtf8(QJsonDocument(body).toJson(QJsonDocument::Compact));

        auto resp = BusinessHandlers::handleLogin(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QVERIFY(json["code"].toInt() != 0);
    }

    /**
     * @brief POST /api/v1/login 参数校验失败
     */
    void testLoginInvalidParams() {
        QJsonObject body;
        // 缺少必填字段
        body["serialNumber"] = "SN12345";

        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/api/v1/login";
        req.body = QString::fromUtf8(QJsonDocument(body).toJson(QJsonDocument::Compact));

        auto resp = BusinessHandlers::handleLogin(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QCOMPARE(json["code"].toInt(), static_cast<int>(Error::InvalidParam));
    }

    /**
     * @brief POST /api/v1/csr 无激活模块返回错误
     */
    void testGenCsrNoModule() {
        QJsonObject body;
        body["serialNumber"] = "SN12345";
        body["appName"] = "TAGM";
        body["containerName"] = "TrustAsia";
        body["keyPairType"] = "SM2_sm2p256v1";
        body["cname"] = "CN";

        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/api/v1/csr";
        req.body = QString::fromUtf8(QJsonDocument(body).toJson(QJsonDocument::Compact));

        auto resp = BusinessHandlers::handleGenCsr(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QVERIFY(json["code"].toInt() != 0);
    }

    /**
     * @brief POST /api/v1/import-cert 无激活模块返回错误
     */
    void testImportCertNoModule() {
        QJsonObject body;
        body["serialNumber"] = "SN12345";
        body["appName"] = "TAGM";
        body["containerName"] = "TrustAsia";
        body["cert"] = "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----";

        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/api/v1/import-cert";
        req.body = QString::fromUtf8(QJsonDocument(body).toJson(QJsonDocument::Compact));

        auto resp = BusinessHandlers::handleImportCert(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QVERIFY(json["code"].toInt() != 0);
    }

    /**
     * @brief GET /api/v1/export-cert 无激活模块返回错误
     */
    void testExportCertNoModule() {
        HttpRequest req;
        req.method = HttpMethod::GET;
        req.path = "/api/v1/export-cert";
        req.queryParams["serialNumber"] = "SN12345";
        req.queryParams["appName"] = "TAGM";
        req.queryParams["containerName"] = "TrustAsia";

        auto resp = BusinessHandlers::handleExportCert(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QVERIFY(json["code"].toInt() != 0);
    }

    /**
     * @brief POST /api/v1/sign 无激活模块返回错误
     */
    void testSignNoModule() {
        QJsonObject body;
        body["serialNumber"] = "SN12345";
        body["appName"] = "TAGM";
        body["containerName"] = "TrustAsia";
        body["data"] = "dGVzdA==";

        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/api/v1/sign";
        req.body = QString::fromUtf8(QJsonDocument(body).toJson(QJsonDocument::Compact));

        auto resp = BusinessHandlers::handleSign(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QVERIFY(json["code"].toInt() != 0);
    }


    /**
     * @brief POST /api/v1/random 无激活模块返回错误
     */
    void testRandomNoModule() {
        QJsonObject body;
        body["serialNumber"] = "SN12345";
        body["length"] = 32;

        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/api/v1/random";
        req.body = QString::fromUtf8(QJsonDocument(body).toJson(QJsonDocument::Compact));

        auto resp = BusinessHandlers::handleRandom(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QVERIFY(json["code"].toInt() != 0);
    }

    /**
     * @brief POST /api/v1/sign 无效 JSON body 返回错误
     */
    void testSignInvalidBody() {
        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/api/v1/sign";
        req.body = "not json";

        auto resp = BusinessHandlers::handleSign(req);
        auto json = QJsonDocument::fromJson(resp.body.toUtf8()).object();
        QCOMPARE(json["code"].toInt(), static_cast<int>(Error::InvalidParam));
    }
};

QTEST_MAIN(TestBusinessHandlers)
#include "test_businesshandlers.moc"
