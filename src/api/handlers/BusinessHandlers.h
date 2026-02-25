/**
 * @file BusinessHandlers.h
 * @brief 业务接口处理器 (M4.5)
 *
 * 处理 /api/v1/ 业务端点，委托给核心服务层
 */

#pragma once

#include "api/dto/HttpTypes.h"

namespace wekey {
namespace api {

/**
 * @brief 业务接口处理器（全部静态方法）
 */
class BusinessHandlers {
public:
    static HttpResponse handleEnumDev(const HttpRequest& request);
    static HttpResponse handleLogin(const HttpRequest& request);
    static HttpResponse handleLogout(const HttpRequest& request);
    static HttpResponse handleGenCsr(const HttpRequest& request);
    static HttpResponse handleImportCert(const HttpRequest& request);
    static HttpResponse handleExportCert(const HttpRequest& request);
    static HttpResponse handleSign(const HttpRequest& request);
    static HttpResponse handleRandom(const HttpRequest& request);
};

}  // namespace api
}  // namespace wekey
