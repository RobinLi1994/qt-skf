/**
 * @file CreateAppDialog.cpp
 * @brief 创建应用对话框实现
 *
 * 参考 Go 实现逻辑，使用 ElaWidgetTools 组件库。
 * 字段：应用名称(必填)、管理员PIN(必填)、管理员重试次数(必填)、
 *       用户PIN(必填)、用户重试次数(必填)
 */

#include "CreateAppDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaSpinBox.h>

#include "gui/UiHelper.h"

namespace wekey {

CreateAppDialog::CreateAppDialog(QWidget* parent) : QDialog(parent) {
    setupUi();
    setWindowTitle("创建应用");
    resize(480, 0);
    UiHelper::styleDialog(this);
}

void CreateAppDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(4);

    // ---- 应用名称（必填）----
    mainLayout->addWidget(createRequiredLabel("应用名称"));
    nameEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(nameEdit_);
    nameEdit_->setPlaceholderText("请输入应用名称");
    connect(nameEdit_, &ElaLineEdit::textChanged, this, &CreateAppDialog::validate);
    mainLayout->addWidget(nameEdit_);
    mainLayout->addSpacing(12);

    // ---- 管理员PIN码（必填）----
    mainLayout->addWidget(createRequiredLabel("管理员PIN码"));
    adminPinEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(adminPinEdit_);
    adminPinEdit_->setPlaceholderText("请输入管理员PIN码");
    adminPinEdit_->setEchoMode(QLineEdit::Password);
    connect(adminPinEdit_, &ElaLineEdit::textChanged, this, &CreateAppDialog::validate);
    mainLayout->addWidget(adminPinEdit_);
    mainLayout->addSpacing(12);

    // ---- 管理员重试次数（必填）----
    mainLayout->addWidget(createRequiredLabel("管理员重试次数"));
    adminRetrySpin_ = new ElaSpinBox(this);
    adminRetrySpin_->setRange(1, 10);
    adminRetrySpin_->setValue(3);
    adminRetrySpin_->setFixedHeight(36);
    mainLayout->addWidget(adminRetrySpin_);
    mainLayout->addSpacing(12);

    // ---- 用户PIN码（必填）----
    mainLayout->addWidget(createRequiredLabel("用户PIN码"));
    userPinEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(userPinEdit_);
    userPinEdit_->setPlaceholderText("请输入用户PIN码");
    userPinEdit_->setEchoMode(QLineEdit::Password);
    connect(userPinEdit_, &ElaLineEdit::textChanged, this, &CreateAppDialog::validate);
    mainLayout->addWidget(userPinEdit_);
    mainLayout->addSpacing(12);

    // ---- 用户重试次数（必填）----
    mainLayout->addWidget(createRequiredLabel("用户重试次数"));
    userRetrySpin_ = new ElaSpinBox(this);
    userRetrySpin_->setRange(1, 10);
    userRetrySpin_->setValue(3);
    userRetrySpin_->setFixedHeight(36);
    mainLayout->addWidget(userRetrySpin_);
    mainLayout->addSpacing(16);

    // ---- 分隔线 ----
    mainLayout->addWidget(UiHelper::createDivider(this));

    // ---- 按钮 ----
    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    cancelButton_ = new ElaPushButton("取消", this);
    UiHelper::styleDefaultButton(cancelButton_);
    connect(cancelButton_, &ElaPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelButton_);
    okButton_ = new ElaPushButton("确定", this);
    UiHelper::stylePrimaryButton(okButton_);
    connect(okButton_, &ElaPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okButton_);
    mainLayout->addLayout(btnLayout);

    // 初始状态：禁用确定按钮
    okButton_->setEnabled(false);
}

QLabel* CreateAppDialog::createRequiredLabel(const QString& text) {
    // 使用 QLabel + 富文本，显式指定黑色文字颜色，避免被主题覆盖
    auto* label = new QLabel(this);
    label->setTextFormat(Qt::RichText);
    label->setText(
        QString("<span style='color:#ff4d4f; font-size:14px;'>* </span>"
                "<span style='color:#000000; font-size:14px;'>%1</span>")
            .arg(text));
    return label;
}

void CreateAppDialog::validate() {
    bool valid = !nameEdit_->text().trimmed().isEmpty()
              && !adminPinEdit_->text().isEmpty()
              && !userPinEdit_->text().isEmpty();
    okButton_->setEnabled(valid);
}

QString CreateAppDialog::appName() const {
    return nameEdit_->text().trimmed();
}

QString CreateAppDialog::adminPin() const {
    return adminPinEdit_->text();
}

int CreateAppDialog::adminRetry() const {
    return adminRetrySpin_->value();
}

QString CreateAppDialog::userPin() const {
    return userPinEdit_->text();
}

int CreateAppDialog::userRetry() const {
    return userRetrySpin_->value();
}

QVariantMap CreateAppDialog::toArgs() const {
    QVariantMap args;
    args["adminPin"] = adminPin();
    args["userPin"] = userPin();
    args["adminRetry"] = adminRetry();
    args["userRetry"] = userRetry();
    return args;
}

}  // namespace wekey
