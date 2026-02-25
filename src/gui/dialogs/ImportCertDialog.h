/**
 * @file ImportCertDialog.h
 * @brief 导入证书和密钥对话框
 *
 * 支持导入签名证书、加密证书、加密私钥。
 * 使用 ElaWidgetTools 组件统一风格。
 */

#pragma once

#include <QDialog>

class ElaLineEdit;
class ElaToggleSwitch;
class ElaPushButton;
class QLabel;

namespace wekey {

class ImportCertDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param containerName 容器名称，显示在标题中
     * @param parent 父窗口
     */
    explicit ImportCertDialog(const QString& containerName = {}, QWidget* parent = nullptr);

    /// 是否为非国密证书
    bool isNonGM() const;
    /// 获取签名证书文件数据（DER 或 PEM）
    QByteArray sigCertData() const;
    /// 获取加密证书文件数据（DER 或 PEM）
    QByteArray encCertData() const;
    /// 获取加密私钥文件数据（base64 解码后的二进制）
    QByteArray encPrivateData() const;

private slots:
    /// 输入验证：至少选择了一个文件时启用确定按钮
    void validate();
    /// 选择签名证书文件
    void onBrowseSigCert();
    /// 选择加密证书文件
    void onBrowseEncCert();
    /// 选择加密私钥文件
    void onBrowseEncPrivate();

private:
    void setupUi(const QString& containerName);
    QLabel* createSectionLabel(const QString& text);
    QLabel* createHintLabel(const QString& text);
    /// 读取文件内容，如果是 PEM 格式则解码为 DER
    QByteArray readCertFile(const QString& filePath);
    /// 读取私钥文件内容（base64 编码文件，解码为二进制）
    QByteArray readKeyFile(const QString& filePath);

    ElaToggleSwitch* nonGMSwitch_ = nullptr;    ///< 非国密证书开关
    ElaLineEdit* sigCertPath_ = nullptr;        ///< 签名证书文件路径
    ElaLineEdit* encCertPath_ = nullptr;        ///< 加密证书文件路径
    ElaLineEdit* encPrivatePath_ = nullptr;     ///< 加密私钥文件路径
    ElaPushButton* okButton_ = nullptr;         ///< 确定按钮

    QByteArray sigCertData_;                    ///< 签名证书文件内容
    QByteArray encCertData_;                    ///< 加密证书文件内容
    QByteArray encPrivateData_;                 ///< 加密私钥文件内容
};

}  // namespace wekey
