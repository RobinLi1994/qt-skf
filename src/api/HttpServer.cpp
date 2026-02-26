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

    // 增加最大待处理连接数（默认 30），防止连续请求时连接队列满
    tcpServer_->setMaxPendingConnections(128);

    // 监听 TCP 接受连接错误，记录日志以便排查
    connect(tcpServer_, &QTcpServer::acceptError, this, [this](QAbstractSocket::SocketError err) {
        LOG_ERROR(QString("TCP accept error: %1 (%2)").arg(static_cast<int>(err)).arg(tcpServer_->errorString()));
    });

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

    LOG_INFO(QString("HTTP 服务器启动成功, 端口: %1, 最大连接队列: 128").arg(port));
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

        // OPTIONS 预检请求直接返回 204
        if (req.method() == QHttpServerRequest::Method::Options) {
            responder.write(corsHeaders, QHttpServerResponder::StatusCode::NoContent);
            return;
        }

        try {
            // Convert QHttpServerRequest to our HttpRequest
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

            LOG_INFO(QString("HTTP %1 %2").arg(httpMethodToString(httpReq.method), httpReq.path));

            // Route and get response
            auto httpResp = router_->handleRequest(httpReq);

            // Write response with CORS headers
            QByteArray body = httpResp.body.toUtf8();
            corsHeaders.append(QHttpHeaders::WellKnownHeader::ContentType,
                               "application/json; charset=utf-8");
            auto status = static_cast<QHttpServerResponder::StatusCode>(httpResp.statusCode);
            responder.write(body, corsHeaders, status);

        } catch (const std::exception& e) {
            // 捕获 handler 中的异常，防止导致 QTcpServer 停止监听
            LOG_ERROR(QString("HTTP handler 异常: %1").arg(e.what()));
            QByteArray errBody = R"({"code":500,"message":"internal server error","data":null})";
            corsHeaders.append(QHttpHeaders::WellKnownHeader::ContentType,
                               "application/json; charset=utf-8");
            responder.write(errBody, corsHeaders, QHttpServerResponder::StatusCode::InternalServerError);
        } catch (...) {
            LOG_ERROR("HTTP handler 未知异常");
            QByteArray errBody = R"({"code":500,"message":"internal server error","data":null})";
            corsHeaders.append(QHttpHeaders::WellKnownHeader::ContentType,
                               "application/json; charset=utf-8");
            responder.write(errBody, corsHeaders, QHttpServerResponder::StatusCode::InternalServerError);
        }
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
