/**
 * @file test_configpage.cpp
 * @brief ConfigPage 单元测试
 */

#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTest>

#include "gui/pages/ConfigPage.h"

using namespace wekey;

class TestConfigPage : public QObject {
    Q_OBJECT

private slots:
    void testLineEditsExist() {
        ConfigPage page;
        struct TestCase {
            const char* name;
            QLineEdit* edit;
        };
        TestCase cases[] = {
            {"appName", page.appNameEdit()},
            {"containerName", page.containerNameEdit()},
            {"commonName", page.commonNameEdit()},
            {"organization", page.organizationEdit()},
            {"unit", page.unitEdit()},
        };
        for (const auto& tc : cases) {
            QVERIFY2(tc.edit != nullptr, tc.name);
        }
    }

    void testRadioButtonsExist() {
        ConfigPage page;
        QVERIFY(page.roleUserRadio() != nullptr);
        QVERIFY(page.roleAdminRadio() != nullptr);
        QVERIFY(page.errorSimpleRadio() != nullptr);
        QVERIFY(page.errorDetailedRadio() != nullptr);
    }

    void testPortSpinExists() {
        ConfigPage page;
        QVERIFY(page.portSpin() != nullptr);
    }

    void testPortSpinRange() {
        ConfigPage page;
        QCOMPARE(page.portSpin()->minimum(), 1024);
        QCOMPARE(page.portSpin()->maximum(), 65535);
    }

    void testButtonsExist() {
        ConfigPage page;
        QVERIFY(page.saveButton() != nullptr);
        QVERIFY(page.resetButton() != nullptr);
    }

    void testButtonLabels() {
        ConfigPage page;
        QCOMPARE(page.saveButton()->text(), "保存");
        QCOMPARE(page.resetButton()->text(), "恢复默认");
    }

    void testDefaultRoleSelected() {
        ConfigPage page;
        // Default config has "user" role
        QVERIFY(page.roleUserRadio()->isChecked() || page.roleAdminRadio()->isChecked());
    }

    void testDefaultErrorModeSelected() {
        ConfigPage page;
        QVERIFY(page.errorSimpleRadio()->isChecked() || page.errorDetailedRadio()->isChecked());
    }
};

QTEST_MAIN(TestConfigPage)
#include "test_configpage.moc"
