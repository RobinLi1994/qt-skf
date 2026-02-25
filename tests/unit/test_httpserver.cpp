/**
 * @file test_httpserver.cpp
 * @brief HttpServer 单元测试 (M4.2.1T)
 */

#include <QSignalSpy>
#include <QTest>
#include <QTcpServer>

#include "api/HttpServer.h"

using namespace wekey;
using namespace wekey::api;

class TestHttpServer : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void init() {
        server_ = new HttpServer();
    }

    void cleanup() {
        if (server_->isRunning()) {
            server_->stop();
        }
        delete server_;
        server_ = nullptr;
    }

    /**
     * @brief 测试启动和停止服务器
     */
    void testStartStop() {
        auto result = server_->start(19001);
        QVERIFY2(result.isOk(), qPrintable(result.isErr() ? result.error().message() : ""));
        QVERIFY(server_->isRunning());

        server_->stop();
        QVERIFY(!server_->isRunning());
    }

    /**
     * @brief 测试端口被占用返回错误
     */
    void testPortInUse() {
        // 先占用端口
        QTcpServer blocker;
        QVERIFY(blocker.listen(QHostAddress::Any, 19002));

        auto result = server_->start(19002);
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::PortInUse);

        blocker.close();
    }

    /**
     * @brief 测试 isRunning() 状态正确
     */
    void testIsRunning() {
        QVERIFY(!server_->isRunning());

        auto result = server_->start(19003);
        QVERIFY(result.isOk());
        QVERIFY(server_->isRunning());

        server_->stop();
        QVERIFY(!server_->isRunning());
    }

    /**
     * @brief 测试 port() 返回正确端口
     */
    void testPort() {
        QCOMPARE(server_->port(), 0);

        auto result = server_->start(19004);
        QVERIFY(result.isOk());
        QCOMPARE(server_->port(), 19004);

        server_->stop();
        QCOMPARE(server_->port(), 0);
    }

    /**
     * @brief 测试启动时发出信号
     */
    void testStartedSignal() {
        QSignalSpy spy(server_, &HttpServer::started);
        QVERIFY(spy.isValid());

        auto result = server_->start(19005);
        QVERIFY(result.isOk());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 19005);
    }

    /**
     * @brief 测试停止时发出信号
     */
    void testStoppedSignal() {
        auto result = server_->start(19006);
        QVERIFY(result.isOk());

        QSignalSpy spy(server_, &HttpServer::stopped);
        QVERIFY(spy.isValid());

        server_->stop();
        QCOMPARE(spy.count(), 1);
    }

private:
    HttpServer* server_ = nullptr;
};

QTEST_MAIN(TestHttpServer)
#include "test_httpserver.moc"
