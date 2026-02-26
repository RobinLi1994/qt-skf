/**
 * @file MainWindow.cpp
 * @brief 主窗口实现
 */

#include "MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <ElaContentDialog.h>
#include <ElaStatusBar.h>
#include <ElaText.h>
#include <ElaTheme.h>

#include "SystemTray.h"
#include "config/Config.h"
#include "pages/ConfigPage.h"
#include "pages/DevicePage.h"
#include "pages/LogPage.h"
#include "pages/ModulePage.h"

namespace wekey {

MainWindow::MainWindow(QWidget* parent) : ElaWindow(parent) {
    // 基本窗口设置
    setWindowTitle("wekey-skf");
    resize(1200, 740);
    setWindowButtonFlag(ElaAppBarType::StayTopButtonHint, false);
    setWindowButtonFlag(ElaAppBarType::ThemeChangeButtonHint, false);
    // Auto 模式：窗口宽时展开导航栏，窄时自动折叠
    setNavigationBarDisplayMode(ElaNavigationType::Auto);

    // 用户信息卡片
    setUserInfoCardVisible(true);
    setUserInfoCardTitle("wekey-skf");
    setUserInfoCardSubTitle("SKF 设备管理工具");
    // 加载 logo 图标（通过 QIcon 渲染 SVG 为 QPixmap）
    QPixmap logoPix = QIcon(":/icons/logo.svg").pixmap(60, 60);
    if (!logoPix.isNull()) {
        setUserInfoCardPixmap(logoPix);
    }

    setupNavigation();

    // 状态栏（用单个容器避免 QStatusBar 默认分隔线）
    auto* statusBar = new ElaStatusBar(this);
    statusBar->setSizeGripEnabled(false);
    auto* statusWidget = new QWidget(this);
    auto* statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(4);
    auto* statusText = new ElaText("就绪", this);
    statusText->setTextPixelSize(14);
    statusLayout->addWidget(statusText);
    auto* statusIcon = new QLabel("●", this);
    statusIcon->setStyleSheet("color: #2ecc71; font-size: 14px;");
    statusIcon->setFixedSize(16, 16);
    statusIcon->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(statusIcon);
    statusBar->addWidget(statusWidget);
    setStatusBar(statusBar);

    // 关闭确认对话框
    closeDialog_ = new ElaContentDialog(this);
    closeDialog_->setLeftButtonText("取消");
    closeDialog_->setMiddleButtonText("最小化");
    closeDialog_->setRightButtonText("退出");
    connect(closeDialog_, &ElaContentDialog::rightButtonClicked, this, &MainWindow::closeWindow);
    connect(closeDialog_, &ElaContentDialog::middleButtonClicked, this, [this]() {
        closeDialog_->close();
        showMinimized();
    });

    // 系统托盘
    if (!Config::instance().systrayDisabled()) {
        systemTray_ = new SystemTray(this);
        // 修复：show() 后调用 raise()+activateWindow() 才能在 macOS 上真正浮到前台
        connect(systemTray_, &SystemTray::showRequested, this, &MainWindow::showWindow);
        connect(systemTray_, &SystemTray::exitRequested, qApp, &QApplication::quit);

        // 修复 macOS：Dock 图标点击会触发 ApplicationActive，此时若窗口隐藏则重新显示
#ifdef Q_OS_MAC
        connect(qApp, &QGuiApplication::applicationStateChanged, this,
                [this](Qt::ApplicationState state) {
                    if (state == Qt::ApplicationActive && !isVisible()) {
                        showWindow();
                    }
                });
#endif

        // 有托盘时，关闭按钮隐藏到托盘
        setIsDefaultClosed(false);
        connect(this, &MainWindow::closeButtonClicked, this, [this]() {
            hide();
            // 修复 Windows：hide() 后延迟一个事件循环再重新注册托盘图标，
            // 确保 Windows 消息泵已处理完 WM_HIDE，避免注册时句柄状态不稳定
#ifdef Q_OS_WIN
            QTimer::singleShot(0, this, [this]() {
                if (systemTray_) systemTray_->reinstall();
            });
#endif
        });
    } else {
        // 无托盘时，关闭按钮弹出确认对话框
        setIsDefaultClosed(false);
        connect(this, &MainWindow::closeButtonClicked, this, [this]() {
            closeDialog_->exec();
        });
    }

    moveToCenter();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // ElaWindow 通过 closeButtonClicked 信号处理关闭逻辑
    // 此处仅处理系统级关闭事件（如 Alt+F4 / macOS Cmd+Q）
    if (systemTray_ != nullptr) {
        hide();
        event->ignore();
#ifdef Q_OS_WIN
        systemTray_->reinstall();
#endif
    } else {
        event->ignore();
        closeDialog_->exec();
    }
}

void MainWindow::showWindow() {
    show();
    raise();
    activateWindow();
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    // 修复 Windows：Explorer 重启后（如系统更新）托盘图标会丢失，
    // TaskbarCreated 消息通知所有程序重新注册托盘图标
    // 使用局部 static 避免全局初始化时 windows.h 类型污染 macOS clangd
    static const UINT wmTaskbarCreated = ::RegisterWindowMessage(L"TaskbarCreated");
    const MSG* msg = static_cast<const MSG*>(message);
    if (msg->message == wmTaskbarCreated && systemTray_) {
        systemTray_->reinstall();
    }
    return ElaWindow::nativeEvent(eventType, message, result);
}
#endif

void MainWindow::setupNavigation() {
    // 使用 ElaWindow 内置导航栏添加页面节点
    // ElaIconType 图标替代原有 SVG 图标
    addPageNode("模块管理", new ModulePage, ElaIconType::Puzzle);
    addPageNode("设备管理", new DevicePage, ElaIconType::MicrochipAi);
    addPageNode("配置管理", new ConfigPage, ElaIconType::Gears);
    addPageNode("日志查看", new LogPage, ElaIconType::ClipboardList);

    // 底部导航节点：配置和日志
    /*QString configKey;
    addFooterNode("配置管理", new ConfigPage, configKey, 0, ElaIconType::Gears);
    QString logKey;
    addFooterNode("日志查看", new LogPage, logKey, 0, ElaIconType::ClipboardList);*/
}

}  // namespace wekey
