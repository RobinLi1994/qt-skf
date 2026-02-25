/**
 * @file test_csrdialog.cpp
 * @brief CsrDialog 单元测试
 */

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTest>

#include "gui/dialogs/CsrDialog.h"

using namespace wekey;

class TestCsrDialog : public QObject {
    Q_OBJECT

private slots:
    void testKeyTypeComboExists() {
        CsrDialog dlg;
        QVERIFY(dlg.keyTypeCombo() != nullptr);
    }

    void testKeyTypeOptions() {
        CsrDialog dlg;
        QCOMPARE(dlg.keyTypeCombo()->count(), 4);
        struct TestCase {
            int index;
            QString expected;
        };
        TestCase cases[] = {
            {0, "SM2 (sm2p256v1)"},
            {1, "RSA-2048"},
            {2, "RSA-3072"},
            {3, "RSA-4096"},
        };
        for (const auto& tc : cases) {
            QCOMPARE(dlg.keyTypeCombo()->itemText(tc.index), tc.expected);
        }
    }

    void testLineEditsExist() {
        CsrDialog dlg;
        QVERIFY(dlg.cnEdit() != nullptr);
        QVERIFY(dlg.orgEdit() != nullptr);
        QVERIFY(dlg.ouEdit() != nullptr);
    }

    void testRegenCheckExists() {
        CsrDialog dlg;
        QVERIFY(dlg.regenCheck() != nullptr);
    }

    void testRegenCheckDefaultUnchecked() {
        CsrDialog dlg;
        QVERIFY(!dlg.regenCheck()->isChecked());
    }

    void testAccessors() {
        CsrDialog dlg;
        dlg.cnEdit()->setText("TestCN");
        dlg.orgEdit()->setText("TestOrg");
        dlg.ouEdit()->setText("TestOU");
        dlg.regenCheck()->setChecked(true);

        QCOMPARE(dlg.commonName(), "TestCN");
        QCOMPARE(dlg.organization(), "TestOrg");
        QCOMPARE(dlg.unit(), "TestOU");
        QVERIFY(dlg.regenerateKey());
    }

    void testWindowTitle() {
        CsrDialog dlg;
        QCOMPARE(dlg.windowTitle(), "生成证书请求 (CSR)");
    }
};

QTEST_MAIN(TestCsrDialog)
#include "test_csrdialog.moc"
