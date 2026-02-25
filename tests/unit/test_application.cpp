/**
 * @file test_application.cpp
 * @brief Application 类单元测试
 *
 * 测试用例 M1.6.3T
 *
 * 注意：由于 QApplication 的限制，每个进程只能有一个实例
 * 因此所有测试共用同一个 Application 实例
 */

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "app/Application.h"
#include "config/Config.h"
#include "log/Logger.h"

using namespace wekey;

// 全局 Application 实例，避免多次创建
static int g_argc = 1;
static char g_arg0[] = "test_application";
static char* g_argv[] = {g_arg0, nullptr};

class TestApplication : public QObject {
    Q_OBJECT

private:
    static QTemporaryDir* tempDir_;
    static QString originalHome_;
    static Application* app_;

private slots:
    void initTestCase() {
        // 保存原始 HOME 环境变量
        originalHome_ = qEnvironmentVariable("HOME");
        tempDir_ = new QTemporaryDir();
        // 使用临时目录作为 HOME
        qputenv("HOME", tempDir_->path().toUtf8());

        // 创建唯一的 Application 实例
        app_ = new Application(g_argc, g_argv);
    }

    void cleanupTestCase() {
        // 关闭并删除 Application
        if (app_) {
            app_->shutdown();
            delete app_;
            app_ = nullptr;
        }

        // 恢复原始 HOME 环境变量
        qputenv("HOME", originalHome_.toUtf8());

        delete tempDir_;
        tempDir_ = nullptr;
    }

    void cleanup() {
        // 每个测试后重置配置
        Config::instance().reset();
    }

    /**
     * @brief 测试应用名称和版本
     */
    void testAppNameAndVersion() {
        QVERIFY(app_ != nullptr);
        QCOMPARE(app_->applicationName(), QString("wekey-skf"));
        QVERIFY(!app_->applicationVersion().isEmpty());
    }

    /**
     * @brief 测试组织信息
     */
    void testOrganizationInfo() {
        QVERIFY(app_ != nullptr);
        QCOMPARE(app_->organizationName(), QString("TrustAsia"));
        QCOMPARE(app_->organizationDomain(), QString("trustasia.com"));
    }

    /**
     * @brief 测试初始化成功
     */
    void testInitialize() {
        QVERIFY(app_ != nullptr);
        bool result = app_->initialize();
        QVERIFY(result);
    }

    /**
     * @brief 测试单例检测（同一实例）
     */
    void testSingleInstanceSelf() {
        QVERIFY(app_ != nullptr);
        // 同一应用实例应该报告自己是主实例
        QVERIFY(app_->isPrimaryInstance());
    }

    /**
     * @brief 测试配置加载
     */
    void testConfigLoaded() {
        Config& config = Config::instance();
        // 初始化后配置应该已加载
        // 验证默认值存在
        QVERIFY(!config.listenPort().isEmpty());
    }

    /**
     * @brief 测试日志系统初始化
     */
    void testLoggerInitialized() {
        // 验证日志系统可用
        Logger& logger = Logger::instance();
        // 发送一条日志不应该崩溃
        logger.info("Test message from unit test", "TestApplication");
    }

    /**
     * @brief 测试内置模块自动注册
     */
    void testBuiltinModuleAutoRegistration() {
        // 在首次初始化时（配置为空），应该自动注册内置模块
        Config& config = Config::instance();

        // 检查是否注册了 gm3000 模块
        QJsonObject modPaths = config.modPaths();

        // 如果找到了内置库，应该自动注册
        if (modPaths.contains("gm3000")) {
            QString path = modPaths["gm3000"].toString();
            QVERIFY(!path.isEmpty());
            QVERIFY(path.contains("libgm3000") || path.contains("mtoken_gm3000"));

            // 检查是否设置为默认激活模块
            QString activeMod = config.activedModName();
            QCOMPARE(activeMod, QString("gm3000"));
        }
    }
};

// 静态成员初始化
QTemporaryDir* TestApplication::tempDir_ = nullptr;
QString TestApplication::originalHome_;
Application* TestApplication::app_ = nullptr;

// 手动定义 main 函数，避免宏问题
int main(int argc, char* argv[]) {
    TestApplication tc;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&tc, argc, argv);
}

#include "test_application.moc"
