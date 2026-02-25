/**
 * @file CsrDialog.h
 * @brief CSR 生成对话框
 *
 * 使用 ElaWidgetTools 组件，QLabel+RichText 标签。
 * 点击确定后在对话框内调用 generateCsr 并展示 PEM 结果，支持复制。
 */

#pragma once

#include <QDialog>

class ElaLineEdit;
class ElaRadioButton;
class ElaToggleSwitch;
class ElaPushButton;
class QLabel;
class QTextEdit;
class QWidget;

namespace wekey {

class CsrDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param parent 父窗口
     */
    explicit CsrDialog(const QString& devName, const QString& appName,
                       const QString& containerName, QWidget* parent = nullptr);

    /// 获取密钥类型（"SM2" 或 "RSA"）
    QString keyType() const;
    /// 获取证书通用名称
    QString commonName() const;
    /// 获取组织名称
    QString organization() const;
    /// 获取部门名称
    QString unit() const;
    /// 是否更新密钥
    bool regenerateKey() const;
    /// 构建 generateCsr 所需的参数 Map
    QVariantMap toArgs() const;
    /// CSR 生成是否成功
    bool isGenerated() const;

private slots:
    /// 输入验证：必填字段为空时禁用确定按钮
    void validate();
    /// 点击确定：调用 generateCsr 并展示结果
    void onGenerate();
    /// 复制 CSR PEM 到剪贴板
    void onCopy();

private:
    void setupUi();
    QLabel* createRequiredLabel(const QString& text);

    QString devName_;                          ///< 设备名称
    QString appName_;                          ///< 应用名称
    QString containerName_;                    ///< 容器名称
    bool generated_ = false;                   ///< CSR 是否已生成

    ElaToggleSwitch* renewSwitch_ = nullptr;   ///< 更新密钥开关
    ElaRadioButton* sm2Radio_ = nullptr;       ///< SM2 密钥类型
    ElaRadioButton* rsaRadio_ = nullptr;       ///< RSA 密钥类型
    ElaLineEdit* cnEdit_ = nullptr;            ///< 证书通用名称
    ElaLineEdit* orgEdit_ = nullptr;           ///< 组织名称
    ElaLineEdit* ouEdit_ = nullptr;            ///< 部门名称
    ElaPushButton* okButton_ = nullptr;        ///< 确定按钮

    // 生成结果区域（初始隐藏）
    QWidget* resultWidget_ = nullptr;          ///< 结果区域容器
    QTextEdit* resultEdit_ = nullptr;          ///< CSR PEM 文本
    ElaPushButton* copyButton_ = nullptr;      ///< 复制按钮
};

}  // namespace wekey
