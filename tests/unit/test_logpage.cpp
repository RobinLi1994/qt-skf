/**
 * @file test_logpage.cpp
 * @brief LogPage 单元测试
 */

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QTest>

#include "gui/pages/LogPage.h"

using namespace wekey;

class TestLogPage : public QObject {
    Q_OBJECT

private slots:
    void testSearchEditExists() {
        LogPage page;
        QVERIFY(page.searchEdit() != nullptr);
    }

    void testLevelComboExists() {
        LogPage page;
        QVERIFY(page.levelCombo() != nullptr);
    }

    void testClearButtonExists() {
        LogPage page;
        QVERIFY(page.clearButton() != nullptr);
    }

    void testTableViewExists() {
        LogPage page;
        QVERIFY(page.tableView() != nullptr);
    }

    void testLogModelExists() {
        LogPage page;
        QVERIFY(page.logModel() != nullptr);
    }

    void testLevelComboItems() {
        LogPage page;
        QCOMPARE(page.levelCombo()->count(), 5);
        struct TestCase {
            int index;
            QString expected;
        };
        TestCase cases[] = {
            {0, "All"},
            {1, "Debug"},
            {2, "Info"},
            {3, "Warn"},
            {4, "Error"},
        };
        for (const auto& tc : cases) {
            QCOMPARE(page.levelCombo()->itemText(tc.index), tc.expected);
        }
    }

    void testClearButtonText() {
        LogPage page;
        QCOMPARE(page.clearButton()->text(), "清空");
    }

    void testTableViewHasModel() {
        LogPage page;
        QCOMPARE(page.tableView()->model(), page.logModel());
    }
};

QTEST_MAIN(TestLogPage)
#include "test_logpage.moc"
