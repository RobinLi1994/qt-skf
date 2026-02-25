/**
 * @file test_mainwindow.cpp
 * @brief MainWindow 单元测试
 */

#include <QApplication>
#include <QListWidget>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTest>

#include "gui/MainWindow.h"

using namespace wekey;

class TestMainWindow : public QObject {
    Q_OBJECT

private slots:
    void testWindowTitle() {
        MainWindow w;
        QCOMPARE(w.windowTitle(), "wekey-skf");
    }

    void testMinimumSize() {
        MainWindow w;
        QVERIFY(w.minimumWidth() >= 900);
        QVERIFY(w.minimumHeight() >= 600);
    }

    void testNavListExists() {
        MainWindow w;
        QVERIFY(w.navList() != nullptr);
    }

    void testContentStackExists() {
        MainWindow w;
        QVERIFY(w.contentStack() != nullptr);
    }

    void testNavItemCount() {
        MainWindow w;
        QCOMPARE(w.navList()->count(), 7);
    }

    void testPageCount() {
        MainWindow w;
        QCOMPARE(w.contentStack()->count(), 7);
    }

    void testNavLabels() {
        MainWindow w;
        struct TestCase {
            int row;
            QString expected;
        };
        TestCase cases[] = {
            {0, "模块管理"},
            {1, "设备管理"},
            {2, "应用管理"},
            {3, "容器管理"},
            {4, "文件管理"},
            {5, "配置管理"},
            {6, "日志查看"},
        };
        for (const auto& tc : cases) {
            QCOMPARE(w.navList()->item(tc.row)->text(), tc.expected);
        }
    }

    void testNavSwitchesPage() {
        MainWindow w;
        w.navList()->setCurrentRow(2);
        QCOMPARE(w.contentStack()->currentIndex(), 2);
    }

    void testStatusBarExists() {
        MainWindow w;
        QVERIFY(w.statusBar() != nullptr);
    }
};

QTEST_MAIN(TestMainWindow)
#include "test_mainwindow.moc"
