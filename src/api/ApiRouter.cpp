/**
 * @file ApiRouter.cpp
 * @brief API 路由器实现 (M4.3.3I)
 */

#include "ApiRouter.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "handlers/BusinessHandlers.h"
#include "handlers/PublicHandlers.h"

namespace wekey {
namespace api {

namespace {

HttpResponse makeErrorResponse(int statusCode, const QString& statusText, const QString& message) {
    HttpResponse resp;
    resp.statusCode = statusCode;
    resp.statusText = statusText;
    resp.headers["Content-Type"] = "application/json; charset=utf-8";

    QJsonObject body;
    body["code"] = statusCode;
    body["message"] = message;
    body["data"] = QJsonValue::Null;

    QJsonDocument doc(body);
    resp.body = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return resp;
}

}  // namespace

void ApiRouter::setupRoutes(PublicHandlers* publicHandlers) {
    // Public
    addRoute(HttpMethod::GET, "/health", PublicHandlers::handleHealth);
    addRoute(HttpMethod::GET, "/exit",
             [publicHandlers](const HttpRequest& req) { return publicHandlers->handleExit(req); });

    // Business
    addRoute(HttpMethod::GET, "/api/v1/enum-dev", BusinessHandlers::handleEnumDev);
    addRoute(HttpMethod::POST, "/api/v1/login", BusinessHandlers::handleLogin);
    addRoute(HttpMethod::POST, "/api/v1/logout", BusinessHandlers::handleLogout);
    addRoute(HttpMethod::POST, "/api/v1/csr", BusinessHandlers::handleGenCsr);
    addRoute(HttpMethod::POST, "/api/v1/import-cert", BusinessHandlers::handleImportCert);
    addRoute(HttpMethod::GET, "/api/v1/export-cert", BusinessHandlers::handleExportCert);
    addRoute(HttpMethod::POST, "/api/v1/sign", BusinessHandlers::handleSign);
    addRoute(HttpMethod::POST, "/api/v1/random", BusinessHandlers::handleRandom);

}

void ApiRouter::addRoute(HttpMethod method, const QString& path, RouteHandler handler) {
    routes_[makeKey(method, path)] = std::move(handler);
    knownPaths_[path] = true;
}

HttpResponse ApiRouter::handleRequest(const HttpRequest& request) {
    auto key = makeKey(request.method, request.path);
    auto it = routes_.find(key);

    if (it != routes_.end()) {
        return it.value()(request);
    }

    if (knownPaths_.contains(request.path)) {
        return makeErrorResponse(405, "Method Not Allowed", "method not allowed");
    }

    return makeErrorResponse(404, "Not Found", "not found");
}

QString ApiRouter::makeKey(HttpMethod method, const QString& path) {
    return httpMethodToString(method) + " " + path;
}

}  // namespace api
}  // namespace wekey
