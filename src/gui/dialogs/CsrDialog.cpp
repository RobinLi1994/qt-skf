/**
 * @file CsrDialog.cpp
 * @brief CSR 生成对话框实现
 *
 * 使用 ElaWidgetTools 组件，QLabel+RichText 标签。
 * 点击确定后在对话框内调用 generateCsr 并展示 PEM 结果，支持复制。
 */

#include "CsrDialog.h"

#include <QApplication>
#include <QButtonGroup>
#include <QClipboard>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaRadioButton.h>
#include <ElaToggleSwitch.h>

#include "config/Config.h"
#include "core/crypto/CertService.h"
#include "gui/UiHelper.h"

namespace wekey {

CsrDialog::CsrDialog(const QString& devName, const QString& appName,
                       const QString& containerName, QWidget* parent)
    : QDialog(parent)
    , devName_(devName)
    , appName_(appName)
    , containerName_(containerName) {
    setupUi();
}

QString CsrDialog::keyType() const {
    return sm2Radio_->isChecked() ? "SM2" : "RSA";
}

QString CsrDialog::commonName() const { return cnEdit_->text().trimmed(); }
QString CsrDialog::organization() const { return orgEdit_->text().trimmed(); }
QString CsrDialog::unit() const { return ouEdit_->text().trimmed(); }
bool CsrDialog::regenerateKey() const { return renewSwitch_->getIsToggled(); }
bool CsrDialog::isGenerated() const { return generated_; }

QVariantMap CsrDialog::toArgs() const {
    QVariantMap args;
    args["renewKey"] = regenerateKey();
    args["cname"] = commonName();
    args["org"] = organization();
    args["unit"] = unit();
    args["keyType"] = keyType();
    return args;
}

void CsrDialog::validate() {
    // 已生成后不再允许修改
    if (generated_) {
        return;
    }
    bool valid = !cnEdit_->text().trimmed().isEmpty()
              && !orgEdit_->text().trimmed().isEmpty()
              && !ouEdit_->text().trimmed().isEmpty();
    okButton_->setEnabled(valid);
}

void CsrDialog::onGenerate() {
    qDebug() << "[CsrDialog] 开始生成 CSR, container:" << containerName_;

    // 禁用确定按钮，防止重复点击
    okButton_->setEnabled(false);
    okButton_->setText("生成中...");

    // 调用 CertService::generateCsr
    auto result = CertService::instance().generateCsr(devName_, appName_, containerName_, toArgs());
    if (!result.isOk()) {
        qWarning() << "[CsrDialog] 生成 CSR 失败:" << result.error().message();
        okButton_->setText("确定");
        okButton_->setEnabled(true);

        // 在结果区域显示错误信息
        resultWidget_->setVisible(true);
        resultEdit_->setPlainText(QString("生成失败: %1").arg(result.error().friendlyMessage()));
        resultEdit_->setStyleSheet("QTextEdit { color: #ff4d4f; font-family: monospace; font-size: 12px; "
                                   "border: 1px solid #ff4d4f; border-radius: 4px; padding: 8px; "
                                   "background-color: #fff2f0; }");
        copyButton_->setVisible(false);
        return;
    }

    // 将 DER 编码的 CSR 转换为 PEM 格式
    QString pemBody = QString::fromLatin1(result.value().toBase64());
    QString formattedPem;
    for (int i = 0; i < pemBody.size(); i += 64) {
        formattedPem += pemBody.mid(i, 64) + "\n";
    }
    QString csrPem = "-----BEGIN CERTIFICATE REQUEST-----\n" + formattedPem + "-----END CERTIFICATE REQUEST-----";

    qDebug() << "[CsrDialog] CSR 生成成功";

    // 标记已生成
    generated_ = true;

    // 禁用输入控件
    renewSwitch_->setEnabled(false);
    sm2Radio_->setEnabled(false);
    rsaRadio_->setEnabled(false);
    cnEdit_->setEnabled(false);
    orgEdit_->setEnabled(false);
    ouEdit_->setEnabled(false);

    // 显示结果区域
    resultWidget_->setVisible(true);
    resultEdit_->setPlainText(csrPem);
    resultEdit_->setStyleSheet("QTextEdit { font-family: monospace; font-size: 12px; "
                               "border: 1px solid #d9d9d9; border-radius: 4px; padding: 8px; "
                               "background-color: #fafafa; }");
    copyButton_->setVisible(true);

    // 确定按钮变为关闭按钮
    okButton_->setText("关闭");
    okButton_->setEnabled(true);
    disconnect(okButton_, &ElaPushButton::clicked, this, &CsrDialog::onGenerate);
    connect(okButton_, &ElaPushButton::clicked, this, &QDialog::accept);
}

void CsrDialog::onCopy() {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(resultEdit_->toPlainText());
    qDebug() << "[CsrDialog] CSR PEM 已复制到剪贴板";

    // 短暂提示已复制
    copyButton_->setText("已复制 ✓");
    copyButton_->setEnabled(false);
    QTimer::singleShot(1500, this, [this]() {
        copyButton_->setText("复制");
        copyButton_->setEnabled(true);
    });
}

void CsrDialog::setupUi() {
    // 标题包含容器名称
    if (containerName_.isEmpty()) {
        setWindowTitle("生成CSR");
    } else {
        setWindowTitle(QString("生成CSR - %1").arg(containerName_));
    }
    resize(480, 0);
    UiHelper::styleDialog(this);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(4);

    // ---- 更新密钥（开关）----
    mainLayout->addWidget(createRequiredLabel("更新密钥"));
    auto* switchLayout = new QHBoxLayout;
    switchLayout->setContentsMargins(0, 0, 0, 0);
    renewSwitch_ = new ElaToggleSwitch(this);
    renewSwitch_->setIsToggled(false);
    switchLayout->addWidget(renewSwitch_);
    switchLayout->addStretch();
    mainLayout->addLayout(switchLayout);
    mainLayout->addSpacing(12);

    // ---- 密钥类型（单选）----
    mainLayout->addWidget(createRequiredLabel("密钥类型"));
    auto* keyTypeLayout = new QHBoxLayout;
    keyTypeLayout->setContentsMargins(0, 0, 0, 0);
    keyTypeLayout->setSpacing(16);
    sm2Radio_ = new ElaRadioButton("SM2", this);
    rsaRadio_ = new ElaRadioButton("RSA", this);
    auto* keyGroup = new QButtonGroup(this);
    keyGroup->addButton(sm2Radio_);
    keyGroup->addButton(rsaRadio_);
    sm2Radio_->setChecked(true);
    keyTypeLayout->addWidget(sm2Radio_);
    keyTypeLayout->addWidget(rsaRadio_);
    keyTypeLayout->addStretch();
    mainLayout->addLayout(keyTypeLayout);
    mainLayout->addSpacing(12);

    // ---- 证书通用名称（必填）----
    auto& cfg = Config::instance();

    mainLayout->addWidget(createRequiredLabel("证书通用名称"));
    cnEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(cnEdit_);
    cnEdit_->setPlaceholderText("请输入通用名称");
    cnEdit_->setText(cfg.defaultCommonName());
    connect(cnEdit_, &ElaLineEdit::textChanged, this, &CsrDialog::validate);
    mainLayout->addWidget(cnEdit_);
    mainLayout->addSpacing(12);

    // ---- 证书组织名称（必填）----
    mainLayout->addWidget(createRequiredLabel("证书组织名称"));
    orgEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(orgEdit_);
    orgEdit_->setPlaceholderText("请输入组织名称");
    orgEdit_->setText(cfg.defaultOrganization());
    connect(orgEdit_, &ElaLineEdit::textChanged, this, &CsrDialog::validate);
    mainLayout->addWidget(orgEdit_);
    mainLayout->addSpacing(12);

    // ---- 证书部门名称（必填）----
    mainLayout->addWidget(createRequiredLabel("证书部门名称"));
    ouEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(ouEdit_);
    ouEdit_->setPlaceholderText("请输入部门名称");
    ouEdit_->setText(cfg.defaultUnit());
    connect(ouEdit_, &ElaLineEdit::textChanged, this, &CsrDialog::validate);
    mainLayout->addWidget(ouEdit_);
    mainLayout->addSpacing(12);

    // ---- 生成结果区域（初始隐藏）----
    resultWidget_ = new QWidget(this);
    auto* resultLayout = new QVBoxLayout(resultWidget_);
    resultLayout->setContentsMargins(0, 0, 0, 0);
    resultLayout->setSpacing(8);

    auto* resultLabel = new QLabel(this);
    resultLabel->setTextFormat(Qt::RichText);
    resultLabel->setText("<span style='color:#000000; font-size:14px; font-weight:bold;'>生成结果</span>");
    resultLayout->addWidget(resultLabel);

    resultEdit_ = new QTextEdit(this);
    resultEdit_->setReadOnly(true);
    resultEdit_->setMinimumHeight(150);
    resultEdit_->setMaximumHeight(200);
    resultLayout->addWidget(resultEdit_);

    // 复制按钮
    auto* copyLayout = new QHBoxLayout;
    copyLayout->addStretch();
    copyButton_ = new ElaPushButton("复制", this);
    UiHelper::styleDefaultButton(copyButton_);
    copyButton_->setFixedWidth(80);
    connect(copyButton_, &ElaPushButton::clicked, this, &CsrDialog::onCopy);
    copyLayout->addWidget(copyButton_);
    resultLayout->addLayout(copyLayout);

    resultWidget_->setVisible(false);
    mainLayout->addWidget(resultWidget_);
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
    connect(okButton_, &ElaPushButton::clicked, this, &CsrDialog::onGenerate);
    btnLayout->addWidget(okButton_);
    mainLayout->addLayout(btnLayout);

    // 初始验证
    validate();
}

QLabel* CsrDialog::createRequiredLabel(const QString& text) {
    auto* label = new QLabel(this);
    label->setTextFormat(Qt::RichText);
    label->setText(
        QString("<span style='color:#ff4d4f; font-size:14px;'>* </span>"
                "<span style='color:#000000; font-size:14px;'>%1</span>")
            .arg(text));
    return label;
}

}  // namespace wekey
