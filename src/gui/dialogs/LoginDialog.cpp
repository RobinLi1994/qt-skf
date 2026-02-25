/**
 * @file LoginDialog.cpp
 * @brief 登录对话框实现
 *
 * 使用 ElaWidgetTools 组件，QLabel+RichText 标签。
 * 登录失败时通过 showError() 显示红色错误提示。
 */

#include "LoginDialog.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaRadioButton.h>

#include "gui/UiHelper.h"

namespace wekey {

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setupUi();
}

QString LoginDialog::pin() const { return pinEdit_->text(); }

QString LoginDialog::role() const {
    return userRadio_->isChecked() ? "user" : "admin";
}

void LoginDialog::validate() {
    okButton_->setEnabled(!pinEdit_->text().isEmpty());
}

void LoginDialog::setupUi() {
    setWindowTitle("登录应用");
    resize(420, 0);
    UiHelper::styleDialog(this);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(4);

    // ---- PIN码（必填）----
    auto* pinLabel = new QLabel(this);
    pinLabel->setTextFormat(Qt::RichText);
    pinLabel->setText(
        "<span style='color:#ff4d4f; font-size:14px;'>* </span>"
        "<span style='color:#000000; font-size:14px;'>PIN码</span>");
    mainLayout->addWidget(pinLabel);

    pinEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(pinEdit_);
    pinEdit_->setEchoMode(QLineEdit::Password);
    pinEdit_->setPlaceholderText("请输入应用PIN码");
    connect(pinEdit_, &ElaLineEdit::textChanged, this, &LoginDialog::validate);
    mainLayout->addWidget(pinEdit_);
    mainLayout->addSpacing(12);

    // ---- 角色选择（必填）----
    auto* roleLabel = new QLabel(this);
    roleLabel->setTextFormat(Qt::RichText);
    roleLabel->setText(
        "<span style='color:#ff4d4f; font-size:14px;'>* </span>"
        "<span style='color:#000000; font-size:14px;'>角色</span>");
    mainLayout->addWidget(roleLabel);

    auto* roleLayout = new QHBoxLayout;
    roleLayout->setContentsMargins(0, 0, 0, 0);
    roleLayout->setSpacing(16);
    userRadio_ = new ElaRadioButton("用户", this);
    adminRadio_ = new ElaRadioButton("管理员", this);
    auto* roleGroup = new QButtonGroup(this);
    roleGroup->addButton(userRadio_);
    roleGroup->addButton(adminRadio_);
    userRadio_->setChecked(true);
    roleLayout->addWidget(userRadio_);
    roleLayout->addWidget(adminRadio_);
    roleLayout->addStretch();
    mainLayout->addLayout(roleLayout);
    mainLayout->addSpacing(16);

    // ---- 分隔线 ----
    mainLayout->addWidget(UiHelper::createDivider(this));

    // ---- 按钮 ----
    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    auto* cancelBtn = new ElaPushButton("取消", this);
    UiHelper::styleDefaultButton(cancelBtn);
    connect(cancelBtn, &ElaPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    okButton_ = new ElaPushButton("确定", this);
    UiHelper::stylePrimaryButton(okButton_);
    okButton_->setEnabled(false);
    connect(okButton_, &ElaPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okButton_);
    mainLayout->addLayout(btnLayout);
}

}  // namespace wekey
