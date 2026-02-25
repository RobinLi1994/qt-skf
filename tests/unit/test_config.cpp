/**
 * @file test_config.cpp
 * @brief Config 类单元测试
 *
 * 测试用例 M1.3.3T
 */

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "config/Config.h"
#include "config/Defaults.h"

using namespace wekey;

class TestConfig : public QObject {
    Q_OBJECT

private:
    QTemporaryDir tempDir_;
    QString originalHome_;

private slots:
    void initTestCase() {
        // 保存原始 HOME 环境变量
        originalHome_ = qEnvironmentVariable("HOME");
        // 使用临时目录作为 HOME，避免污染真实配置
        qputenv("HOME", tempDir_.path().toUtf8());
    }

    void cleanupTestCase() {
        // 恢复原始 HOME 环境变量
        qputenv("HOME", originalHome_.toUtf8());
    }

    void cleanup() {
        // 每个测试后重置配置
        Config::instance().reset();
        // 删除临时配置文件
        QString configPath = tempDir_.path() + "/" + defaults::CONFIG_FILENAME;
        QFile::remove(configPath);
    }

    /**
     * @brief 测试单例模式
     */
    void testSingleton() {
        Config& config1 = Config::instance();
        Config& config2 = Config::instance();
        QCOMPARE(&config1, &config2);
    }

    /**
     * @brief 测试默认值
     */
    void testDefaultValues() {
        Config& config = Config::instance();

        QCOMPARE(config.listenPort(), QString(defaults::LISTEN_PORT));
        QCOMPARE(config.logLevel(), QString(defaults::LOG_LEVEL));
        QCOMPARE(config.errorMode(), QString(defaults::ERROR_MODE_SIMPLE));
        QCOMPARE(config.systrayDisabled(), false);
        QVERIFY(config.activedModName().isEmpty());
    }

    /**
     * @brief 测试默认应用配置
     */
    void testDefaultAppConfig() {
        Config& config = Config::instance();

        QCOMPARE(config.defaultAppName(), QString(defaults::APP_NAME));
        QCOMPARE(config.defaultContainerName(), QString(defaults::CONTAINER_NAME));
        QCOMPARE(config.defaultCommonName(), QString(defaults::COMMON_NAME));
        QCOMPARE(config.defaultOrganization(), QString(defaults::ORGANIZATION));
        QCOMPARE(config.defaultUnit(), QString(defaults::UNIT));
        QCOMPARE(config.defaultRole(), QString(defaults::ROLE_USER));
    }

    /**
     * @brief 测试基本 Setter/Getter
     */
    void testSettersGetters() {
        Config& config = Config::instance();

        config.setListenPort(":8080");
        QCOMPARE(config.listenPort(), QString(":8080"));

        config.setLogLevel("debug");
        QCOMPARE(config.logLevel(), QString("debug"));

        config.setErrorMode("detailed");
        QCOMPARE(config.errorMode(), QString("detailed"));

        config.setSystrayDisabled(true);
        QCOMPARE(config.systrayDisabled(), true);

        config.setActivedModName("skf");
        QCOMPARE(config.activedModName(), QString("skf"));
    }

    /**
     * @brief 测试模块路径管理
     */
    void testModPaths() {
        Config& config = Config::instance();

        // 初始应该为空
        QVERIFY(config.modPaths().isEmpty());

        // 添加模块
        config.setModPath("skf", "/path/to/skf.dll");
        config.setModPath("p11", "/path/to/p11.dll");

        auto paths = config.modPaths();
        QCOMPARE(paths.size(), 2);
        QCOMPARE(paths["skf"].toString(), QString("/path/to/skf.dll"));
        QCOMPARE(paths["p11"].toString(), QString("/path/to/p11.dll"));

        // 删除模块
        config.removeModPath("p11");
        paths = config.modPaths();
        QCOMPARE(paths.size(), 1);
        QVERIFY(!paths.contains("p11"));
        QVERIFY(paths.contains("skf"));
    }

    /**
     * @brief 测试默认值设置
     */
    void testSetDefault() {
        Config& config = Config::instance();

        config.setDefault("appName", "CustomApp");
        QCOMPARE(config.defaultAppName(), QString("CustomApp"));

        config.setDefault("containerName", "CustomContainer");
        QCOMPARE(config.defaultContainerName(), QString("CustomContainer"));

        config.setDefault("role", "admin");
        QCOMPARE(config.defaultRole(), QString("admin"));
    }

    /**
     * @brief 测试加载不存在的配置文件
     */
    void testLoadNonExistent() {
        Config& config = Config::instance();

        // 加载不存在的文件应该返回 true（使用默认值）
        bool result = config.load();
        QVERIFY(result);

        // 应该使用默认值
        QCOMPARE(config.listenPort(), QString(defaults::LISTEN_PORT));
    }

    /**
     * @brief 测试保存和加载
     */
    void testSaveLoad() {
        Config& config = Config::instance();

        // 修改配置
        config.setListenPort(":9999");
        config.setLogLevel("error");
        config.setModPath("test", "/path/to/test.dll");
        config.setActivedModName("test");
        config.setDefault("appName", "TestApp");

        // 保存
        QVERIFY(config.save());

        // 重置配置
        config.reset();
        QCOMPARE(config.listenPort(), QString(defaults::LISTEN_PORT));

        // 重新加载
        QVERIFY(config.load());

        // 验证加载的值
        QCOMPARE(config.listenPort(), QString(":9999"));
        QCOMPARE(config.logLevel(), QString("error"));
        QCOMPARE(config.activedModName(), QString("test"));
        QCOMPARE(config.defaultAppName(), QString("TestApp"));

        auto paths = config.modPaths();
        QCOMPARE(paths["test"].toString(), QString("/path/to/test.dll"));
    }

    /**
     * @brief 测试 reset() 恢复默认值
     */
    void testReset() {
        Config& config = Config::instance();

        // 修改配置
        config.setListenPort(":8080");
        config.setLogLevel("debug");
        config.setModPath("skf", "/path/to/skf.dll");

        // 重置
        config.reset();

        // 验证恢复默认值
        QCOMPARE(config.listenPort(), QString(defaults::LISTEN_PORT));
        QCOMPARE(config.logLevel(), QString(defaults::LOG_LEVEL));
        QVERIFY(config.modPaths().isEmpty());
    }

    /**
     * @brief 测试 configChanged 信号
     */
    void testConfigChangedSignal() {
        Config& config = Config::instance();
        QSignalSpy spy(&config, &Config::configChanged);

        // 保存应该触发信号
        config.save();
        QCOMPARE(spy.count(), 1);

        // 重置应该触发信号
        config.reset();
        QCOMPARE(spy.count(), 2);
    }

    /**
     * @brief 测试日志路径
     */
    void testLogPath() {
        Config& config = Config::instance();

        // 默认应该是临时目录
        QVERIFY(!config.logPath().isEmpty());

        // 设置自定义路径
        config.setLogPath("/custom/log/path");
        QCOMPARE(config.logPath(), QString("/custom/log/path"));
    }

    /**
     * @brief 测试配置版本
     */
    void testVersion() {
        Config& config = Config::instance();

        // 验证版本号
        QCOMPARE(config.version(), QString(defaults::CONFIG_VERSION));
    }
};

QTEST_MAIN(TestConfig)
#include "test_config.moc"
