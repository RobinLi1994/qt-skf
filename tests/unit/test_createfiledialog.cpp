/**
 * @file test_createfiledialog.cpp
 * @brief CreateFileDialog 单元测试
 */

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTest>

#include "gui/dialogs/CreateFileDialog.h"

using namespace wekey;

class TestCreateFileDialog : public QObject {
    Q_OBJECT

private slots:
    // --- 基础 UI 测试 ---

    void testConstructor() {
        CreateFileDialog dialog;
        QVERIFY(true);
    }

    void testWindowTitle() {
        CreateFileDialog dialog;
        QCOMPARE(dialog.windowTitle(), "创建文件");
    }

    void testHasFileNameEdit() {
        CreateFileDialog dialog;
        auto edits = dialog.findChildren<QLineEdit*>();
        // 应该至少有文件名和路径两个输入框
        QVERIFY(edits.size() >= 2);
    }

    void testHasPermissionCombos() {
        CreateFileDialog dialog;
        auto combos = dialog.findChildren<QComboBox*>();
        // 应该有读权限和写权限两个下拉框
        QCOMPARE(combos.size(), 2);
    }

    void testHasButtonBox() {
        CreateFileDialog dialog;
        auto* buttonBox = dialog.findChild<QDialogButtonBox*>();
        QVERIFY(buttonBox != nullptr);
    }

    void testHasOkCancelButtons() {
        CreateFileDialog dialog;
        auto* buttonBox = dialog.findChild<QDialogButtonBox*>();
        QVERIFY(buttonBox != nullptr);

        auto* okBtn = buttonBox->button(QDialogButtonBox::Ok);
        auto* cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);

        QVERIFY(okBtn != nullptr);
        QVERIFY(cancelBtn != nullptr);
    }

    // --- 功能测试 ---

    void testInitialOkButtonDisabled() {
        CreateFileDialog dialog;
        auto* buttonBox = dialog.findChild<QDialogButtonBox*>();
        QVERIFY(buttonBox != nullptr);

        auto* okBtn = buttonBox->button(QDialogButtonBox::Ok);
        QVERIFY(okBtn != nullptr);

        // 初始状态应该禁用确定按钮
        QVERIFY(!okBtn->isEnabled());
    }

    void testReadRightsComboHasExpectedOptions() {
        CreateFileDialog dialog;
        auto combos = dialog.findChildren<QComboBox*>();
        QCOMPARE(combos.size(), 2);

        auto* readRightsCombo = combos[0];
        // 应该有三个选项: Everyone, UserOnly, AdminOnly
        QVERIFY(readRightsCombo->count() >= 3);
    }

    void testWriteRightsComboHasExpectedOptions() {
        CreateFileDialog dialog;
        auto combos = dialog.findChildren<QComboBox*>();
        QCOMPARE(combos.size(), 2);

        auto* writeRightsCombo = combos[1];
        // 应该有三个选项: Everyone, UserOnly, AdminOnly
        QVERIFY(writeRightsCombo->count() >= 3);
    }

    void testDefaultPermissions() {
        CreateFileDialog dialog;

        // 读权限默认应该是 Everyone
        QString readRights = dialog.readRights();
        QCOMPARE(readRights, "Everyone");

        // 写权限默认应该是 AdminOnly
        QString writeRights = dialog.writeRights();
        QCOMPARE(writeRights, "AdminOnly");
    }

    void testFileNameGetter() {
        CreateFileDialog dialog;
        // 初始应该为空
        QVERIFY(dialog.fileName().isEmpty());
    }

    void testFilePathGetter() {
        CreateFileDialog dialog;
        // 初始应该为空
        QVERIFY(dialog.filePath().isEmpty());
    }

    void testReadRightsGetter() {
        CreateFileDialog dialog;
        QString rights = dialog.readRights();
        QVERIFY(!rights.isEmpty());
    }

    void testWriteRightsGetter() {
        CreateFileDialog dialog;
        QString rights = dialog.writeRights();
        QVERIFY(!rights.isEmpty());
    }
};

QTEST_MAIN(TestCreateFileDialog)
#include "test_createfiledialog.moc"
