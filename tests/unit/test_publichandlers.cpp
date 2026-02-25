/**
 * @file test_publichandlers.cpp
 * @brief 公共接口处理器单元测试 (M4.4.2T)
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

#include "api/handlers/PublicHandlers.h"

using namespace wekey;
using namespace wekey::api;

class TestPublicHandlers : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /**
     * @brief GET /health 返回 {"code":0,"message":"success","data":{"status":"ok","version":"1.0.0"}}
     */
    void testHealth() {
        HttpRequest req;
        req.method = HttpMethod::GET;
        req.path = "/health";

        auto resp = PublicHandlers::handleHealth(req);
        QCOMPARE(resp.statusCode, 200);

        auto doc = QJsonDocument::fromJson(resp.body.toUtf8());
        QVERIFY(doc.isObject());
        auto json = doc.object();
        QCOMPARE(json["code"].toInt(), 0);

        auto data = json["data"].toObject();
        QCOMPARE(data["status"].toString(), QString("ok"));
        QCOMPARE(data["version"].toString(), QString("1.0.0"));
    }

    /**
     * @brief GET /exit 触发退出信号
     */
    void testExit() {
        PublicHandlers handlers;
        QSignalSpy spy(&handlers, &PublicHandlers::exitRequested);
        QVERIFY(spy.isValid());

        HttpRequest req;
        req.method = HttpMethod::GET;
        req.path = "/exit";

        auto resp = handlers.handleExit(req);
        QCOMPARE(resp.statusCode, 200);

        auto doc = QJsonDocument::fromJson(resp.body.toUtf8());
        auto json = doc.object();
        QCOMPARE(json["code"].toInt(), 0);

        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(TestPublicHandlers)
#include "test_publichandlers.moc"
