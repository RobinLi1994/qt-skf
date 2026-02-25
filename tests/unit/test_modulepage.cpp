/**
 * @file test_modulepage.cpp
 * @brief ModulePage 单元测试
 */

#include <QApplication>
#include <QPushButton>
#include <QTableWidget>
#include <QTest>
#include <memory>

#include "gui/pages/ModulePage.h"
#include "plugin/PluginManager.h"
#include "plugin/interface/IDriverPlugin.h"

using namespace wekey;

/**
 * @brief 最小 Mock 插件，仅用于 ModulePage 测试
 *
 * ModulePage 只关心 PluginManager 的注册/激活，不调用任何 IDriverPlugin 方法，
 * 所以这里只需要提供空实现。
 */
class StubPlugin : public IDriverPlugin {
public:
    Result<QList<DeviceInfo>> enumDevices(bool /*login*/) override { return Result<QList<DeviceInfo>>::ok({}); }
    Result<void> changeDeviceAuth(const QString& /*d*/, const QString& /*o*/, const QString& /*n*/) override { return Result<void>::ok(); }
    Result<void> setDeviceLabel(const QString& /*d*/, const QString& /*l*/) override { return Result<void>::ok(); }
    Result<int> waitForDeviceEvent() override { return Result<int>::ok(0); }
    Result<QList<AppInfo>> enumApps(const QString& /*d*/) override { return Result<QList<AppInfo>>::ok({}); }
    Result<void> createApp(const QString& /*d*/, const QString& /*a*/, const QVariantMap& /*args*/) override { return Result<void>::ok(); }
    Result<void> deleteApp(const QString& /*d*/, const QString& /*a*/) override { return Result<void>::ok(); }
    Result<void> openApp(const QString& /*d*/, const QString& /*a*/, const QString& /*r*/, const QString& /*p*/) override { return Result<void>::ok(); }
    Result<void> closeApp(const QString& /*d*/, const QString& /*a*/) override { return Result<void>::ok(); }
    Result<void> changePin(const QString& /*d*/, const QString& /*a*/, const QString& /*r*/, const QString& /*o*/, const QString& /*n*/) override { return Result<void>::ok(); }
    Result<void> unlockPin(const QString& /*d*/, const QString& /*a*/, const QString& /*ap*/, const QString& /*np*/, const QVariantMap& /*args*/) override { return Result<void>::ok(); }
    Result<int> getRetryCount(const QString& /*d*/, const QString& /*a*/, const QString& /*r*/) override { return Result<int>::ok(10); }
    Result<QList<ContainerInfo>> enumContainers(const QString& /*d*/, const QString& /*a*/) override { return Result<QList<ContainerInfo>>::ok({}); }
    Result<void> createContainer(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/) override { return Result<void>::ok(); }
    Result<void> deleteContainer(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/) override { return Result<void>::ok(); }
    Result<QByteArray> generateKeyPair(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/, const QString& /*t*/) override { return Result<QByteArray>::ok({}); }
    Result<void> importCert(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/, const QByteArray& /*data*/, bool /*s*/) override { return Result<void>::ok(); }
    Result<QByteArray> exportCert(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/, bool /*s*/) override { return Result<QByteArray>::ok({}); }
    Result<CertInfo> getCertInfo(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/, bool /*s*/) override { return Result<CertInfo>::ok({}); }
    Result<QByteArray> sign(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/, const QByteArray& /*data*/, const QString& /*algo*/) override { return Result<QByteArray>::ok({}); }
    Result<bool> verify(const QString& /*d*/, const QString& /*a*/, const QString& /*c*/, const QByteArray& /*data*/, const QByteArray& /*sig*/, const QString& /*algo*/) override { return Result<bool>::ok(true); }
    Result<QStringList> enumFiles(const QString& /*d*/, const QString& /*a*/) override { return Result<QStringList>::ok({}); }
    Result<QByteArray> readFile(const QString& /*d*/, const QString& /*a*/, const QString& /*f*/) override { return Result<QByteArray>::ok({}); }
    Result<void> writeFile(const QString& /*d*/, const QString& /*a*/, const QString& /*f*/, const QByteArray& /*data*/) override { return Result<void>::ok(); }
    Result<void> deleteFile(const QString& /*d*/, const QString& /*a*/, const QString& /*f*/) override { return Result<void>::ok(); }
    Result<QByteArray> generateRandom(const QString& /*d*/, int count) override { return Result<QByteArray>::ok(QByteArray(count, '\0')); }
};

class TestModulePage : public QObject {
    Q_OBJECT

private:
    void cleanupPlugins() {
        auto& pm = PluginManager::instance();
        for (const auto& name : pm.listPlugins()) {
            pm.unregisterPlugin(name);
        }
    }

private slots:
    void init() {
        cleanupPlugins();
    }

    void cleanup() {
        cleanupPlugins();
    }

    // --- 基础 UI 测试 ---

    void testTableExists() {
        ModulePage page;
        QVERIFY(page.table() != nullptr);
    }

    void testAddButtonExists() {
        ModulePage page;
        QVERIFY(page.addButton() != nullptr);
    }

    void testTableColumnCount() {
        ModulePage page;
        QCOMPARE(page.table()->columnCount(), 4);
    }

    void testTableHeaders() {
        ModulePage page;
        struct TestCase {
            int col;
            QString expected;
        };
        TestCase cases[] = {
            {0, "名称"},
            {1, "路径"},
            {2, "状态"},
            {3, "操作"},
        };
        for (const auto& tc : cases) {
            QCOMPARE(page.table()->horizontalHeaderItem(tc.col)->text(), tc.expected);
        }
    }

    void testTableInitiallyEmpty() {
        ModulePage page;
        QCOMPARE(page.table()->rowCount(), 0);
    }

    void testAddButtonText() {
        ModulePage page;
        QCOMPARE(page.addButton()->text(), "+ 添加模块");
    }

    // --- 功能测试 ---

    void testRefreshTable() {
        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("pluginA", std::make_shared<StubPlugin>());
        pm.registerPluginInstance("pluginB", std::make_shared<StubPlugin>());

        ModulePage page;
        page.refreshTable();

        QCOMPARE(page.table()->rowCount(), 2);

        // 验证名称列包含两个插件
        QStringList names;
        for (int r = 0; r < page.table()->rowCount(); ++r) {
            names << page.table()->item(r, 0)->text();
        }
        QVERIFY(names.contains("pluginA"));
        QVERIFY(names.contains("pluginB"));
    }

    void testRefreshTableShowsPath() {
        auto& pm = PluginManager::instance();
        pm.registerPlugin("testmod", "/usr/lib/libtest.so");

        ModulePage page;
        page.refreshTable();

        QCOMPARE(page.table()->rowCount(), 1);
        QCOMPARE(page.table()->item(0, 1)->text(), "/usr/lib/libtest.so");
    }

    void testActiveStatus() {
        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mod1", std::make_shared<StubPlugin>());
        pm.registerPluginInstance("mod2", std::make_shared<StubPlugin>());
        pm.setActivePlugin("mod1");

        ModulePage page;
        page.refreshTable();

        // 找到 mod1 所在行，验证状态列
        for (int r = 0; r < page.table()->rowCount(); ++r) {
            if (page.table()->item(r, 0)->text() == "mod1") {
                QCOMPARE(page.table()->item(r, 2)->text(), "已激活");
            } else {
                QCOMPARE(page.table()->item(r, 2)->text(), "未激活");
            }
        }
    }

    void testDeleteButtonInOperationColumn() {
        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mod1", std::make_shared<StubPlugin>());

        ModulePage page;
        page.refreshTable();

        QCOMPARE(page.table()->rowCount(), 1);

        // 操作列（第3列）应包含一个 widget，其中含有"删除"按钮
        auto* cellWidget = page.table()->cellWidget(0, 3);
        QVERIFY(cellWidget != nullptr);
        auto* deleteBtn = cellWidget->findChild<QPushButton*>("deleteButton");
        QVERIFY(deleteBtn != nullptr);
    }

    void testActivateButtonForInactiveModule() {
        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mod1", std::make_shared<StubPlugin>());

        ModulePage page;
        page.refreshTable();

        // 未激活的模块应有"激活"按钮
        auto* cellWidget = page.table()->cellWidget(0, 3);
        QVERIFY(cellWidget != nullptr);
        auto* activateBtn = cellWidget->findChild<QPushButton*>("activateButton");
        QVERIFY(activateBtn != nullptr);
    }

    void testNoActivateButtonForActiveModule() {
        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mod1", std::make_shared<StubPlugin>());
        pm.setActivePlugin("mod1");

        ModulePage page;
        page.refreshTable();

        // 已激活的模块不应有"激活"按钮
        auto* cellWidget = page.table()->cellWidget(0, 3);
        QVERIFY(cellWidget != nullptr);
        auto* activateBtn = cellWidget->findChild<QPushButton*>("activateButton");
        QVERIFY(activateBtn == nullptr);
    }

    void testDeleteButtonClickRemovesPlugin() {
        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mod1", std::make_shared<StubPlugin>());

        ModulePage page;
        page.refreshTable();

        // 点击删除按钮
        auto* cellWidget = page.table()->cellWidget(0, 3);
        auto* deleteBtn = cellWidget->findChild<QPushButton*>("deleteButton");
        QVERIFY(deleteBtn != nullptr);
        QTest::mouseClick(deleteBtn, Qt::LeftButton);

        // 插件应被删除，表格应刷新为空
        QCOMPARE(pm.listPlugins().size(), 0);
        QCOMPARE(page.table()->rowCount(), 0);
    }

    void testActivateButtonClickActivatesPlugin() {
        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mod1", std::make_shared<StubPlugin>());

        ModulePage page;
        page.refreshTable();

        // 点击激活按钮
        auto* cellWidget = page.table()->cellWidget(0, 3);
        auto* activateBtn = cellWidget->findChild<QPushButton*>("activateButton");
        QVERIFY(activateBtn != nullptr);
        QTest::mouseClick(activateBtn, Qt::LeftButton);

        // 插件应被激活
        QCOMPARE(pm.activePluginName(), QString("mod1"));
    }

    void testSignalTriggersRefresh() {
        ModulePage page;

        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mod1", std::make_shared<StubPlugin>());

        // 信号连接后，注册新插件应自动刷新表格
        // 注意：ModulePage 构造时就连接了信号，此时 mod1 注册在构造之后，
        // 信号 pluginRegistered 会触发 refreshTable
        QCOMPARE(page.table()->rowCount(), 1);
    }
};

QTEST_MAIN(TestModulePage)
#include "test_modulepage.moc"
