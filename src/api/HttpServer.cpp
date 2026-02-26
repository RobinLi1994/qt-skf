/**
 * @file HttpServer.cpp
 * @brief HTTP 服务器实现 (M4.2.3I)
 */

#include "HttpServer.h"

#include <QHttpHeaders>
#include <QHttpServerRequest>
#include <QHttpServerResponder>
#include <QTcpServer>
#include <QUrlQuery>
#include <QThreadPool>

#include "dto/HttpTypes.h"
#include "log/Logger.h"

namespace wekey {
namespace api {

HttpServer::HttpServer(QObject* parent)
    : QObject(parent), server_(new QHttpServer(this)) {}

HttpServer::~HttpServer() {
    if (running_) {
        stop();
    }
}

Result<void> HttpServer::start(int port) {
    if (running_) {
        return Result<void>::err(
            Error(Error::Fail, "server is already running", "HttpServer::start"));
    }

    tcpServer_ = new QTcpServer(this);
    tcpServer_->setMaxPendingConnections(512);

    // 确保端口独占绑定，防止其他程序复用端口
    // 注意：在 macOS 上，QTcpServer 默认行为已经是独占的
    // 但我们显式记录这一点以便调试
    if (!tcpServer_->listen(QHostAddress::Any, static_cast<quint16>(port))) {
        QString errorMsg = tcpServer_->errorString();
        delete tcpServer_;
        tcpServer_ = nullptr;
        return Result<void>::err(
            Error(Error::PortInUse,
                  QString("failed to listen on port %1: %2").arg(port).arg(errorMsg),
                  "HttpServer::start"));
    }

    server_->bind(tcpServer_);
    running_ = true;
    port_ = port;

    emit started(port);
    return Result<void>::ok();
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    if (tcpServer_) {
        tcpServer_->close();
        delete tcpServer_;
        tcpServer_ = nullptr;
    }

    running_ = false;
    port_ = 0;

    emit stopped();
}

bool HttpServer::isRunning() const {
    return running_;
}

int HttpServer::port() const {
    return port_;
}

void HttpServer::setRouter(ApiRouter* router) {
    router_ = router;

    server_->setMissingHandler(this, [this](const QHttpServerRequest& req, QHttpServerResponder& responder) {
        // 构建 CORS 响应头
        QHttpHeaders corsHeaders;
        corsHeaders.append(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin, "*");
        corsHeaders.append(QHttpHeaders::WellKnownHeader::AccessControlAllowMethods,
                           "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        corsHeaders.append(QHttpHeaders::WellKnownHeader::AccessControlAllowHeaders,
                           "Content-Type, Authorization, X-Requested-With");
        corsHeaders.append(QHttpHeaders::WellKnownHeader::AccessControlMaxAge, "86400");

        // OPTIONS 预检请求直接返回 204（无需异步）
        if (req.method() == QHttpServerRequest::Method::Options) {
            responder.write(corsHeaders, QHttpServerResponder::StatusCode::NoContent);
            return;
        }

        // 在主线程中提取请求数据（QHttpServerRequest 不可跨线程使用）
        HttpRequest httpReq;
        httpReq.path = req.url().path();
        httpReq.body = QString::fromUtf8(req.body());

        // Convert method
        switch (req.method()) {
            case QHttpServerRequest::Method::Get: httpReq.method = HttpMethod::GET; break;
            case QHttpServerRequest::Method::Post: httpReq.method = HttpMethod::POST; break;
            case QHttpServerRequest::Method::Put: httpReq.method = HttpMethod::PUT; break;
            case QHttpServerRequest::Method::Delete: httpReq.method = HttpMethod::DELETE; break;
            case QHttpServerRequest::Method::Patch: httpReq.method = HttpMethod::PATCH; break;
            case QHttpServerRequest::Method::Head: httpReq.method = HttpMethod::HEAD; break;
            case QHttpServerRequest::Method::Options: httpReq.method = HttpMethod::OPTIONS; break;
            default: httpReq.method = HttpMethod::GET; break;
        }

        // Convert query params
        QUrlQuery urlQuery(req.url());
        for (const auto& item : urlQuery.queryItems()) {
            httpReq.queryParams[item.first] = item.second;
        }

        // Convert headers
        auto headers = req.headers();
        for (qsizetype i = 0; i < headers.size(); ++i) {
            httpReq.headers[QString(headers.nameAt(i))] =
                QString::fromUtf8(headers.valueAt(i));
        }

        // 将阻塞的业务处理移到线程池中异步执行，避免阻塞主线程事件循环
        // QHttpServerResponder 底层 socket 绑定主线程，write() 必须在主线程调用
        // 因此：线程池执行业务逻辑 → QMetaObject::invokeMethod 回主线程写响应
        auto responderPtr = std::make_shared<QHttpServerResponder>(std::move(responder));
        auto* router = router_;
        auto* self = this;

        QThreadPool::globalInstance()->start([httpReq = std::move(httpReq), corsHeaders = std::move(corsHeaders),
                           responderPtr, router, self]() mutable {
            LOG_DEBUG(QString("[HttpServer] 异步处理请求: %1").arg(httpReq.path));

            // 在线程池中执行路由分发和业务逻辑（可能涉及耗时的硬件操作）
            auto httpResp = router->handleRequest(httpReq);

            LOG_DEBUG(QString("[HttpServer] 业务处理完成: %1 -> %2").arg(httpReq.path).arg(httpResp.statusCode));

            // 回到主线程写响应（QHttpServerResponder::write 必须在主线程调用）
            QMetaObject::invokeMethod(self, [responderPtr, httpResp = std::move(httpResp),
                                              corsHeaders = std::move(corsHeaders)]() mutable {
                QByteArray body = httpResp.body.toUtf8();
                corsHeaders.append(QHttpHeaders::WellKnownHeader::ContentType,
                                   "application/json; charset=utf-8");
                auto status = static_cast<QHttpServerResponder::StatusCode>(httpResp.statusCode);
                responderPtr->write(body, corsHeaders, status);
            });
        });
    });
}

QHttpServer* HttpServer::server() const {
    return server_;
}

bool HttpServer::isListening() const {
    if (!running_ || !tcpServer_) {
        return false;
    }
    return tcpServer_->isListening();
}

}  // namespace api
}  // namespace wekey
