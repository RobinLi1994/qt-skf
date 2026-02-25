/**
 * @file test_containerservice.cpp
 * @brief ContainerService 单元测试
 */

#include <QTest>

#include "core/container/ContainerService.h"
#include "plugin/PluginManager.h"

using namespace wekey;

class TestContainerService : public QObject {
    Q_OBJECT

private slots:
    void cleanup() {
        auto& mgr = PluginManager::instance();
        for (const auto& name : mgr.listPlugins()) {
            mgr.unregisterPlugin(name);
        }
    }

    void testEnumContainersNoActivePlugin() {
        auto result = ContainerService::instance().enumContainers("dev", "app");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testCreateContainerNoActivePlugin() {
        auto result = ContainerService::instance().createContainer("dev", "app", "container");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testDeleteContainerNoActivePlugin() {
        auto result = ContainerService::instance().deleteContainer("dev", "app", "container");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testErrorContext() {
        auto result = ContainerService::instance().enumContainers("dev", "app");
        QVERIFY(result.isErr());
        QVERIFY(result.error().context().contains("ContainerService"));
    }
};

QTEST_MAIN(TestContainerService)
#include "test_containerservice.moc"
