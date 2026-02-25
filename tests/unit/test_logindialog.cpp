/**
 * @file test_logindialog.cpp
 * @brief LoginDialog 单元测试
 */

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QTest>

#include "gui/dialogs/LoginDialog.h"

using namespace wekey;

class TestLoginDialog : public QObject {
    Q_OBJECT

private slots:
    void testPinEditExists() {
        LoginDialog dlg;
        QVERIFY(dlg.pinEdit() != nullptr);
    }

    void testPinEditIsMasked() {
        LoginDialog dlg;
        QCOMPARE(dlg.pinEdit()->echoMode(), QLineEdit::Password);
    }

    void testRoleRadiosExist() {
        LoginDialog dlg;
        QVERIFY(dlg.userRadio() != nullptr);
        QVERIFY(dlg.adminRadio() != nullptr);
    }

    void testDefaultRoleIsUser() {
        LoginDialog dlg;
        QVERIFY(dlg.userRadio()->isChecked());
        QVERIFY(!dlg.adminRadio()->isChecked());
    }

    void testRetryLabelExists() {
        LoginDialog dlg;
        QVERIFY(dlg.retryLabel() != nullptr);
    }

    void testSetRetryCount() {
        LoginDialog dlg;
        struct TestCase {
            int count;
            QString expected;
        };
        TestCase cases[] = {
            {5, "剩余尝试次数: 5"},
            {3, "剩余尝试次数: 3"},
            {0, "剩余尝试次数: 0"},
        };
        for (const auto& tc : cases) {
            dlg.setRetryCount(tc.count);
            QCOMPARE(dlg.retryLabel()->text(), tc.expected);
        }
    }

    void testPinAccessor() {
        LoginDialog dlg;
        dlg.pinEdit()->setText("123456");
        QCOMPARE(dlg.pin(), "123456");
    }

    void testRoleAccessor() {
        LoginDialog dlg;
        dlg.userRadio()->setChecked(true);
        QCOMPARE(dlg.role(), "user");
        dlg.adminRadio()->setChecked(true);
        QCOMPARE(dlg.role(), "admin");
    }

    void testWindowTitle() {
        LoginDialog dlg;
        QCOMPARE(dlg.windowTitle(), "登录应用");
    }
};

QTEST_MAIN(TestLoginDialog)
#include "test_logindialog.moc"
