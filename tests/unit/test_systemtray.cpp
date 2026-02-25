/**
 * @file test_systemtray.cpp
 * @brief SystemTray 单元测试
 */

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSignalSpy>
#include <QSystemTrayIcon>
#include <QTest>

#include "gui/SystemTray.h"

using namespace wekey;

class TestSystemTray : public QObject {
    Q_OBJECT

private slots:
    void testTrayIconExists() {
        SystemTray tray;
        QVERIFY(tray.trayIcon() != nullptr);
    }

    void testTrayMenuExists() {
        SystemTray tray;
        QVERIFY(tray.trayMenu() != nullptr);
    }

    void testMenuItemCount() {
        SystemTray tray;
        auto actions = tray.trayMenu()->actions();
        // "显示主窗口", separator, "退出"
        QCOMPARE(actions.size(), 3);
    }

    void testMenuLabels() {
        SystemTray tray;
        auto actions = tray.trayMenu()->actions();
        QCOMPARE(actions.at(0)->text(), "显示主窗口");
        QVERIFY(actions.at(1)->isSeparator());
        QCOMPARE(actions.at(2)->text(), "退出");
    }

    void testShowSignal() {
        SystemTray tray;
        QSignalSpy spy(&tray, &SystemTray::showRequested);
        auto actions = tray.trayMenu()->actions();
        actions.at(0)->trigger();
        QCOMPARE(spy.count(), 1);
    }

    void testExitSignal() {
        SystemTray tray;
        QSignalSpy spy(&tray, &SystemTray::exitRequested);
        auto actions = tray.trayMenu()->actions();
        actions.at(2)->trigger();
        QCOMPARE(spy.count(), 1);
    }

    void testTooltip() {
        SystemTray tray;
        QCOMPARE(tray.trayIcon()->toolTip(), "wekey-skf");
    }
};

QTEST_MAIN(TestSystemTray)
#include "test_systemtray.moc"
