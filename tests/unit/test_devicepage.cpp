/**
 * @file test_devicepage.cpp
 * @brief DevicePage 单元测试
 */

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTest>
#include <memory>

#include "gui/pages/DevicePage.h"
#include "integration/MockPlugin.h"
#include "plugin/PluginManager.h"

using namespace wekey;
using wekey::test::MockPlugin;

class TestDevicePage : public QObject {
    Q_OBJECT

private:
    void cleanupPlugins() {
        auto& pm = PluginManager::instance();
        for (const auto& name : pm.listPlugins()) {
            pm.unregisterPlugin(name);
        }
    }

private slots:
    void init() { cleanupPlugins(); }
    void cleanup() { cleanupPlugins(); }

    // --- 基础 UI 测试 ---

    void testTableExists() {
        DevicePage page;
        QVERIFY(page.table() != nullptr);
    }

    void testRefreshButtonExists() {
        DevicePage page;
        QVERIFY(page.refreshButton() != nullptr);
    }

    void testDetailsGroupExists() {
        DevicePage page;
        QVERIFY(page.detailsGroup() != nullptr);
    }

    void testTableColumnCount() {
        DevicePage page;
        QCOMPARE(page.table()->columnCount(), 4);
    }

    void testTableHeaders() {
        DevicePage page;
        struct TestCase {
            int col;
            QString expected;
        };
        TestCase cases[] = {
            {0, "序列号"},
            {1, "标签"},
            {2, "制造商"},
            {3, "操作"},
        };
        for (const auto& tc : cases) {
            QCOMPARE(page.table()->horizontalHeaderItem(tc.col)->text(), tc.expected);
        }
    }

    void testRefreshButtonText() {
        DevicePage page;
        QCOMPARE(page.refreshButton()->text(), "刷新");
    }

    void testDetailsGroupTitle() {
        DevicePage page;
        QCOMPARE(page.detailsGroup()->title(), "设备详情");
    }

    // --- 功能测试 ---

    void testRefreshTable() {
        auto mock = std::make_shared<MockPlugin>();
        DeviceInfo d1;
        d1.deviceName = "DEV001";
        d1.serialNumber = "SN001";
        d1.label = "MyKey1";
        d1.manufacturer = "VendorA";
        DeviceInfo d2;
        d2.deviceName = "DEV002";
        d2.serialNumber = "SN002";
        d2.label = "MyKey2";
        d2.manufacturer = "VendorB";
        mock->devices_ = {d1, d2};

        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mockDev", mock);
        pm.setActivePlugin("mockDev");

        DevicePage page;
        page.refreshTable();

        QCOMPARE(page.table()->rowCount(), 2);

        struct TestCase {
            int row;
            QString serial;
            QString label;
            QString manufacturer;
        };
        TestCase cases[] = {
            {0, "SN001", "MyKey1", "VendorA"},
            {1, "SN002", "MyKey2", "VendorB"},
        };
        for (const auto& tc : cases) {
            QCOMPARE(page.table()->item(tc.row, 0)->text(), tc.serial);
            QCOMPARE(page.table()->item(tc.row, 1)->text(), tc.label);
            QCOMPARE(page.table()->item(tc.row, 2)->text(), tc.manufacturer);
        }
    }

    void testRefreshTableShowsOperationButtons() {
        auto mock = std::make_shared<MockPlugin>();
        mock->addDevice("DEV001");

        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mockDev", mock);
        pm.setActivePlugin("mockDev");

        DevicePage page;
        page.refreshTable();

        QCOMPARE(page.table()->rowCount(), 1);

        auto* cellWidget = page.table()->cellWidget(0, 3);
        QVERIFY(cellWidget != nullptr);
        auto* setLabelBtn = cellWidget->findChild<QPushButton*>("setLabelButton");
        QVERIFY(setLabelBtn != nullptr);
        auto* changeAuthBtn = cellWidget->findChild<QPushButton*>("changeAuthButton");
        QVERIFY(changeAuthBtn != nullptr);
    }

    void testDeviceDetailsUpdate() {
        auto mock = std::make_shared<MockPlugin>();
        DeviceInfo d;
        d.deviceName = "DEV001";
        d.serialNumber = "SN001";
        d.label = "TestLabel";
        d.manufacturer = "TestVendor";
        d.hardwareVersion = "HW1.0";
        d.firmwareVersion = "FW2.0";
        mock->devices_ = {d};

        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mockDev", mock);
        pm.setActivePlugin("mockDev");

        DevicePage page;
        page.refreshTable();

        // 选中第 0 行触发 updateDetails
        page.table()->setCurrentCell(0, 0);

        auto* group = page.detailsGroup();
        auto labels = group->findChildren<QLabel*>();

        // 验证详情区包含正确的设备信息文本
        bool foundManufacturer = false;
        bool foundHwVersion = false;
        bool foundFwVersion = false;
        for (auto* label : labels) {
            if (label->text() == "TestVendor") foundManufacturer = true;
            if (label->text() == "HW1.0") foundHwVersion = true;
            if (label->text() == "FW2.0") foundFwVersion = true;
        }
        QVERIFY(foundManufacturer);
        QVERIFY(foundHwVersion);
        QVERIFY(foundFwVersion);
    }

    void testRefreshButtonTriggersRefresh() {
        auto mock = std::make_shared<MockPlugin>();
        mock->addDevice("DEV001");

        auto& pm = PluginManager::instance();
        pm.registerPluginInstance("mockDev", mock);
        pm.setActivePlugin("mockDev");

        DevicePage page;
        // 构造时已通过 refreshTable 填充，先清空验证按钮能重新刷新
        mock->addDevice("DEV002");
        page.refreshButton()->click();

        QCOMPARE(page.table()->rowCount(), 2);
    }

    void testEmptyWhenNoActivePlugin() {
        DevicePage page;
        page.refreshTable();

        QCOMPARE(page.table()->rowCount(), 0);
    }
};

QTEST_MAIN(TestDevicePage)
#include "test_devicepage.moc"
