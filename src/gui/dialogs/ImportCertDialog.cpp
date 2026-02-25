/**
 * @file ImportCertDialog.cpp
 * @brief 导入证书和密钥对话框实现
 *
 * 支持导入签名证书、加密证书、加密私钥。
 * 使用 ElaWidgetTools 组件统一风格。
 */

#include "ImportCertDialog.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaToggleSwitch.h>

#include "gui/UiHelper.h"

namespace wekey {

ImportCertDialog::ImportCertDialog(const QString& containerName, QWidget* parent)
    : QDialog(parent) {
    setupUi(containerName);
}

bool ImportCertDialog::isNonGM() const { return nonGMSwitch_->getIsToggled(); }
QByteArray ImportCertDialog::sigCertData() const { return sigCertData_; }
QByteArray ImportCertDialog::encCertData() const { return encCertData_; }
QByteArray ImportCertDialog::encPrivateData() const { return encPrivateData_; }

void ImportCertDialog::validate() {
    // 至少选择了一个文件时启用确定按钮
    bool hasAny = !sigCertData_.isEmpty() || !encCertData_.isEmpty() || !encPrivateData_.isEmpty();
    okButton_->setEnabled(hasAny);
}

void ImportCertDialog::onBrowseSigCert() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择签名证书文件", QString(),
        "证书文件 (*.pem *.cer *.crt *.der);;所有文件 (*)");
    if (filePath.isEmpty()) return;

    sigCertData_ = readCertFile(filePath);
    if (!sigCertData_.isEmpty()) {
        sigCertPath_->setText(QFileInfo(filePath).fileName());
        qDebug() << "[ImportCertDialog] 签名证书已加载, size:" << sigCertData_.size();
    } else {
        sigCertPath_->setText("");
        qWarning() << "[ImportCertDialog] 签名证书文件读取失败:" << filePath;
    }
    validate();
}

void ImportCertDialog::onBrowseEncCert() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择加密证书文件", QString(),
        "证书文件 (*.pem *.cer *.crt *.der);;所有文件 (*)");
    if (filePath.isEmpty()) return;

    encCertData_ = readCertFile(filePath);
    if (!encCertData_.isEmpty()) {
        encCertPath_->setText(QFileInfo(filePath).fileName());
        qDebug() << "[ImportCertDialog] 加密证书已加载, size:" << encCertData_.size();
    } else {
        encCertPath_->setText("");
        qWarning() << "[ImportCertDialog] 加密证书文件读取失败:" << filePath;
    }
    validate();
}

void ImportCertDialog::onBrowseEncPrivate() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择加密私钥文件", QString(),
        "密钥文件 (*.key *.pem *.bin);;所有文件 (*)");
    if (filePath.isEmpty()) return;

    encPrivateData_ = readKeyFile(filePath);
    if (!encPrivateData_.isEmpty()) {
        encPrivatePath_->setText(QFileInfo(filePath).fileName());
        qDebug() << "[ImportCertDialog] 加密私钥已加载, size:" << encPrivateData_.size();
    } else {
        encPrivatePath_->setText("");
        qWarning() << "[ImportCertDialog] 加密私钥文件读取失败:" << filePath;
    }
    validate();
}

void ImportCertDialog::setupUi(const QString& containerName) {
    // 标题包含容器名称
    if (containerName.isEmpty()) {
        setWindowTitle("导入证书和密钥");
    } else {
        setWindowTitle(QString("导入证书和密钥 - %1").arg(containerName));
    }
    resize(520, 0);
    UiHelper::styleDialog(this);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(4);

    // ---- 非国密证书（开关）----
    mainLayout->addWidget(createSectionLabel("非国密证书"));
    auto* switchLayout = new QHBoxLayout;
    switchLayout->setContentsMargins(0, 0, 0, 0);
    nonGMSwitch_ = new ElaToggleSwitch(this);
    nonGMSwitch_->setIsToggled(false);
    switchLayout->addWidget(nonGMSwitch_);
    switchLayout->addStretch();
    mainLayout->addLayout(switchLayout);
    mainLayout->addWidget(createHintLabel("勾选此项表示导入的是非国密证书"));
    mainLayout->addSpacing(12);

    // ---- 签名证书 ----
    mainLayout->addWidget(createSectionLabel("签名证书"));
    auto* sigLayout = new QHBoxLayout;
    sigLayout->setContentsMargins(0, 0, 0, 0);
    sigLayout->setSpacing(8);
    sigCertPath_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(sigCertPath_);
    sigCertPath_->setReadOnly(true);
    sigCertPath_->setPlaceholderText("未选择文件");
    sigLayout->addWidget(sigCertPath_, 1);
    auto* sigBrowseBtn = new ElaPushButton("选择文件", this);
    UiHelper::styleDefaultButton(sigBrowseBtn);
    sigBrowseBtn->setFixedWidth(90);
    connect(sigBrowseBtn, &ElaPushButton::clicked, this, &ImportCertDialog::onBrowseSigCert);
    sigLayout->addWidget(sigBrowseBtn);
    mainLayout->addLayout(sigLayout);
    mainLayout->addWidget(createHintLabel("可以是PEM格式或DER格式文件"));
    mainLayout->addSpacing(12);

    // ---- 加密证书 ----
    mainLayout->addWidget(createSectionLabel("加密证书"));
    auto* encLayout = new QHBoxLayout;
    encLayout->setContentsMargins(0, 0, 0, 0);
    encLayout->setSpacing(8);
    encCertPath_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(encCertPath_);
    encCertPath_->setReadOnly(true);
    encCertPath_->setPlaceholderText("未选择文件");
    encLayout->addWidget(encCertPath_, 1);
    auto* encBrowseBtn = new ElaPushButton("选择文件", this);
    UiHelper::styleDefaultButton(encBrowseBtn);
    encBrowseBtn->setFixedWidth(90);
    connect(encBrowseBtn, &ElaPushButton::clicked, this, &ImportCertDialog::onBrowseEncCert);
    encLayout->addWidget(encBrowseBtn);
    mainLayout->addLayout(encLayout);
    mainLayout->addWidget(createHintLabel("可以是PEM格式或DER格式文件"));
    mainLayout->addSpacing(12);

    // ---- 加密私钥 ----
    mainLayout->addWidget(createSectionLabel("加密私钥"));
    auto* keyLayout = new QHBoxLayout;
    keyLayout->setContentsMargins(0, 0, 0, 0);
    keyLayout->setSpacing(8);
    encPrivatePath_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(encPrivatePath_);
    encPrivatePath_->setReadOnly(true);
    encPrivatePath_->setPlaceholderText("未选择文件");
    keyLayout->addWidget(encPrivatePath_, 1);
    auto* keyBrowseBtn = new ElaPushButton("选择文件", this);
    UiHelper::styleDefaultButton(keyBrowseBtn);
    keyBrowseBtn->setFixedWidth(90);
    connect(keyBrowseBtn, &ElaPushButton::clicked, this, &ImportCertDialog::onBrowseEncPrivate);
    keyLayout->addWidget(keyBrowseBtn);
    mainLayout->addLayout(keyLayout);
    mainLayout->addWidget(createHintLabel("base64编码格式文件"));
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
    connect(okButton_, &ElaPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okButton_);
    mainLayout->addLayout(btnLayout);

    // 初始验证（无文件时禁用确定按钮）
    validate();
}

QLabel* ImportCertDialog::createSectionLabel(const QString& text) {
    auto* label = new QLabel(this);
    label->setTextFormat(Qt::RichText);
    label->setText(
        QString("<span style='font-size:14px; font-weight:bold; color:#000000;'>%1</span>")
            .arg(text));
    return label;
}

QLabel* ImportCertDialog::createHintLabel(const QString& text) {
    auto* label = new QLabel(text, this);
    label->setStyleSheet("QLabel { color: #999999; font-size: 12px; }");
    return label;
}

QByteArray ImportCertDialog::readCertFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QByteArray raw = file.readAll();
    file.close();

    if (raw.isEmpty()) return {};

    // 检查是否为 PEM 格式，如果是则解码为 DER
    QString text = QString::fromUtf8(raw).trimmed();
    if (text.startsWith("-----BEGIN")) {
        // 移除 PEM 头尾和空白
        QStringList lines = text.split('\n');
        QString base64;
        for (const QString& line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.startsWith("-----")) continue;
            base64 += trimmed;
        }
        QByteArray decoded = QByteArray::fromBase64(base64.toLatin1());
        qDebug() << "[ImportCertDialog] PEM 证书解码, base64 size:" << base64.size()
                 << "DER size:" << decoded.size();
        return decoded;
    }

    // DER 格式，直接返回
    qDebug() << "[ImportCertDialog] DER 证书, size:" << raw.size();
    return raw;
}

QByteArray ImportCertDialog::readKeyFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QByteArray raw = file.readAll();
    file.close();

    if (raw.isEmpty()) return {};

    // 尝试 base64 解码
    QString text = QString::fromUtf8(raw).trimmed();
    // 如果包含 PEM 头尾，去掉
    if (text.startsWith("-----")) {
        QStringList lines = text.split('\n');
        QString base64;
        for (const QString& line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.startsWith("-----")) continue;
            base64 += trimmed;
        }
        QByteArray decoded = QByteArray::fromBase64(base64.toLatin1());
        qDebug() << "[ImportCertDialog] PEM 私钥解码, DER size:" << decoded.size();
        return decoded;
    }

    // 纯 base64 文本
    QByteArray decoded = QByteArray::fromBase64(text.toLatin1());
    if (!decoded.isEmpty()) {
        qDebug() << "[ImportCertDialog] base64 私钥解码, size:" << decoded.size();
        return decoded;
    }

    // 二进制文件，直接返回
    qDebug() << "[ImportCertDialog] 二进制私钥, size:" << raw.size();
    return raw;
}

}  // namespace wekey
