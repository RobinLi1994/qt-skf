/**
 * @file test_httptypes.cpp
 * @brief HttpTypes 单元测试 (M4.1.2T)
 *
 * 测试 HTTP 基础类型：HttpMethod, HttpRequest, HttpResponse
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "api/dto/HttpTypes.h"

using namespace wekey::api;

class TestHttpTypes : public QObject {
    Q_OBJECT

private slots:
    // HttpRequest 测试
    void testHttpRequestJsonBody();
    void testHttpRequestJsonBodyInvalid();
    void testHttpRequestQuery();
    void testHttpRequestQueryDefault();
    void testHttpRequestHeader();
    void testHttpRequestHeaderDefault();

    // HttpResponse 测试
    void testHttpResponseSetJson();
    void testHttpResponseSetError();
    void testHttpResponseSetSuccess();
    void testHttpResponseSetSuccessWithData();

    // HttpMethod 测试
    void testHttpMethodToString();
    void testStringToHttpMethod();
};

// ==============================================================================
// HttpRequest 测试
// ==============================================================================

void TestHttpTypes::testHttpRequestJsonBody() {
    HttpRequest req;
    req.body = R"({"name": "test", "value": 123})";

    auto result = req.jsonBody();
    QVERIFY(result.isOk());

    QJsonObject obj = result.value();
    QCOMPARE(obj["name"].toString(), QString("test"));
    QCOMPARE(obj["value"].toInt(), 123);
}

void TestHttpTypes::testHttpRequestJsonBodyInvalid() {
    HttpRequest req;
    req.body = R"({"invalid json)";

    auto result = req.jsonBody();
    QVERIFY(result.isErr());
    QCOMPARE(result.error().code(), wekey::Error::InvalidParam);
}

void TestHttpTypes::testHttpRequestQuery() {
    HttpRequest req;
    req.queryParams["serialNumber"] = "12345";
    req.queryParams["appName"] = "TAGM";

    QCOMPARE(req.query("serialNumber"), QString("12345"));
    QCOMPARE(req.query("appName"), QString("TAGM"));
}

void TestHttpTypes::testHttpRequestQueryDefault() {
    HttpRequest req;
    req.queryParams["existing"] = "value";

    QCOMPARE(req.query("existing", "default"), QString("value"));
    QCOMPARE(req.query("missing", "default"), QString("default"));
    QCOMPARE(req.query("missing"), QString());
}

void TestHttpTypes::testHttpRequestHeader() {
    HttpRequest req;
    req.headers["Content-Type"] = "application/json";
    req.headers["Authorization"] = "Bearer token123";

    QCOMPARE(req.header("Content-Type"), QString("application/json"));
    QCOMPARE(req.header("Authorization"), QString("Bearer token123"));
}

void TestHttpTypes::testHttpRequestHeaderDefault() {
    HttpRequest req;
    req.headers["Existing"] = "value";

    QCOMPARE(req.header("Existing", "default"), QString("value"));
    QCOMPARE(req.header("Missing", "default"), QString("default"));
    QCOMPARE(req.header("Missing"), QString());
}

// ==============================================================================
// HttpResponse 测试
// ==============================================================================

void TestHttpTypes::testHttpResponseSetJson() {
    HttpResponse resp;
    QJsonObject data;
    data["key"] = "value";
    data["number"] = 42;

    resp.setJson(data);

    QCOMPARE(resp.statusCode, 200);
    QCOMPARE(resp.headers["Content-Type"], QString("application/json; charset=utf-8"));

    QJsonDocument doc = QJsonDocument::fromJson(resp.body.toUtf8());
    QVERIFY(doc.isObject());
    QCOMPARE(doc["key"].toString(), QString("value"));
    QCOMPARE(doc["number"].toInt(), 42);
}

void TestHttpTypes::testHttpResponseSetError() {
    HttpResponse resp;
    wekey::Error error(wekey::Error::InvalidParam, "参数无效", "TestFunction");

    resp.setError(error);

    QCOMPARE(resp.statusCode, 400);
    QCOMPARE(resp.headers["Content-Type"], QString("application/json; charset=utf-8"));

    QJsonDocument doc = QJsonDocument::fromJson(resp.body.toUtf8());
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();

    QCOMPARE(obj["code"].toInt(), static_cast<int>(wekey::Error::InvalidParam));
    QVERIFY(obj["message"].toString().contains("参数无效"));
    QVERIFY(obj["data"].isNull());
}

void TestHttpTypes::testHttpResponseSetSuccess() {
    HttpResponse resp;

    resp.setSuccess();

    QCOMPARE(resp.statusCode, 200);
    QCOMPARE(resp.headers["Content-Type"], QString("application/json; charset=utf-8"));

    QJsonDocument doc = QJsonDocument::fromJson(resp.body.toUtf8());
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();

    QCOMPARE(obj["code"].toInt(), 0);
    QCOMPARE(obj["message"].toString(), QString("success"));
    QVERIFY(obj["data"].isNull());
}

void TestHttpTypes::testHttpResponseSetSuccessWithData() {
    HttpResponse resp;
    QJsonObject data;
    data["result"] = "ok";
    data["count"] = 10;

    resp.setSuccess(data);

    QCOMPARE(resp.statusCode, 200);

    QJsonDocument doc = QJsonDocument::fromJson(resp.body.toUtf8());
    QJsonObject obj = doc.object();

    QCOMPARE(obj["code"].toInt(), 0);
    QCOMPARE(obj["message"].toString(), QString("success"));
    QVERIFY(obj["data"].isObject());

    QJsonObject dataObj = obj["data"].toObject();
    QCOMPARE(dataObj["result"].toString(), QString("ok"));
    QCOMPARE(dataObj["count"].toInt(), 10);
}

// ==============================================================================
// HttpMethod 测试
// ==============================================================================

void TestHttpTypes::testHttpMethodToString() {
    QCOMPARE(httpMethodToString(HttpMethod::GET), QString("GET"));
    QCOMPARE(httpMethodToString(HttpMethod::POST), QString("POST"));
    QCOMPARE(httpMethodToString(HttpMethod::PUT), QString("PUT"));
    QCOMPARE(httpMethodToString(HttpMethod::DELETE), QString("DELETE"));
    QCOMPARE(httpMethodToString(HttpMethod::PATCH), QString("PATCH"));
    QCOMPARE(httpMethodToString(HttpMethod::HEAD), QString("HEAD"));
    QCOMPARE(httpMethodToString(HttpMethod::OPTIONS), QString("OPTIONS"));
}

void TestHttpTypes::testStringToHttpMethod() {
    QCOMPARE(stringToHttpMethod("GET"), HttpMethod::GET);
    QCOMPARE(stringToHttpMethod("POST"), HttpMethod::POST);
    QCOMPARE(stringToHttpMethod("PUT"), HttpMethod::PUT);
    QCOMPARE(stringToHttpMethod("DELETE"), HttpMethod::DELETE);
    QCOMPARE(stringToHttpMethod("PATCH"), HttpMethod::PATCH);
    QCOMPARE(stringToHttpMethod("HEAD"), HttpMethod::HEAD);
    QCOMPARE(stringToHttpMethod("OPTIONS"), HttpMethod::OPTIONS);

    // 测试小写
    QCOMPARE(stringToHttpMethod("get"), HttpMethod::GET);
    QCOMPARE(stringToHttpMethod("post"), HttpMethod::POST);

    // 测试未知方法
    QCOMPARE(stringToHttpMethod("UNKNOWN"), HttpMethod::GET);  // 默认返回 GET
}

QTEST_MAIN(TestHttpTypes)
#include "test_httptypes.moc"
