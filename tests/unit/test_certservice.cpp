/**
 * @file test_certservice.cpp
 * @brief CertService 单元测试
 */

#include <QTest>

#include "core/crypto/CertService.h"
#include "plugin/PluginManager.h"

using namespace wekey;

class TestCertService : public QObject {
    Q_OBJECT

private slots:
    void cleanup() {
        auto& mgr = PluginManager::instance();
        for (const auto& name : mgr.listPlugins()) {
            mgr.unregisterPlugin(name);
        }
    }

    void testGenerateKeyPairNoActivePlugin() {
        auto result = CertService::instance().generateKeyPair("dev", "app", "container", "SM2");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testImportCertNoActivePlugin() {
        auto result = CertService::instance().importCert("dev", "app", "container", QByteArray(), true);
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testExportCertNoActivePlugin() {
        auto result = CertService::instance().exportCert("dev", "app", "container", true);
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testGetCertInfoNoActivePlugin() {
        auto result = CertService::instance().getCertInfo("dev", "app", "container", true);
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testSignNoActivePlugin() {
        auto result = CertService::instance().sign("dev", "app", "container", QByteArray("data"), "SM2");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testVerifyNoActivePlugin() {
        auto result = CertService::instance().verify("dev", "app", "container",
                                                      QByteArray("data"), QByteArray("sig"), "SM2");
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::NoActiveModule);
    }

    void testErrorContext() {
        auto result = CertService::instance().sign("dev", "app", "container", QByteArray("data"), "SM2");
        QVERIFY(result.isErr());
        QVERIFY(result.error().context().contains("CertService"));
    }
};

QTEST_MAIN(TestCertService)
#include "test_certservice.moc"
