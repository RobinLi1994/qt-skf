/**
 * @file AppListView.cpp
 * @brief 应用列表子视图实现
 */

#include "AppListView.h"

#include <QButtonGroup>
#include <QDialog>
#include <QEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QHeaderView>
#include <QVBoxLayout>

#include <ElaContentDialog.h>
#include <ElaIcon.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaRadioButton.h>
#include <ElaText.h>

#include "gui/UiHelper.h"
#include "core/application/AppService.h"
#include "gui/dialogs/CreateAppDialog.h"
#include "gui/dialogs/LoginDialog.h"
#include "gui/dialogs/MessageBox.h"

namespace wekey {

AppListView::AppListView(QWidget* parent) : QWidget(parent) {
    setupUi();
    connectSignals();
}

void AppListView::setDevice(const QString& devName) {
    devName_ = devName;
    titleText_->setText(QString("设备 %1 的应用列表").arg(devName_));
    refreshApps();
}

void AppListView::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(UiHelper::kSpaceMD);

    // 标题栏：返回箭头 + 标题
    auto* headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(UiHelper::kSpaceSM);
    backButton_ = new QLabel("←", this);
    backButton_->setCursor(Qt::PointingHandCursor);
    backButton_->setStyleSheet(
        "QLabel { color: #000000; font-size: 20px; }"
        "QLabel:hover { color: #1677ff; }"
    );
    backButton_->installEventFilter(this);
    headerLayout->addWidget(backButton_);
    titleText_ = new ElaText("应用列表", this);
    titleText_->setTextPixelSize(20);
    headerLayout->addWidget(titleText_);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // 操作栏：创建 + 刷新
    auto* actionLayout = new QHBoxLayout;
    createButton_ = new ElaPushButton("创建应用", this);
    UiHelper::stylePrimaryButton(createButton_);
    actionLayout->addWidget(createButton_);
    refreshButton_ = new ElaPushButton("刷新", this);
    UiHelper::styleDefaultButton(refreshButton_);
    actionLayout->addWidget(refreshButton_);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);

    // 应用表格
    table_ = new QTableWidget(0, 3, this);
    table_->setHorizontalHeaderLabels({"应用名称", "登录状态", "操作"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    table_->horizontalHeader()->resizeSection(2, 350);
    UiHelper::styleTable(table_);
    mainLayout->addWidget(table_, 1);
}

void AppListView::connectSignals() {
    // backButton_ 是 QLabel，点击通过 eventFilter 处理
    connect(createButton_, &ElaPushButton::clicked, this, &AppListView::onCreateApp);
    connect(refreshButton_, &ElaPushButton::clicked, this, &AppListView::refreshApps);
}

void AppListView::refreshApps() {
    if (refreshing_ || devName_.isEmpty()) return;
    refreshing_ = true;

    table_->setRowCount(0);

    auto result = AppService::instance().enumApps(devName_);
    if (!result.isOk()) {
        refreshing_ = false;
        return;
    }

    for (const auto& app : result.value()) {
        int row = table_->rowCount();
        table_->insertRow(row);

        table_->setItem(row, 0, new QTableWidgetItem(app.appName));
        // 登录状态 Tag
        auto* statusTag = app.isLoggedIn
            ? UiHelper::createSuccessTag("已登录")
            : UiHelper::createDefaultTag("未登录");
        table_->setCellWidget(row, 1, statusTag);

        // 操作链接（纯文字带颜色，无按钮边框）
        auto* actionWidget = new QWidget();
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(16);

        if (!app.isLoggedIn) {
            auto* loginLink = UiHelper::createActionLink(ElaIconType::RightToBracket, "登录");
            connect(loginLink, &QLabel::linkActivated, this,
                    [this, a = app.appName]() { onLogin(a); });
            actionLayout->addWidget(loginLink);
        } else {
            auto* logoutLink = UiHelper::createActionLink(ElaIconType::RightFromBracket, "登出");
            connect(logoutLink, &QLabel::linkActivated, this,
                    [this, a = app.appName]() { onLogout(a); });
            actionLayout->addWidget(logoutLink);
        }

        auto* changePinLink = UiHelper::createActionLink(ElaIconType::PenToSquare, "编辑 PIN");
        connect(changePinLink, &QLabel::linkActivated, this,
                [this, a = app.appName]() { onChangePin(a); });
        actionLayout->addWidget(changePinLink);

        // 详情：未登录时灰色不可点击
        if (app.isLoggedIn) {
            auto* detailLink = UiHelper::createActionLink(ElaIconType::FileLines, "详情");
            connect(detailLink, &QLabel::linkActivated, this,
                    [this, a = app.appName]() { emit detailRequested(devName_, a); });
            actionLayout->addWidget(detailLink);
        } else {
            auto* detailLink = UiHelper::createDisabledLink(ElaIconType::FileLines, "详情");
            actionLayout->addWidget(detailLink);
        }

        auto* deleteLink = UiHelper::createDangerLink(ElaIconType::TrashCan, "删除");
        connect(deleteLink, &QLabel::linkActivated, this,
                [this, a = app.appName]() { onDeleteApp(a); });
        actionLayout->addWidget(deleteLink);

        actionLayout->addStretch();
        table_->setCellWidget(row, 2, actionWidget);
    }

    refreshing_ = false;
}

void AppListView::onCreateApp() {
    if (devName_.isEmpty()) {
        MessageBox::error(this, "创建应用失败", "未选择设备");
        return;
    }

    CreateAppDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString appName = dialog.appName();
    QVariantMap args = dialog.toArgs();
    qDebug() << "[AppListView] 创建应用:" << appName << "args:" << args;

    auto result = AppService::instance().createApp(devName_, appName, args);
    if (!result.isOk()) {
        MessageBox::error(this, "创建应用失败", result.error());
    } else {
        refreshApps();
    }
}

void AppListView::onDeleteApp(const QString& appName) {
    auto* confirmDialog = new ElaContentDialog(this);
    confirmDialog->setWindowTitle("删除提示");
    confirmDialog->setMinimumWidth(500);  // 设置最小宽度确保文字完整显示
    confirmDialog->setLeftButtonText("取消");
    confirmDialog->setRightButtonText("确定");

    // 隐藏中间按钮（需要在 setCentralWidget 之前设置）
    confirmDialog->setMiddleButtonText("");
    // 通过 findChild 查找并隐藏中间按钮
    auto buttons = confirmDialog->findChildren<ElaPushButton*>();
    if (buttons.size() >= 2) {
        buttons[1]->setVisible(false);  // 中间按钮是第二个
    }

    // 创建中心内容：图标 + 提示文字
    auto* centralWidget = new QWidget(confirmDialog);
    auto* layout = new QHBoxLayout(centralWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);

    // 警告图标
    auto* iconLabel = new QLabel("⚠️", centralWidget);
    iconLabel->setStyleSheet("font-size: 24px;");
    layout->addWidget(iconLabel);

    // 提示文字
    auto* textLabel = new QLabel(QString("确定要删除应用 %1 吗？此操作不可恢复！").arg(appName), centralWidget);
    textLabel->setStyleSheet("font-size: 14px; color: #000000;");
    textLabel->setWordWrap(true);
    layout->addWidget(textLabel, 1);

    confirmDialog->setCentralWidget(centralWidget);

    connect(confirmDialog, &ElaContentDialog::rightButtonClicked, this,
            [this, appName]() {
                auto result = AppService::instance().deleteApp(devName_, appName);
                if (!result.isOk()) {
                    MessageBox::error(this, "删除应用失败", result.error());
                } else {
                    refreshApps();
                }
            });

    confirmDialog->exec();
}

void AppListView::onLogin(const QString& appName) {
    LoginDialog dialog(this);
    dialog.setWindowTitle(QString("登录应用 %1").arg(appName));

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString selectedRole = dialog.role();
    QString inputPin = dialog.pin();
    qDebug() << "[onLogin] 尝试登录, app:" << appName << "role:" << selectedRole;

    auto result = AppService::instance().login(devName_, appName, selectedRole, inputPin);
    if (result.isOk()) {
        qDebug() << "[onLogin] 登录成功";
        refreshApps();
        return;
    }
    MessageBox::error(this, "登录失败", result.error());
}

void AppListView::onLogout(const QString& appName) {
    auto result = AppService::instance().logout(devName_, appName);
    if (!result.isOk()) {
        MessageBox::error(this, "登出失败", result.error());
    } else {
        refreshApps();
    }
}

void AppListView::onChangePin(const QString& appName) {
    auto* dialog = new QDialog(this);
    dialog->setWindowTitle(QString("编辑应用 %1 的PIN码").arg(appName));
    dialog->resize(420, 0);
    UiHelper::styleDialog(dialog);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(4);

    // ---- 角色选择（必填）----
    auto* roleLabel = new QLabel(dialog);
    roleLabel->setTextFormat(Qt::RichText);
    roleLabel->setText(
        "<span style='color:#ff4d4f; font-size:14px;'>* </span>"
        "<span style='color:#000000; font-size:14px;'>角色</span>");
    mainLayout->addWidget(roleLabel);

    auto* roleLayout = new QHBoxLayout;
    roleLayout->setContentsMargins(0, 0, 0, 0);
    roleLayout->setSpacing(16);
    auto* adminRadio = new ElaRadioButton("管理员", dialog);
    auto* userRadio = new ElaRadioButton("用户", dialog);
    auto* roleGroup = new QButtonGroup(dialog);
    roleGroup->addButton(adminRadio);
    roleGroup->addButton(userRadio);
    userRadio->setChecked(true);
    roleLayout->addWidget(adminRadio);
    roleLayout->addWidget(userRadio);
    roleLayout->addStretch();
    mainLayout->addLayout(roleLayout);
    mainLayout->addSpacing(12);

    // ---- 原PIN码（必填）----
    auto* oldPinLabel = new QLabel(dialog);
    oldPinLabel->setTextFormat(Qt::RichText);
    oldPinLabel->setText(
        "<span style='color:#ff4d4f; font-size:14px;'>* </span>"
        "<span style='color:#000000; font-size:14px;'>原PIN码</span>");
    mainLayout->addWidget(oldPinLabel);

    auto* oldPinEdit = new ElaLineEdit(dialog);
    UiHelper::styleLineEdit(oldPinEdit);
    oldPinEdit->setEchoMode(QLineEdit::Password);
    oldPinEdit->setPlaceholderText("请输入原PIN码");
    mainLayout->addWidget(oldPinEdit);
    mainLayout->addSpacing(12);

    // ---- 新PIN码（必填）----
    auto* newPinLabel = new QLabel(dialog);
    newPinLabel->setTextFormat(Qt::RichText);
    newPinLabel->setText(
        "<span style='color:#ff4d4f; font-size:14px;'>* </span>"
        "<span style='color:#000000; font-size:14px;'>新PIN码</span>");
    mainLayout->addWidget(newPinLabel);

    auto* newPinEdit = new ElaLineEdit(dialog);
    UiHelper::styleLineEdit(newPinEdit);
    newPinEdit->setEchoMode(QLineEdit::Password);
    newPinEdit->setPlaceholderText("请输入新PIN码");
    mainLayout->addWidget(newPinEdit);
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
    auto validateInputs = [okBtn, oldPinEdit, newPinEdit]() {
        okBtn->setEnabled(!oldPinEdit->text().isEmpty() && !newPinEdit->text().isEmpty());
    };
    connect(oldPinEdit, &ElaLineEdit::textChanged, dialog, validateInputs);
    connect(newPinEdit, &ElaLineEdit::textChanged, dialog, validateInputs);

    if (dialog->exec() != QDialog::Accepted) {
        dialog->deleteLater();
        return;
    }

    QString role = adminRadio->isChecked() ? "admin" : "user";
    QString oldPin = oldPinEdit->text();
    QString newPin = newPinEdit->text();
    dialog->deleteLater();

    auto result = AppService::instance().changePin(devName_, appName, role, oldPin, newPin);
    if (!result.isOk()) {
        MessageBox::error(this, "修改 PIN 失败", result.error());
    } else {
        MessageBox::info(this, "成功", "PIN 已修改");
    }
}

void AppListView::onUnlockPin(const QString& appName) {
    auto* dialog = new QDialog(this);
    dialog->setWindowTitle("解锁 PIN");
    dialog->setMinimumWidth(350);

    auto* mainLayout = new QVBoxLayout(dialog);
    auto* formLayout = new QFormLayout;

    auto* adminPinEdit = new ElaLineEdit(dialog);
    adminPinEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("管理员 PIN:", adminPinEdit);

    auto* newUserPinEdit = new ElaLineEdit(dialog);
    newUserPinEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("新用户 PIN:", newUserPinEdit);
    mainLayout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    auto* cancelBtn = new ElaPushButton("取消", dialog);
    auto* okBtn = new ElaPushButton("确定", dialog);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(okBtn);
    mainLayout->addLayout(btnLayout);

    connect(okBtn, &ElaPushButton::clicked, dialog, &QDialog::accept);
    connect(cancelBtn, &ElaPushButton::clicked, dialog, &QDialog::reject);

    if (dialog->exec() != QDialog::Accepted) {
        dialog->deleteLater();
        return;
    }

    QString adminPin = adminPinEdit->text();
    QString newUserPin = newUserPinEdit->text();
    dialog->deleteLater();

    auto result = AppService::instance().unlockPin(devName_, appName, adminPin, newUserPin, {});
    if (!result.isOk()) {
        MessageBox::error(this, "解锁 PIN 失败", result.error());
    } else {
        MessageBox::info(this, "成功", "PIN 已解锁");
        refreshApps();
    }
}

bool AppListView::eventFilter(QObject* obj, QEvent* event) {
    if (obj == backButton_ && event->type() == QEvent::MouseButtonRelease) {
        emit backRequested();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

}  // namespace wekey
