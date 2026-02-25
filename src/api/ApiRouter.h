/**
 * @file ApiRouter.h
 * @brief API 路由器 (M4.3.2I)
 *
 * 管理 HTTP 路由表，将请求分发到对应的处理器
 */

#pragma once

#include <QMap>
#include <QString>
#include <functional>

#include "dto/HttpTypes.h"

namespace wekey {
namespace api {

class PublicHandlers;

/**
 * @brief 路由处理器类型
 */
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief API 路由器
 *
 * 维护 method+path -> handler 的映射表
 */
class ApiRouter {
public:
    ApiRouter() = default;

    /**
     * @brief 添加路由
     * @param method HTTP 方法
     * @param path 路径
     * @param handler 处理函数
     */
    /**
     * @brief 注册所有 API 路由
     * @param publicHandlers 公共处理器实例（用于 /exit 信号）
     */
    void setupRoutes(PublicHandlers* publicHandlers);

    void addRoute(HttpMethod method, const QString& path, RouteHandler handler);

    /**
     * @brief 处理请求
     * @param request HTTP 请求
     * @return HTTP 响应
     */
    HttpResponse handleRequest(const HttpRequest& request);

private:
    /**
     * @brief 路由键: "METHOD /path"
     */
    static QString makeKey(HttpMethod method, const QString& path);

    // method+path -> handler
    QMap<QString, RouteHandler> routes_;
    // path -> 是否存在（用于区分 404 和 405）
    QMap<QString, bool> knownPaths_;
};

}  // namespace api
}  // namespace wekey
