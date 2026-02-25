/**
 * @file PublicHandlers.h
 * @brief 公共接口处理器 (M4.4.3I)
 *
 * 处理 /health 和 /exit 等公共端点
 */

#pragma once

#include <QObject>

#include "api/dto/HttpTypes.h"

namespace wekey {
namespace api {

/**
 * @brief 公共接口处理器
 *
 * handleHealth 为静态方法（无状态），
 * handleExit 为成员方法（需要发信号）
 */
class PublicHandlers : public QObject {
    Q_OBJECT

public:
    explicit PublicHandlers(QObject* parent = nullptr);

    /**
     * @brief GET /health
     * @return {"code":0,"message":"success","data":{"status":"ok","version":"..."}}
     */
    static HttpResponse handleHealth(const HttpRequest& request);

    /**
     * @brief GET /exit - 触发应用退出
     * @return 成功响应，随后发出 exitRequested 信号
     */
    HttpResponse handleExit(const HttpRequest& request);

signals:
    void exitRequested();
};

}  // namespace api
}  // namespace wekey
