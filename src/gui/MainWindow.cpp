/**
 * @file MainWindow.cpp
 * @brief 主窗口实现
 */

#include "MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>

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
        connect(systemTray_, &SystemTray::showRequested, this, &QWidget::show);
        connect(systemTray_, &SystemTray::exitRequested, qApp, &QApplication::quit);
        // 有托盘时，关闭按钮隐藏到托盘
        setIsDefaultClosed(false);
        connect(this, &MainWindow::closeButtonClicked, this, [this]() {
            hide();
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
    // 此处仅处理系统级关闭事件（如 Alt+F4）
    if (systemTray_ != nullptr) {
        hide();
        event->ignore();
    } else {
        event->ignore();
        closeDialog_->exec();
    }
}

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
