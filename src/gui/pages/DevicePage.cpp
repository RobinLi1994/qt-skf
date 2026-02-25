/**
 * @file DevicePage.cpp
 * @brief 设备管理页实现
 *
 * 三级钻取导航：设备列表 → 应用列表 → 应用详情（容器+文件）
 */

#include "DevicePage.h"

#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaScrollPageArea.h>
#include <ElaText.h>

#include "gui/UiHelper.h"
#include "gui/views/AppListView.h"
#include "gui/views/AppDetailView.h"
#include "core/device/DeviceService.h"
#include "gui/dialogs/MessageBox.h"
#include "plugin/PluginManager.h"

namespace wekey {

DevicePage::DevicePage(QWidget* parent) : ElaScrollPage(parent) {
    setWindowTitle("设备管理");
    setTitleVisible(false);
    setupUi();
    connectSignals();
    refreshTable();
}

void DevicePage::setupUi() {
    // === QStackedWidget 管理三级视图 ===
    stack_ = new QStackedWidget(this);

    // --- 视图 0: 设备列表 ---
    deviceListWidget_ = new QWidget(this);
    auto* devLayout = new QVBoxLayout(deviceListWidget_);
    devLayout->setContentsMargins(0, 0, 0, 0);
    devLayout->setSpacing(UiHelper::kSpaceMD);

    // 操作栏
    auto* headerArea = new ElaScrollPageArea(this);
    UiHelper::styleCard(headerArea);
    auto* headerLayout = new QHBoxLayout(headerArea);
    auto* descText = new ElaText("查看和管理已连接的 SKF 设备", this);
    descText->setTextStyle(ElaTextType::Body);
    descText->setWordWrap(false);
    headerLayout->addWidget(descText);
    headerLayout->addStretch();
    refreshButton_ = new ElaPushButton("刷新", this);
    UiHelper::styleDefaultButton(refreshButton_);
    headerLayout->addWidget(refreshButton_);

    // 设备表格：序列号（可点击）、标签、制造商、操作
    table_ = new QTableWidget(0, 4, this);
    table_->setHorizontalHeaderLabels({"序列号", "标签", "制造商", "操作"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    UiHelper::styleTable(table_);

    // 设备详情
    detailsGroup_ = new ElaScrollPageArea(this);
    UiHelper::styleCard(detailsGroup_);
    auto* detailsLayout = new QVBoxLayout(detailsGroup_);
    auto* detailsTitle = new ElaText("设备详情", this);
    detailsTitle->setTextStyle(ElaTextType::BodyStrong);
    detailsLayout->addWidget(detailsTitle);

    auto* detailsForm = new QFormLayout;
    detailsForm->setSpacing(UiHelper::kSpaceSM);
    manufacturerValue_ = new ElaText("-", this);
    manufacturerValue_->setTextStyle(ElaTextType::Body);
    hwVersionValue_ = new ElaText("-", this);
    hwVersionValue_->setTextStyle(ElaTextType::Body);
    fwVersionValue_ = new ElaText("-", this);
    fwVersionValue_->setTextStyle(ElaTextType::Body);
    detailsForm->addRow("制造商:", manufacturerValue_);
    detailsForm->addRow("硬件版本:", hwVersionValue_);
    detailsForm->addRow("固件版本:", fwVersionValue_);
    detailsLayout->addLayout(detailsForm);

    devLayout->addWidget(headerArea);
    devLayout->addWidget(table_, 1);
    devLayout->addWidget(detailsGroup_);

    // --- 视图 1: 应用列表 ---
    appListView_ = new AppListView(this);

    // --- 视图 2: 应用详情（容器+文件 Tab）---
    appDetailView_ = new AppDetailView(this);

    // 添加到 stack
    stack_->addWidget(deviceListWidget_);  // index 0
    stack_->addWidget(appListView_);       // index 1
    stack_->addWidget(appDetailView_);     // index 2
    stack_->setCurrentIndex(0);

    addCentralWidget(stack_, true, true, 0);
}

void DevicePage::connectSignals() {
    connect(refreshButton_, &ElaPushButton::clicked, this, &DevicePage::refreshTable);

    auto& ds = DeviceService::instance();
    connect(&ds, &DeviceService::deviceListChanged, this, &DevicePage::refreshTable);
    connect(&ds, &DeviceService::deviceInserted, this, [this](const QString& /*devName*/) {
        refreshTable();
    });
    connect(&ds, &DeviceService::deviceRemoved, this, [this](const QString& /*devName*/) {
        refreshTable();
    });

    auto& pm = PluginManager::instance();
    connect(&pm, &PluginManager::activePluginChanged, this, &DevicePage::refreshTable);

    connect(table_, &QTableWidget::currentCellChanged, this,
            [this](int row, int /*col*/, int /*prevRow*/, int /*prevCol*/) {
                updateDetails(row);
            });

    // 应用列表视图：返回 → 设备列表，详情 → 应用详情
    connect(appListView_, &AppListView::backRequested, this, &DevicePage::navigateToDeviceList);
    connect(appListView_, &AppListView::detailRequested, this, &DevicePage::navigateToAppDetail);

    // 应用详情视图：返回 → 应用列表
    connect(appDetailView_, &AppDetailView::backRequested, this, [this]() {
        stack_->setCurrentIndex(1);
    });
}

void DevicePage::refreshTable() {
    if (refreshing_) return;
    refreshing_ = true;

    table_->setRowCount(0);
    devices_.clear();

    auto result = DeviceService::instance().enumDevices(false);
    if (!result.isOk()) {
        refreshing_ = false;
        return;
    }

    devices_ = result.value();
    for (const auto& dev : devices_) {
        int row = table_->rowCount();
        table_->insertRow(row);

        // 序列号列：蓝色可点击文字，点击跳转到应用列表
        auto* snLabel = new QLabel(QString("<a href='#' style='color:#1677ff;text-decoration:none;'>%1</a>").arg(dev.serialNumber));
        snLabel->setCursor(Qt::PointingHandCursor);
        snLabel->setContentsMargins(16, 0, 16, 0);
        connect(snLabel, &QLabel::linkActivated, this, [this, name = dev.deviceName]() {
            navigateToAppList(name);
        });
        table_->setCellWidget(row, 0, snLabel);

        table_->setItem(row, 1, new QTableWidgetItem(dev.label));
        table_->setItem(row, 2, new QTableWidgetItem(dev.manufacturer));

        // 操作链接（纯文字带颜色，无按钮边框）
        auto* actionWidget = new QWidget();
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(16);

        auto* setLabelLink = UiHelper::createActionLink(ElaIconType::PenToSquare, "设置标签");
        connect(setLabelLink, &QLabel::linkActivated, this, [this, name = dev.deviceName]() {
            onSetLabel(name);
        });
        actionLayout->addWidget(setLabelLink);

        /*auto* changeAuthLink = UiHelper::createActionLink(ElaIconType::Key, "修改认证");
        connect(changeAuthLink, &QLabel::linkActivated, this, [this, name = dev.deviceName]() {
            onChangeAuth(name);
        });
        actionLayout->addWidget(changeAuthLink);*/

        actionLayout->addStretch();
        table_->setCellWidget(row, 3, actionWidget);
    }

    refreshing_ = false;
}

void DevicePage::updateDetails(int row) {
    if (row < 0 || row >= devices_.size()) {
        manufacturerValue_->setText("-");
        hwVersionValue_->setText("-");
        fwVersionValue_->setText("-");
        return;
    }

    const auto& dev = devices_.at(row);
    manufacturerValue_->setText(dev.manufacturer);
    hwVersionValue_->setText(dev.hardwareVersion);
    fwVersionValue_->setText(dev.firmwareVersion);
}

void DevicePage::onSetLabel(const QString& devName) {
    auto* dialog = new QDialog(this);
    dialog->setWindowTitle("设置设备标签");
    dialog->resize(420, 0);
    UiHelper::styleDialog(dialog);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(4);

    // ---- 设备名称（必填）----
    auto* nameLabel = new QLabel(dialog);
    nameLabel->setTextFormat(Qt::RichText);
    nameLabel->setText(
        "<span style='color:#ff4d4f; font-size:14px;'>* </span>"
        "<span style='color:#000000; font-size:14px;'>设备名称</span>");
    mainLayout->addWidget(nameLabel);

    auto* labelEdit = new ElaLineEdit(dialog);
    UiHelper::styleLineEdit(labelEdit);
    labelEdit->setPlaceholderText("请输入新标签");
    mainLayout->addWidget(labelEdit);
    mainLayout->addSpacing(16);

    // ---- 分隔线 ----
    mainLayout->addWidget(UiHelper::createDivider(dialog));

    // ---- 按钮 ----
    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    auto* cancelBtn = new ElaPushButton("取消", dialog);
    UiHelper::styleDefaultButton(cancelBtn);
    connect(cancelBtn, &ElaPushButton::clicked, dialog, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto* okBtn = new ElaPushButton("确定", dialog);
    UiHelper::stylePrimaryButton(okBtn);
    okBtn->setEnabled(false);
    connect(okBtn, &ElaPushButton::clicked, dialog, &QDialog::accept);
    btnLayout->addWidget(okBtn);
    mainLayout->addLayout(btnLayout);

    // 输入内容变化时启用/禁用确定按钮
    connect(labelEdit, &ElaLineEdit::textChanged, dialog, [okBtn](const QString& text) {
        okBtn->setEnabled(!text.trimmed().isEmpty());
    });

    if (dialog->exec() != QDialog::Accepted || labelEdit->text().isEmpty()) {
        dialog->deleteLater();
        return;
    }

    QString newLabel = labelEdit->text();
    dialog->deleteLater();

    auto result = DeviceService::instance().setDeviceLabel(devName, newLabel);
    if (!result.isOk()) {
        MessageBox::error(this, "设置标签失败", result.error());
    } else {
        refreshTable();
    }
}

void DevicePage::onChangeAuth(const QString& devName) {
    auto* dialog = new QDialog(this);
    dialog->setWindowTitle("修改设备认证密钥");
    dialog->setMinimumWidth(350);

    auto* layout = new QVBoxLayout(dialog);
    auto* formLayout = new QFormLayout;
    auto* oldPinEdit = new ElaLineEdit(dialog);
    oldPinEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("旧密钥:", oldPinEdit);
    auto* newPinEdit = new ElaLineEdit(dialog);
    newPinEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("新密钥:", newPinEdit);
    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    auto* cancelBtn = new ElaPushButton("取消", dialog);
    auto* okBtn = new ElaPushButton("确定", dialog);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(okBtn);
    layout->addLayout(btnLayout);

    connect(okBtn, &ElaPushButton::clicked, dialog, &QDialog::accept);
    connect(cancelBtn, &ElaPushButton::clicked, dialog, &QDialog::reject);

    if (dialog->exec() != QDialog::Accepted) {
        dialog->deleteLater();
        return;
    }

    QString oldPin = oldPinEdit->text();
    QString newPin = newPinEdit->text();
    dialog->deleteLater();

    auto result = DeviceService::instance().changeDeviceAuth(devName, oldPin, newPin);
    if (!result.isOk()) {
        MessageBox::error(this, "修改认证密钥失败", result.error());
    } else {
        MessageBox::info(this, "成功", "设备认证密钥已修改");
    }
}

void DevicePage::navigateToAppList(const QString& devName) {
    qDebug() << "[DevicePage] navigateToAppList:" << devName;
    appListView_->setDevice(devName);
    stack_->setCurrentIndex(1);
}

void DevicePage::navigateToAppDetail(const QString& devName, const QString& appName) {
    qDebug() << "[DevicePage] navigateToAppDetail:" << devName << appName;
    appDetailView_->setContext(devName, appName);
    stack_->setCurrentIndex(2);
}

void DevicePage::navigateToDeviceList() {
    qDebug() << "[DevicePage] navigateToDeviceList";
    stack_->setCurrentIndex(0);
    refreshTable();
}

void DevicePage::showEvent(QShowEvent* event) {
    ElaScrollPage::showEvent(event);
    // 每次通过左侧导航菜单切换到设备管理时，重置到设备列表页
    // 仅在非设备列表页时才重置（避免首次显示时与构造函数中的 refreshTable 重复）
    if (stack_->currentIndex() != 0) {
        stack_->setCurrentIndex(0);
        // 延迟刷新：让页面切换动画先完成，避免同步刷新阻塞 UI 导致卡顿
        QTimer::singleShot(0, this, &DevicePage::refreshTable);
    }
}

}  // namespace wekey
