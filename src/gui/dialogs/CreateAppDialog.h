/**
 * @file CreateAppDialog.h
 * @brief 创建应用对话框
 *
 * 参考 Go 实现逻辑，支持设备认证PIN、管理员PIN/重试次数、用户PIN/重试次数。
 * UI 风格使用 ElaWidgetTools 组件库，带红色星号必填标签。
 */

#pragma once

#include <QDialog>
#include <QLabel>

class ElaLineEdit;
class ElaPushButton;
class ElaSpinBox;

namespace wekey {

class CreateAppDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateAppDialog(QWidget* parent = nullptr);

    /// @brief 获取应用名称
    QString appName() const;
    /// @brief 获取管理员PIN码
    QString adminPin() const;
    /// @brief 获取管理员重试次数
    int adminRetry() const;
    /// @brief 获取用户PIN码
    QString userPin() const;
    /// @brief 获取用户重试次数
    int userRetry() const;

    /// @brief 将所有参数打包为 QVariantMap（不含 appName）
    QVariantMap toArgs() const;

private slots:
    void validate();

private:
    void setupUi();

    /// @brief 创建带红色星号的必填标签行
    QLabel* createRequiredLabel(const QString& text);
    /// @brief 创建红色错误提示标签（初始隐藏）
    QLabel* createHintLabel(const QString& text);

    ElaLineEdit* nameEdit_ = nullptr;             ///< 应用名称
    ElaLineEdit* adminPinEdit_ = nullptr;         ///< 管理员PIN码
    QLabel* adminPinHint_ = nullptr;              ///< 管理员PIN码长度错误提示
    ElaLineEdit* adminPinConfirmEdit_ = nullptr;  ///< 管理员PIN码确认
    QLabel* adminPinConfirmHint_ = nullptr;       ///< 管理员PIN码确认不一致提示
    ElaSpinBox* adminRetrySpin_ = nullptr;        ///< 管理员重试次数
    ElaLineEdit* userPinEdit_ = nullptr;          ///< 用户PIN码
    QLabel* userPinHint_ = nullptr;               ///< 用户PIN码长度错误提示
    ElaLineEdit* userPinConfirmEdit_ = nullptr;   ///< 用户PIN码确认
    QLabel* userPinConfirmHint_ = nullptr;        ///< 用户PIN码确认不一致提示
    ElaSpinBox* userRetrySpin_ = nullptr;         ///< 用户重试次数
    ElaPushButton* okButton_ = nullptr;      ///< 确定按钮
    ElaPushButton* cancelButton_ = nullptr;  ///< 取消按钮
};

}  // namespace wekey
