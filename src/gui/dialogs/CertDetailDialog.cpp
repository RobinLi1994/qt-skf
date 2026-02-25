/**
 * @file CertDetailDialog.cpp
 * @brief 证书详情对话框实现
 *
 * 展示容器中签名证书和加密证书的详细信息。
 * 使用折叠区域分别显示签名证书和加密证书。
 */

#include "CertDetailDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>

#include <ElaPushButton.h>

#include "core/crypto/CertService.h"
#include "gui/UiHelper.h"

namespace wekey {

CertDetailDialog::CertDetailDialog(const QString& devName, const QString& appName,
                                   const QString& containerName, QWidget* parent)
    : QDialog(parent)
    , devName_(devName)
    , appName_(appName)
    , containerName_(containerName) {
    setupUi(containerName);
}

void CertDetailDialog::setupUi(const QString& containerName) {
    setWindowTitle(QString("证书信息 - %1").arg(containerName));
    resize(680, 600);
    UiHelper::styleDialog(this);

    // 滚动区域包裹内容
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: #ffffff; border: none; }");

    auto* contentWidget = new QWidget;
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(24, 24, 24, 16);
    contentLayout->setSpacing(16);

    // 尝试获取签名证书信息
    auto signResult = CertService::instance().getCertInfo(devName_, appName_, containerName_, true);
    if (signResult.isOk()) {
        qDebug() << "[CertDetailDialog] 签名证书获取成功, SN:" << signResult.value().serialNumber;
        addCertSection(contentLayout, signResult.value(), true);
    } else {
        qDebug() << "[CertDetailDialog] 签名证书获取失败:" << signResult.error().message();
    }

    // 尝试获取加密证书信息
    auto encResult = CertService::instance().getCertInfo(devName_, appName_, containerName_, false);
    if (encResult.isOk()) {
        qDebug() << "[CertDetailDialog] 加密证书获取成功, SN:" << encResult.value().serialNumber;
        addCertSection(contentLayout, encResult.value(), false);
    } else {
        qDebug() << "[CertDetailDialog] 加密证书获取失败:" << encResult.error().message();
    }

    // 如果两个证书都没有
    if (signResult.isErr() && encResult.isErr()) {
        auto* emptyLabel = new QLabel("该容器中没有证书", contentWidget);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("QLabel { color: #999999; font-size: 14px; padding: 40px; }");
        contentLayout->addWidget(emptyLabel);
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    outerLayout->addWidget(scrollArea);

    // 底部按钮区域
    auto* bottomWidget = new QWidget(this);
    bottomWidget->setStyleSheet("QWidget { background: #ffffff; border-top: 1px solid #f0f0f0; }");
    auto* btnLayout = new QHBoxLayout(bottomWidget);
    btnLayout->setContentsMargins(24, 12, 24, 12);
    btnLayout->addStretch();
    auto* closeBtn = new ElaPushButton("关 闭", this);
    UiHelper::styleDefaultButton(closeBtn);
    connect(closeBtn, &ElaPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    outerLayout->addWidget(bottomWidget);
}

void CertDetailDialog::addCertSection(QVBoxLayout* layout, const CertInfo& info, bool isSignCert) {
    // 折叠区域容器
    auto* sectionWidget = new QWidget;
    auto* sectionLayout = new QVBoxLayout(sectionWidget);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(0);

    // --- 折叠标题栏 ---
    auto* headerWidget = new QWidget;
    headerWidget->setObjectName("certHeader");
    headerWidget->setStyleSheet(
        "#certHeader { background: #fafafa; border: 1px solid #f0f0f0; "
        "border-radius: 8px 8px 0px 0px; }");
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 10, 16, 10);

    // 展开/折叠按钮
    auto* toggleBtn = new QToolButton;
    toggleBtn->setArrowType(Qt::DownArrow);
    toggleBtn->setAutoRaise(true);
    toggleBtn->setFixedSize(20, 20);
    headerLayout->addWidget(toggleBtn);

    // 证书类型 Tag
    headerLayout->addWidget(createCertTypeTag(isSignCert));
    headerLayout->addSpacing(8);

    // 证书 CN + SN 摘要
    QString summary = info.commonName;
    if (!info.serialNumber.isEmpty()) {
        summary += QString("  (SN: %1)").arg(info.serialNumber);
    }
    auto* summaryLabel = new QLabel(summary);
    summaryLabel->setStyleSheet("QLabel { color: rgba(0,0,0,0.65); font-size: 13px; background: transparent; border: none; }");
    headerLayout->addWidget(summaryLabel, 1);

    sectionLayout->addWidget(headerWidget);

    // --- 详情内容区域 ---
    auto* detailWidget = new QWidget;
    detailWidget->setObjectName("certDetail");
    detailWidget->setStyleSheet(
        "#certDetail { background: #ffffff; border: 1px solid #f0f0f0; "
        "border-top: none; border-radius: 0px 0px 8px 8px; }");
    auto* detailLayout = new QVBoxLayout(detailWidget);
    detailLayout->setContentsMargins(16, 12, 16, 12);
    detailLayout->setSpacing(0);

    // 信息行
    addInfoRow(detailLayout, "序列号", info.serialNumber);
    addInfoRow(detailLayout, "主题", info.subjectDn);
    addInfoRow(detailLayout, "通用名称", info.commonName);
    addInfoRow(detailLayout, "颁发者", info.issuerDn);

    // 有效期
    QString validity;
    if (info.notBefore.isValid() && info.notAfter.isValid()) {
        validity = info.notBefore.toString(Qt::ISODate) + "  至  " + info.notAfter.toString(Qt::ISODate);
    }
    addInfoRow(detailLayout, "有效期", validity);

    // 证书类型 Tag 行
    addInfoRow(detailLayout, "证书类型", createCertTypeTag(isSignCert));

    // 公钥哈希
    addInfoRow(detailLayout, "公钥哈希", info.pubKeyHash.toUpper());

    // 证书内容（PEM）
    if (!info.cert.isEmpty()) {
        // 分隔线
        auto* divider = new QFrame;
        divider->setFrameShape(QFrame::HLine);
        divider->setStyleSheet("QFrame { color: #f0f0f0; border: none; background: #f0f0f0; }");
        divider->setFixedHeight(1);
        detailLayout->addWidget(divider);

        auto* pemRow = new QHBoxLayout;
        pemRow->setContentsMargins(0, 8, 0, 8);
        pemRow->setSpacing(0);

        auto* pemLabel = new QLabel("证书内容");
        pemLabel->setFixedWidth(80);
        pemLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        pemLabel->setStyleSheet("QLabel { color: rgba(0,0,0,0.65); font-size: 13px; padding-top: 6px; background: transparent; border: none; }");
        pemRow->addWidget(pemLabel);

        auto* pemEdit = new QTextEdit;
        pemEdit->setPlainText(info.cert);
        pemEdit->setReadOnly(true);
        pemEdit->setFixedHeight(160);
        pemEdit->setStyleSheet(
            "QTextEdit {"
            "  border: 1px solid #d9d9d9;"
            "  border-radius: 6px;"
            "  padding: 8px;"
            "  font-family: 'Courier New', monospace;"
            "  font-size: 12px;"
            "  color: rgba(0,0,0,0.85);"
            "  background: #fafafa;"
            "}");
        pemRow->addWidget(pemEdit, 1);

        detailLayout->addLayout(pemRow);
    }

    sectionLayout->addWidget(detailWidget);

    // 折叠切换逻辑
    connect(toggleBtn, &QToolButton::clicked, [toggleBtn, detailWidget]() {
        bool visible = detailWidget->isVisible();
        detailWidget->setVisible(!visible);
        toggleBtn->setArrowType(visible ? Qt::RightArrow : Qt::DownArrow);
    });

    layout->addWidget(sectionWidget);
}

void CertDetailDialog::addInfoRow(QVBoxLayout* layout, const QString& label, const QString& value) {
    if (value.isEmpty()) return;

    // 分隔线
    auto* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("QFrame { color: #f0f0f0; border: none; background: #f0f0f0; }");
    divider->setFixedHeight(1);
    layout->addWidget(divider);

    auto* rowLayout = new QHBoxLayout;
    rowLayout->setContentsMargins(0, 8, 0, 8);
    rowLayout->setSpacing(0);

    auto* keyLabel = new QLabel(label);
    keyLabel->setFixedWidth(80);
    keyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    keyLabel->setStyleSheet("QLabel { color: rgba(0,0,0,0.65); font-size: 13px; background: transparent; border: none; }");
    rowLayout->addWidget(keyLabel);

    auto* valLabel = new QLabel(value);
    valLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    valLabel->setWordWrap(true);
    valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valLabel->setStyleSheet("QLabel { color: rgba(0,0,0,0.85); font-size: 13px; background: transparent; border: none; }");
    rowLayout->addWidget(valLabel, 1);

    layout->addLayout(rowLayout);
}

void CertDetailDialog::addInfoRow(QVBoxLayout* layout, const QString& label, QWidget* widget) {
    // 分隔线
    auto* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("QFrame { color: #f0f0f0; border: none; background: #f0f0f0; }");
    divider->setFixedHeight(1);
    layout->addWidget(divider);

    auto* rowLayout = new QHBoxLayout;
    rowLayout->setContentsMargins(0, 8, 0, 8);
    rowLayout->setSpacing(0);

    auto* keyLabel = new QLabel(label);
    keyLabel->setFixedWidth(80);
    keyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    keyLabel->setStyleSheet("QLabel { color: rgba(0,0,0,0.65); font-size: 13px; background: transparent; border: none; }");
    rowLayout->addWidget(keyLabel);

    rowLayout->addWidget(widget, 1);

    layout->addLayout(rowLayout);
}

QWidget* CertDetailDialog::createCertTypeTag(bool isSignCert) {
    if (isSignCert) {
        // 蓝色边框 Tag
        return UiHelper::createInfoTag("签名证书");
    } else {
        // 绿色边框 Tag
        return UiHelper::createSuccessTag("加密证书");
    }
}

}  // namespace wekey
