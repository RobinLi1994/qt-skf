/**
 * @file test_importcertdialog.cpp
 * @brief ImportCertDialog 单元测试
 */

#include <QApplication>
#include <QPushButton>
#include <QTest>
#include <QTextEdit>

#include "gui/dialogs/ImportCertDialog.h"

using namespace wekey;

class TestImportCertDialog : public QObject {
    Q_OBJECT

private slots:
    void testCertEditExists() {
        ImportCertDialog dlg;
        QVERIFY(dlg.certEdit() != nullptr);
    }

    void testBrowseButtonExists() {
        ImportCertDialog dlg;
        QVERIFY(dlg.browseButton() != nullptr);
    }

    void testBrowseButtonText() {
        ImportCertDialog dlg;
        QCOMPARE(dlg.browseButton()->text(), "从文件导入...");
    }

    void testCertPemAccessor() {
        ImportCertDialog dlg;
        QString pem = "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----";
        dlg.certEdit()->setPlainText(pem);
        QCOMPARE(dlg.certPem(), pem);
    }

    void testWindowTitle() {
        ImportCertDialog dlg;
        QCOMPARE(dlg.windowTitle(), "导入证书");
    }

    void testMinimumSize() {
        ImportCertDialog dlg;
        QVERIFY(dlg.minimumWidth() >= 400);
        QVERIFY(dlg.minimumHeight() >= 300);
    }
};

QTEST_MAIN(TestImportCertDialog)
#include "test_importcertdialog.moc"
