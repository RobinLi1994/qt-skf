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

    // ---- 管理员PIN码（必填，≥6位）----
    mainLayout->addWidget(createRequiredLabel("管理员PIN码"));
    adminPinEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(adminPinEdit_);
    adminPinEdit_->setPlaceholderText("请输入管理员PIN码（至少6位）");
    adminPinEdit_->setEchoMode(QLineEdit::Password);
    connect(adminPinEdit_, &ElaLineEdit::textChanged, this, &CreateAppDialog::validate);
    mainLayout->addWidget(adminPinEdit_);
    adminPinHint_ = createHintLabel("PIN码至少需要6位");
    mainLayout->addWidget(adminPinHint_);
    mainLayout->addSpacing(8);

    // ---- 管理员PIN码确认（必填）----
    mainLayout->addWidget(createRequiredLabel("确认管理员PIN码"));
    adminPinConfirmEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(adminPinConfirmEdit_);
    adminPinConfirmEdit_->setPlaceholderText("请再次输入管理员PIN码");
    adminPinConfirmEdit_->setEchoMode(QLineEdit::Password);
    connect(adminPinConfirmEdit_, &ElaLineEdit::textChanged, this, &CreateAppDialog::validate);
    mainLayout->addWidget(adminPinConfirmEdit_);
    adminPinConfirmHint_ = createHintLabel("两次输入的PIN码不一致");
    mainLayout->addWidget(adminPinConfirmHint_);
    mainLayout->addSpacing(12);

    // ---- 管理员重试次数（必填）----
    mainLayout->addWidget(createRequiredLabel("管理员重试次数"));
    adminRetrySpin_ = new ElaSpinBox(this);
    adminRetrySpin_->setRange(1, 99);
    adminRetrySpin_->setValue(15);
    adminRetrySpin_->setFixedHeight(36);
    mainLayout->addWidget(adminRetrySpin_);
    mainLayout->addSpacing(12);

    // ---- 用户PIN码（必填，≥6位）----
    mainLayout->addWidget(createRequiredLabel("用户PIN码"));
    userPinEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(userPinEdit_);
    userPinEdit_->setPlaceholderText("请输入用户PIN码（至少6位）");
    userPinEdit_->setEchoMode(QLineEdit::Password);
    connect(userPinEdit_, &ElaLineEdit::textChanged, this, &CreateAppDialog::validate);
    mainLayout->addWidget(userPinEdit_);
    userPinHint_ = createHintLabel("PIN码至少需要6位");
    mainLayout->addWidget(userPinHint_);
    mainLayout->addSpacing(8);

    // ---- 用户PIN码确认（必填）----
    mainLayout->addWidget(createRequiredLabel("确认用户PIN码"));
    userPinConfirmEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(userPinConfirmEdit_);
    userPinConfirmEdit_->setPlaceholderText("请再次输入用户PIN码");
    userPinConfirmEdit_->setEchoMode(QLineEdit::Password);
    connect(userPinConfirmEdit_, &ElaLineEdit::textChanged, this, &CreateAppDialog::validate);
    mainLayout->addWidget(userPinConfirmEdit_);
    userPinConfirmHint_ = createHintLabel("两次输入的PIN码不一致");
    mainLayout->addWidget(userPinConfirmHint_);
    mainLayout->addSpacing(12);

    // ---- 用户重试次数（必填）----
    mainLayout->addWidget(createRequiredLabel("用户重试次数"));
    userRetrySpin_ = new ElaSpinBox(this);
    userRetrySpin_->setRange(1, 99);
    userRetrySpin_->setValue(15);
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

QLabel* CreateAppDialog::createHintLabel(const QString& text) {
    auto* label = new QLabel(text, this);
    label->setStyleSheet("color: #ff4d4f; font-size: 12px;");
    label->setVisible(false);
    return label;
}

void CreateAppDialog::validate() {
    const QString adminPin        = adminPinEdit_->text();
    const QString adminPinConfirm = adminPinConfirmEdit_->text();
    const QString userPin         = userPinEdit_->text();
    const QString userPinConfirm  = userPinConfirmEdit_->text();

    // 管理员PIN长度提示：已有输入但不足6位时显示
    const bool adminPinTooShort = !adminPin.isEmpty() && adminPin.length() < 6;
    adminPinHint_->setVisible(adminPinTooShort);

    // 管理员PIN确认不一致提示：两框均非空且不匹配时显示
    const bool adminPinMismatch = !adminPinConfirm.isEmpty() && adminPin != adminPinConfirm;
    adminPinConfirmHint_->setVisible(adminPinMismatch);

    // 用户PIN长度提示
    const bool userPinTooShort = !userPin.isEmpty() && userPin.length() < 6;
    userPinHint_->setVisible(userPinTooShort);

    // 用户PIN确认不一致提示
    const bool userPinMismatch = !userPinConfirm.isEmpty() && userPin != userPinConfirm;
    userPinConfirmHint_->setVisible(userPinMismatch);

    bool valid = !nameEdit_->text().trimmed().isEmpty()
              && adminPin.length() >= 6
              && adminPin == adminPinConfirm
              && userPin.length() >= 6
              && userPin == userPinConfirm;
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
