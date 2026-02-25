/**
 * @file CreateFileDialog.h
 * @brief 创建文件对话框
 *
 * 用于配置文件名、选择本地文件、设置读写权限
 */

#pragma once

#include <QDialog>
#include <QLabel>

#include <ElaComboBox.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>

namespace wekey {

class CreateFileDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateFileDialog(QWidget* parent = nullptr);

    /// 获取设备文件名
    QString fileName() const;
    /// 获取已选择的本地文件路径
    QString filePath() const;
    /// 获取读权限值（SKF 规范：0x00=禁止 0x01=管理员 0x10=用户 0xFF=任何人）
    int readRights() const;
    /// 获取写权限值（同上）
    int writeRights() const;
    /// 获取已读取的文件数据
    QByteArray fileData() const;

private slots:
    void onBrowseClicked();
    void onFilePathChanged();
    void validate();

private:
    void setupUi();

    ElaLineEdit* nameEdit_ = nullptr;
    ElaLineEdit* pathEdit_ = nullptr;
    ElaPushButton* browseButton_ = nullptr;
    ElaComboBox* readRightsCombo_ = nullptr;
    ElaComboBox* writeRightsCombo_ = nullptr;
    QLabel* sizeLabel_ = nullptr;
    QLabel* errorLabel_ = nullptr;   ///< 文件大小/错误提示
    ElaPushButton* okButton_ = nullptr;
    ElaPushButton* cancelButton_ = nullptr;

    QByteArray fileData_;  ///< 已读取的本地文件数据

    static constexpr qint64 MAX_FILE_SIZE = 65536;  // 64KB（与 readFile 缓冲区一致）
};

}  // namespace wekey
