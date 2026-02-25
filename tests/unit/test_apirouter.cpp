/**
 * @file test_apirouter.cpp
 * @brief ApiRouter 单元测试 (M4.3.1T)
 */

#include <QTest>

#include "api/ApiRouter.h"
#include "api/dto/HttpTypes.h"
#include "api/handlers/PublicHandlers.h"

using namespace wekey;
using namespace wekey::api;

class TestApiRouter : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void init() {
        router_ = new ApiRouter();
    }

    void cleanup() {
        delete router_;
        router_ = nullptr;
    }

    /**
     * @brief 测试添加路由成功
     */
    void testAddRoute() {
        bool called = false;
        router_->addRoute(HttpMethod::GET, "/test",
                          [&called](const HttpRequest& /*req*/) -> HttpResponse {
                              called = true;
                              HttpResponse resp;
                              resp.setSuccess();
                              return resp;
                          });

        HttpRequest req;
        req.method = HttpMethod::GET;
        req.path = "/test";
        auto resp = router_->handleRequest(req);
        QVERIFY(called);
        QCOMPARE(resp.statusCode, 200);
    }

    /**
     * @brief 测试路由匹配正确处理器
     */
    void testHandleRequest() {
        int calledHandler = 0;

        router_->addRoute(HttpMethod::GET, "/api/v1/devices",
                          [&calledHandler](const HttpRequest& /*req*/) -> HttpResponse {
                              calledHandler = 1;
                              HttpResponse resp;
                              resp.setSuccess();
                              return resp;
                          });

        router_->addRoute(HttpMethod::POST, "/api/v1/login",
                          [&calledHandler](const HttpRequest& /*req*/) -> HttpResponse {
                              calledHandler = 2;
                              HttpResponse resp;
                              resp.setSuccess();
                              return resp;
                          });

        HttpRequest req1;
        req1.method = HttpMethod::GET;
        req1.path = "/api/v1/devices";
        router_->handleRequest(req1);
        QCOMPARE(calledHandler, 1);

        HttpRequest req2;
        req2.method = HttpMethod::POST;
        req2.path = "/api/v1/login";
        router_->handleRequest(req2);
        QCOMPARE(calledHandler, 2);
    }

    /**
     * @brief 测试未匹配路由返回 404
     */
    void testHandleRequestNotFound() {
        HttpRequest req;
        req.method = HttpMethod::GET;
        req.path = "/nonexistent";
        auto resp = router_->handleRequest(req);
        QCOMPARE(resp.statusCode, 404);
    }

    /**
     * @brief 测试方法不匹配返回 405
     */
    void testHandleRequestMethodNotAllowed() {
        router_->addRoute(HttpMethod::GET, "/test",
                          [](const HttpRequest& /*req*/) -> HttpResponse {
                              HttpResponse resp;
                              resp.setSuccess();
                              return resp;
                          });

        HttpRequest req;
        req.method = HttpMethod::POST;
        req.path = "/test";
        auto resp = router_->handleRequest(req);
        QCOMPARE(resp.statusCode, 405);
    }

    /**
     * @brief setupRoutes 注册所有路由后，请求不应返回 404
     */
    void testSetupRoutes() {
        PublicHandlers pub;
        router_->setupRoutes(&pub);

        struct RouteCase {
            HttpMethod method;
            QString path;
        };

        QList<RouteCase> cases = {
            {HttpMethod::GET, "/health"},
            {HttpMethod::GET, "/exit"},
            {HttpMethod::GET, "/api/v1/enum-dev"},
            {HttpMethod::POST, "/api/v1/login"},
            {HttpMethod::POST, "/api/v1/csr"},
            {HttpMethod::POST, "/api/v1/import-cert"},
            {HttpMethod::GET, "/api/v1/export-cert"},
            {HttpMethod::POST, "/api/v1/sign"},
            {HttpMethod::POST, "/api/v1/verify"},
            {HttpMethod::POST, "/api/v1/random"},
            {HttpMethod::GET, "/admin/mod/list"},
            {HttpMethod::POST, "/admin/mod/create"},
            {HttpMethod::POST, "/admin/mod/active"},
            {HttpMethod::DELETE, "/admin/mod/delete"},
            {HttpMethod::GET, "/admin/dev/list"},
            {HttpMethod::POST, "/admin/dev/change-auth"},
            {HttpMethod::POST, "/admin/dev/set-label"},
            {HttpMethod::GET, "/admin/app/list"},
            {HttpMethod::POST, "/admin/app/create"},
            {HttpMethod::DELETE, "/admin/app/delete"},
            {HttpMethod::POST, "/admin/app/login"},
            {HttpMethod::POST, "/admin/app/logout"},
            {HttpMethod::POST, "/admin/app/update-pin"},
            {HttpMethod::POST, "/admin/app/unblock"},
            {HttpMethod::GET, "/admin/container/list"},
            {HttpMethod::POST, "/admin/container/create"},
            {HttpMethod::DELETE, "/admin/container/delete"},
            {HttpMethod::POST, "/admin/container/gen-csr"},
            {HttpMethod::POST, "/admin/container/import-cert"},
            {HttpMethod::GET, "/admin/container/export-cert"},
            {HttpMethod::POST, "/admin/container/verify"},
            {HttpMethod::GET, "/admin/file/list"},
            {HttpMethod::POST, "/admin/file/create"},
            {HttpMethod::GET, "/admin/file/read"},
            {HttpMethod::DELETE, "/admin/file/delete"},
            {HttpMethod::GET, "/admin/settings/defaults"},
            {HttpMethod::POST, "/admin/settings/defaults"},
        };

        for (const auto& tc : cases) {
            HttpRequest req;
            req.method = tc.method;
            req.path = tc.path;
            auto resp = router_->handleRequest(req);
            QVERIFY2(resp.statusCode != 404,
                      qPrintable(QString("Route not found: %1 %2")
                                     .arg(httpMethodToString(tc.method), tc.path)));
        }
    }

private:
    ApiRouter* router_ = nullptr;
};

QTEST_MAIN(TestApiRouter)
#include "test_apirouter.moc"
