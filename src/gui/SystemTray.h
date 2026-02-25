/**
 * @file SystemTray.h
 * @brief 系统托盘 (M5.3)
 */

#pragma once

#include <QObject>
#include <QSystemTrayIcon>

#include <ElaMenu.h>

namespace wekey {

class SystemTray : public QObject {
    Q_OBJECT

public:
    explicit SystemTray(QObject* parent = nullptr);
    ~SystemTray() override = default;

    QSystemTrayIcon* trayIcon() const;
    ElaMenu* trayMenu() const;

signals:
    void showRequested();
    void exitRequested();

private:
    void setupMenu();

    QSystemTrayIcon* trayIcon_ = nullptr;
    ElaMenu* trayMenu_ = nullptr;
    QAction* showAction_ = nullptr;
    QAction* exitAction_ = nullptr;
};

}  // namespace wekey
