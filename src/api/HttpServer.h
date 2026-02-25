/**
 * @file HttpServer.h
 * @brief HTTP 服务器封装 (M4.2.2I)
 *
 * 基于 Qt6HttpServer 的轻量封装，提供启动/停止和信号通知
 */

#pragma once

#include <QHttpServer>
#include <QObject>

#include "ApiRouter.h"
#include "common/Result.h"

namespace wekey {
namespace api {

/**
 * @brief HTTP 服务器
 *
 * 封装 QHttpServer，提供简单的启动/停止接口和状态管理
 */
class HttpServer : public QObject {
    Q_OBJECT

public:
    explicit HttpServer(QObject* parent = nullptr);
    ~HttpServer() override;

    /**
     * @brief 启动服务器
     * @param port 监听端口
     * @return Result<void> 成功或错误
     */
    Result<void> start(int port);

    /**
     * @brief 停止服务器
     */
    void stop();

    /**
     * @brief 是否正在运行
     */
    [[nodiscard]] bool isRunning() const;

    /**
     * @brief 获取当前监听端口
     * @return 端口号，未运行时返回 0
     */
    [[nodiscard]] int port() const;

    /**
     * @brief 设置路由器
     *
     * 将所有未匹配的 QHttpServer 请求转发给 ApiRouter
     * 必须在 start() 之前调用
     */
    void setRouter(ApiRouter* router);

    /**
     * @brief 获取底层 QHttpServer 指针
     */
    [[nodiscard]] QHttpServer* server() const;

    /**
     * @brief 验证服务器是否真正在监听端口
     * @return true 如果服务器正在监听
     */
    [[nodiscard]] bool isListening() const;

signals:
    /**
     * @brief 服务器启动信号
     * @param port 监听端口
     */
    void started(int port);

    /**
     * @brief 服务器停止信号
     */
    void stopped();

private:
    QHttpServer* server_ = nullptr;
    QTcpServer* tcpServer_ = nullptr;
    ApiRouter* router_ = nullptr;
    bool running_ = false;
    int port_ = 0;
};

}  // namespace api
}  // namespace wekey
