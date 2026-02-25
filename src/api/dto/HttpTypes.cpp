/**
 * @file HttpTypes.cpp
 * @brief HTTP 基础类型实现 (M4.1.2I)
 */

#include "HttpTypes.h"

#include <QJsonDocument>

namespace wekey {
namespace api {

// ==============================================================================
// HttpMethod 转换函数
// ==============================================================================

QString httpMethodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET:
            return "GET";
        case HttpMethod::POST:
            return "POST";
        case HttpMethod::PUT:
            return "PUT";
        case HttpMethod::DELETE:
            return "DELETE";
        case HttpMethod::PATCH:
            return "PATCH";
        case HttpMethod::HEAD:
            return "HEAD";
        case HttpMethod::OPTIONS:
            return "OPTIONS";
    }
    return "GET";
}

HttpMethod stringToHttpMethod(const QString& str) {
    QString upper = str.toUpper();
    if (upper == "GET") return HttpMethod::GET;
    if (upper == "POST") return HttpMethod::POST;
    if (upper == "PUT") return HttpMethod::PUT;
    if (upper == "DELETE") return HttpMethod::DELETE;
    if (upper == "PATCH") return HttpMethod::PATCH;
    if (upper == "HEAD") return HttpMethod::HEAD;
    if (upper == "OPTIONS") return HttpMethod::OPTIONS;
    return HttpMethod::GET;  // 默认返回 GET
}

// ==============================================================================
// HttpRequest 实现
// ==============================================================================

Result<QJsonObject> HttpRequest::jsonBody() const {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return Result<QJsonObject>::err(
            Error(Error::InvalidParam,
                  QString("JSON 解析失败: %1").arg(parseError.errorString()),
                  "HttpRequest::jsonBody"));
    }

    if (!doc.isObject()) {
        return Result<QJsonObject>::err(
            Error(Error::InvalidParam, "请求体不是有效的 JSON 对象", "HttpRequest::jsonBody"));
    }

    return Result<QJsonObject>::ok(doc.object());
}

QString HttpRequest::query(const QString& key, const QString& defaultValue) const {
    return queryParams.value(key, defaultValue);
}

QString HttpRequest::header(const QString& key, const QString& defaultValue) const {
    return headers.value(key, defaultValue);
}

// ==============================================================================
// HttpResponse 实现
// ==============================================================================

void HttpResponse::setJson(const QJsonObject& json) {
    statusCode = 200;
    statusText = "OK";
    headers["Content-Type"] = "application/json; charset=utf-8";

    QJsonDocument doc(json);
    body = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

void HttpResponse::setError(const Error& error) {
    // 根据错误码映射 HTTP 状态码
    switch (error.code()) {
        case Error::InvalidParam:
            statusCode = 400;
            statusText = "Bad Request";
            break;
        case Error::NotAuthorized:
        case Error::NotLoggedIn:
            statusCode = 401;
            statusText = "Unauthorized";
            break;
        case Error::NotFound:
            statusCode = 404;
            statusText = "Not Found";
            break;
        case Error::AlreadyExists:
            statusCode = 409;
            statusText = "Conflict";
            break;
        default:
            statusCode = 500;
            statusText = "Internal Server Error";
            break;
    }

    headers["Content-Type"] = "application/json; charset=utf-8";

    QJsonObject response;
    response["code"] = static_cast<int>(error.code());
    response["message"] = error.friendlyMessage();
    response["data"] = QJsonValue::Null;

    QJsonDocument doc(response);
    body = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

void HttpResponse::setSuccess(const QJsonObject& data) {
    statusCode = 200;
    statusText = "OK";
    headers["Content-Type"] = "application/json; charset=utf-8";

    QJsonObject response;
    response["code"] = 0;
    response["message"] = "success";

    if (data.isEmpty()) {
        response["data"] = QJsonValue::Null;
    } else {
        response["data"] = data;
    }

    QJsonDocument doc(response);
    body = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

}  // namespace api
}  // namespace wekey
