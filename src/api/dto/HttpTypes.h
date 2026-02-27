/**
 * @file HttpTypes.h
 * @brief HTTP 基础类型定义 (M4.1.2I)
 *
 * 定义 HTTP 请求/响应的基础数据结构，解耦 DTO 层与 QtHttpServer 实现
 */

#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QString>

#include "common/Error.h"
#include "common/Result.h"

namespace wekey {
namespace api {

/**
 * @brief HTTP 方法枚举
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};

/**
 * @brief HTTP 方法转字符串
 */
QString httpMethodToString(HttpMethod method);

/**
 * @brief 字符串转 HTTP 方法
 * @param str 方法字符串（大小写不敏感）
 * @return HTTP 方法，未知时返回 GET
 */
HttpMethod stringToHttpMethod(const QString& str);

/**
 * @brief HTTP 请求结构
 */
struct HttpRequest {
    HttpMethod method = HttpMethod::GET;
    QString path;
    QMap<QString, QString> headers;
    QMap<QString, QString> queryParams;
    QString body;

    /**
     * @brief 解析 JSON 请求体
     * @return Result<QJsonObject> 成功返回 JSON 对象，失败返回错误
     */
    Result<QJsonObject> jsonBody() const;

    /**
     * @brief 获取查询参数
     * @param key 参数名
     * @param defaultValue 默认值
     * @return 参数值，不存在时返回默认值
     */
    QString query(const QString& key, const QString& defaultValue = QString()) const;

    /**
     * @brief 获取请求头
     * @param key 请求头名称
     * @param defaultValue 默认值
     * @return 请求头值，不存在时返回默认值
     */
    QString header(const QString& key, const QString& defaultValue = QString()) const;
};

/**
 * @brief HTTP 响应结构
 */
struct HttpResponse {
    int statusCode = 200;
    QString statusText = "OK";
    QMap<QString, QString> headers;
    QString body;

    /**
     * @brief 设置 JSON 响应体
     * @param json JSON 对象
     */
    void setJson(const QJsonObject& json);

    /**
     * @brief 设置错误响应
     * @param error 错误对象
     *
     * 生成标准错误响应格式：
     * { "code": <error_code>, "message": "<error_message>", "data": null }
     */
    void setError(const Error& error);

    /**
     * @brief 设置成功响应（data 为 JSON 对象）
     * @param data 可选的数据对象，默认为 null
     *
     * 生成标准成功响应格式：
     * { "code": 0, "message": "success", "data": <data> }
     */
    void setSuccess(const QJsonObject& data = QJsonObject());

    /**
     * @brief 设置成功响应（data 为任意 JSON 值，支持对象、数组、字符串等）
     * @param data JSON 值（QJsonObject / QJsonArray / QString 等）
     *
     * 生成标准成功响应格式：
     * { "code": 0, "message": "success", "data": <data> }
     */
    void setSuccess(const QJsonValue& data);

};

}  // namespace api
}  // namespace wekey
