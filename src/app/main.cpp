/**
 * @file main.cpp
 * @brief 应用程序入口点
 */

#include <QMessageBox>
#include <QTimer>

#include <ElaApplication.h>
#include <ElaMessageBar.h>

#include "api/ApiRouter.h"
#include "api/HttpServer.h"
#include "api/handlers/PublicHandlers.h"
#include "app/Application.h"
#include "config/Config.h"
#include "gui/MainWindow.h"
#include "log/Logger.h"

using namespace wekey;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // 初始化 ElaWidgetTools 主题引擎
    eApp->init();

    if (!app.initialize()) {
        QMessageBox::critical(nullptr, "错误", "应用程序初始化失败");
        return 1;
    }

    if (!app.isPrimaryInstance()) {
        QMessageBox::warning(nullptr, "提示", "程序已在运行中");
        return 0;
    }

    LOG_INFO("wekey-skf 启动");

    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();

    // 启动 HTTP API 服务器
    auto* publicHandlers = new api::PublicHandlers(&app);
    auto* router = new api::ApiRouter;
    router->setupRoutes(publicHandlers);

    auto* httpServer = new api::HttpServer(&app);
    httpServer->setRouter(router);

    QString portStr = Config::instance().listenPort();
    portStr.remove(':');
    int port = portStr.toInt();
    if (port == 0) port = 9001;

    auto result = httpServer->start(port);
    MainWindow* mainWindowPtr = &mainWindow;

    if (result.isOk()) {
        LOG_INFO(QString("HTTP API 已启动: :%1").arg(port));
        ElaMessageBar::success(ElaMessageBarType::TopRight, "HTTP API 已启动",
            QString("监听端口: %1").arg(port), 3000, mainWindowPtr);

        // 定期检查 HTTP 服务器状态，停止监听时自动尝试重启
        auto* statusChecker = new QTimer(&app);
        QObject::connect(statusChecker, &QTimer::timeout, [httpServer, mainWindowPtr, port]() {
            static int retryCount = 0;
            static const int maxRetries = 3;

            if (!httpServer->isListening()) {
                if (retryCount < maxRetries) {
                    retryCount++;
                    LOG_ERROR(QString("HTTP API 已停止监听端口 %1，尝试自动恢复 (%2/%3)")
                        .arg(port).arg(retryCount).arg(maxRetries));
                    httpServer->stop();
                    auto restartResult = httpServer->start(port);
                    if (restartResult.isOk()) {
                        LOG_INFO(QString("HTTP API 自动恢复成功，端口: %1").arg(port));
                        ElaMessageBar::success(ElaMessageBarType::TopRight, "HTTP API 已恢复",
                            QString("端口 %1 重新监听中").arg(port), 3000, mainWindowPtr);
                        retryCount = 0;
                    } else {
                        LOG_ERROR(QString("HTTP API 自动恢复失败: %1").arg(restartResult.error().message()));
                    }
                } else {
                    LOG_ERROR(QString("HTTP API 自动恢复失败，已达最大重试次数 %1").arg(maxRetries));
                    ElaMessageBar::error(ElaMessageBarType::TopRight, "HTTP API 异常",
                        QString("端口 %1 多次恢复失败，API 功能已失效").arg(port),
                        10000, mainWindowPtr);
                }
            } else {
                // 服务器正常运行，重置重试计数
                retryCount = 0;
            }
        });
        // 每 10 秒检查一次（缩短检查间隔以更快发现问题）
        statusChecker->start(10000);
    } else {
        LOG_ERROR(QString("HTTP API 启动失败: %1").arg(result.error().message()));
        ElaMessageBar::error(ElaMessageBarType::TopRight, "HTTP API 启动失败",
            QString("端口 %1 可能已被占用，API 功能将不可用。\n%2")
                .arg(port)
                .arg(result.error().message()), 8000, mainWindowPtr);
    }

    QObject::connect(publicHandlers, &api::PublicHandlers::exitRequested, &app, &QApplication::quit);

    int exitCode = app.exec();

    httpServer->stop();
    app.shutdown();
    return exitCode;
}
