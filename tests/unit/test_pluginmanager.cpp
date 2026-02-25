/**
 * @file test_pluginmanager.cpp
 * @brief PluginManager 单元测试
 */

#include <QSignalSpy>
#include <QTest>

#include "plugin/PluginManager.h"

using namespace wekey;

class TestPluginManager : public QObject {
    Q_OBJECT

private slots:
    void cleanup() {
        // 每个测试后清理所有注册的插件
        auto& mgr = PluginManager::instance();
        for (const auto& name : mgr.listPlugins()) {
            mgr.unregisterPlugin(name);
        }
    }

    //=== 单例测试 ===

    void testSingleton() {
        auto& a = PluginManager::instance();
        auto& b = PluginManager::instance();
        QCOMPARE(&a, &b);
    }

    //=== 注册/卸载测试 ===

    void testRegisterPlugin() {
        auto& mgr = PluginManager::instance();
        auto result = mgr.registerPlugin("test-mod", "/some/path/to/skf.dylib");
        QVERIFY(result.isOk());
    }

    void testRegisterPluginInvalidPath() {
        auto& mgr = PluginManager::instance();
        auto result = mgr.registerPlugin("bad-mod", "");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::InvalidParam);
    }

    void testRegisterPluginDuplicate() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("dup-mod", "/path/a.dylib");
        auto result = mgr.registerPlugin("dup-mod", "/path/b.dylib");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::AlreadyExists);
    }

    void testUnregisterPlugin() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("unreg-mod", "/path/lib.dylib");
        auto result = mgr.unregisterPlugin("unreg-mod");
        QVERIFY(result.isOk());
        QVERIFY(mgr.getPlugin("unreg-mod") == nullptr);
    }

    void testUnregisterPluginNotExists() {
        auto& mgr = PluginManager::instance();
        auto result = mgr.unregisterPlugin("nonexistent");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NotFound);
    }

    //=== 获取插件测试 ===

    void testGetPlugin() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("get-mod", "/path/lib.dylib");
        auto* plugin = mgr.getPlugin("get-mod");
        QVERIFY(plugin != nullptr);
    }

    void testGetPluginNotExists() {
        auto& mgr = PluginManager::instance();
        auto* plugin = mgr.getPlugin("nonexistent");
        QVERIFY(plugin == nullptr);
    }

    //=== 激活插件测试 ===

    void testActivePluginDefault() {
        auto& mgr = PluginManager::instance();
        QVERIFY(mgr.activePlugin() == nullptr);
        QVERIFY(mgr.activePluginName().isEmpty());
    }

    void testSetActivePlugin() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("active-mod", "/path/lib.dylib");
        auto result = mgr.setActivePlugin("active-mod");
        QVERIFY(result.isOk());
        QCOMPARE(mgr.activePluginName(), "active-mod");
        QVERIFY(mgr.activePlugin() != nullptr);
    }

    void testSetActivePluginNotExists() {
        auto& mgr = PluginManager::instance();
        auto result = mgr.setActivePlugin("nonexistent");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NotFound);
    }

    void testUnregisterActivePlugin() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("act-mod", "/path/lib.dylib");
        mgr.setActivePlugin("act-mod");
        mgr.unregisterPlugin("act-mod");
        QVERIFY(mgr.activePlugin() == nullptr);
        QVERIFY(mgr.activePluginName().isEmpty());
    }

    //=== 列出插件测试 ===

    void testListPluginsEmpty() {
        auto& mgr = PluginManager::instance();
        QVERIFY(mgr.listPlugins().isEmpty());
    }

    void testListPlugins() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("mod-a", "/path/a.dylib");
        mgr.registerPlugin("mod-b", "/path/b.dylib");
        auto list = mgr.listPlugins();
        QCOMPARE(list.size(), 2);
        QVERIFY(list.contains("mod-a"));
        QVERIFY(list.contains("mod-b"));
    }

    //=== 信号测试 ===

    void testPluginRegisteredSignal() {
        auto& mgr = PluginManager::instance();
        QSignalSpy spy(&mgr, &PluginManager::pluginRegistered);
        mgr.registerPlugin("sig-mod", "/path/lib.dylib");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), "sig-mod");
    }

    void testPluginUnregisteredSignal() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("unsig-mod", "/path/lib.dylib");
        QSignalSpy spy(&mgr, &PluginManager::pluginUnregistered);
        mgr.unregisterPlugin("unsig-mod");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), "unsig-mod");
    }

    void testActivePluginChangedSignal() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("chg-mod", "/path/lib.dylib");
        QSignalSpy spy(&mgr, &PluginManager::activePluginChanged);
        mgr.setActivePlugin("chg-mod");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), "chg-mod");
    }

    //=== 路径查询测试 ===

    void testGetPluginPath() {
        auto& mgr = PluginManager::instance();
        mgr.registerPlugin("path-mod", "/some/path/lib.dylib");
        QCOMPARE(mgr.getPluginPath("path-mod"), "/some/path/lib.dylib");
    }

    void testGetPluginPathNotExists() {
        auto& mgr = PluginManager::instance();
        QVERIFY(mgr.getPluginPath("nonexistent").isEmpty());
    }
};

QTEST_MAIN(TestPluginManager)
#include "test_pluginmanager.moc"
