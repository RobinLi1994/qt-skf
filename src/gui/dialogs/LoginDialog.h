/**
 * @file LoginDialog.h
 * @brief 登录对话框
 *
 * 使用 ElaWidgetTools 组件，支持 PIN 输入和角色选择。
 * 登录失败时可通过 showError() 显示错误提示。
 */

#pragma once

#include <QDialog>
#include <QLabel>

#include <ElaLineEdit.h>
#include <ElaRadioButton.h>

class ElaPushButton;

namespace wekey {

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);

    /// @brief 获取输入的 PIN 码
    QString pin() const;
    /// @brief 获取选择的角色（"user" 或 "admin"）
    QString role() const;

private slots:
    void validate();

private:
    void setupUi();

    ElaLineEdit* pinEdit_ = nullptr;
    ElaRadioButton* userRadio_ = nullptr;
    ElaRadioButton* adminRadio_ = nullptr;
    ElaPushButton* okButton_ = nullptr;  ///< 确定按钮
};

}  // namespace wekey
