/**
 * @file SystemTray.cpp
 * @brief 系统托盘实现
 */

#include "SystemTray.h"

#include <QIcon>

#include <ElaMenu.h>

namespace wekey {

SystemTray::SystemTray(QObject* parent) : QObject(parent) {
    trayIcon_ = new QSystemTrayIcon(this);
    trayIcon_->setIcon(QIcon(":/icons/app.png"));
    trayIcon_->setToolTip("wekey-skf");

    setupMenu();

    connect(trayIcon_, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                // 单击或双击均可打开主窗口；右键菜单由 contextMenu 单独处理
                if (reason == QSystemTrayIcon::Trigger ||
                    reason == QSystemTrayIcon::DoubleClick) {
                    emit showRequested();
                }
            });

    trayIcon_->show();
}

QSystemTrayIcon* SystemTray::trayIcon() const { return trayIcon_; }

ElaMenu* SystemTray::trayMenu() const { return trayMenu_; }

void SystemTray::reinstall() {
    // 仅在 Windows Explorer 重启（TaskbarCreated）后重新注册图标。
    // 不在此处调用 hide()：hide() 会触发 Shell_NotifyIcon(NIM_DELETE) 移除图标，
    // 若紧接着的 show() 在 shell 繁忙时失败，图标将永久消失。
    trayIcon_->show();
}

void SystemTray::setupMenu() {
    trayMenu_ = new ElaMenu;

    showAction_ = trayMenu_->addElaIconAction(ElaIconType::WindowRestore, "显示主窗口");
    connect(showAction_, &QAction::triggered, this, &SystemTray::showRequested);

    trayMenu_->addSeparator();

    exitAction_ = trayMenu_->addElaIconAction(ElaIconType::ArrowRightFromBracket, "退出");
    connect(exitAction_, &QAction::triggered, this, &SystemTray::exitRequested);

    trayIcon_->setContextMenu(trayMenu_);
}

}  // namespace wekey
