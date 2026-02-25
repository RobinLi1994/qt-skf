/**
 * @file CreateFileDialog.cpp
 * @brief 上传文件对话框实现
 *
 * 支持选择本地文件上传到设备，设置文件名和读写权限。
 * 权限值对应 SKF 规范 SECURE_* 常量。
 */

#include "CreateFileDialog.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <ElaComboBox.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>

#include "gui/UiHelper.h"

namespace wekey {

// SKF 权限常量（对应文档 6.4.12 权限类型表）
static constexpr int SECURE_NEVER_ACCOUNT  = 0x00000000;  ///< 不允许
static constexpr int SECURE_ADM_ACCOUNT    = 0x00000001;  ///< 管理员权限
static constexpr int SECURE_USER_ACCOUNT   = 0x00000010;  ///< 用户权限
static constexpr int SECURE_ANYONE_ACCOUNT = 0x000000FF;  ///< 任何人

CreateFileDialog::CreateFileDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("上传文件");
    resize(520, 380);
    UiHelper::styleDialog(this);
    setupUi();
}

void CreateFileDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(UiHelper::kSpaceXL, UiHelper::kSpaceXL,
                                   UiHelper::kSpaceXL, UiHelper::kSpaceLG);
    mainLayout->setSpacing(UiHelper::kSpaceLG);

    // ---- 文件名（必填）----
    auto* nameRow = new QVBoxLayout;
    nameRow->setSpacing(UiHelper::kSpaceSM);
    auto* nameLabel = new QLabel(this);
    nameLabel->setTextFormat(Qt::RichText);
    nameLabel->setText("<span style='color:#ff4d4f;'>*</span> 文件名");
    nameRow->addWidget(nameLabel);
    nameEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(nameEdit_);
    nameEdit_->setPlaceholderText("请输入文件名");
    connect(nameEdit_, &ElaLineEdit::textChanged, this, &CreateFileDialog::validate);
    nameRow->addWidget(nameEdit_);
    mainLayout->addLayout(nameRow);

    // ---- 文件选择（必填）----
    auto* fileRow = new QVBoxLayout;
    fileRow->setSpacing(UiHelper::kSpaceSM);
    auto* fileLabel = new QLabel(this);
    fileLabel->setTextFormat(Qt::RichText);
    fileLabel->setText("<span style='color:#ff4d4f;'>*</span> 文件");
    fileRow->addWidget(fileLabel);

    auto* filePickRow = new QHBoxLayout;
    filePickRow->setSpacing(UiHelper::kSpaceSM);

    // 隐藏路径框，只用按钮触发文件选择
    pathEdit_ = new ElaLineEdit(this);
    pathEdit_->setVisible(false);  // 路径在按钮旁边的标签展示

    browseButton_ = new ElaPushButton("  选择文件", this);
    UiHelper::styleDefaultButton(browseButton_);
    browseButton_->setMinimumWidth(110);
    connect(browseButton_, &ElaPushButton::clicked, this, &CreateFileDialog::onBrowseClicked);
    filePickRow->addWidget(browseButton_);

    sizeLabel_ = new QLabel(this);
    sizeLabel_->setStyleSheet("QLabel { color: rgba(0,0,0,0.45); font-size: 13px; }");
    filePickRow->addWidget(sizeLabel_, 1);
    fileRow->addLayout(filePickRow);

    // 错误提示
    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet("QLabel { color: #ff4d4f; font-size: 12px; }");
    errorLabel_->setVisible(false);
    fileRow->addWidget(errorLabel_);

    mainLayout->addLayout(fileRow);

    // ---- 读取权限（可选）----
    auto* readRow = new QVBoxLayout;
    readRow->setSpacing(UiHelper::kSpaceSM);
    readRow->addWidget(new QLabel("读取权限", this));
    readRightsCombo_ = new ElaComboBox(this);
    UiHelper::styleComboBox(readRightsCombo_);
    readRightsCombo_->addItem("限制权限",   SECURE_NEVER_ACCOUNT);
    readRightsCombo_->addItem("管理员权限", SECURE_ADM_ACCOUNT);
    readRightsCombo_->addItem("普通用户权限", SECURE_USER_ACCOUNT);
    readRightsCombo_->addItem("任何人权限", SECURE_ANYONE_ACCOUNT);
    readRightsCombo_->setCurrentIndex(3);  // 默认任何人可读
    // placeholder 效果（ElaComboBox 不支持 placeholder，用第一项空占位已足够）
    readRow->addWidget(readRightsCombo_);
    mainLayout->addLayout(readRow);

    // ---- 写入权限（可选）----
    auto* writeRow = new QVBoxLayout;
    writeRow->setSpacing(UiHelper::kSpaceSM);
    writeRow->addWidget(new QLabel("写入权限", this));
    writeRightsCombo_ = new ElaComboBox(this);
    UiHelper::styleComboBox(writeRightsCombo_);
    writeRightsCombo_->addItem("限制权限",   SECURE_NEVER_ACCOUNT);
    writeRightsCombo_->addItem("管理员权限", SECURE_ADM_ACCOUNT);
    writeRightsCombo_->addItem("普通用户权限", SECURE_USER_ACCOUNT);
    writeRightsCombo_->addItem("任何人权限", SECURE_ANYONE_ACCOUNT);
    writeRightsCombo_->setCurrentIndex(3);  // 默认任何人可写
    writeRow->addWidget(writeRightsCombo_);
    mainLayout->addLayout(writeRow);

    mainLayout->addStretch();
    mainLayout->addWidget(UiHelper::createDivider(this));

    // ---- 底部按钮 ----
    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    cancelButton_ = new ElaPushButton("取 消", this);
    UiHelper::styleDefaultButton(cancelButton_);
    connect(cancelButton_, &ElaPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelButton_);
    okButton_ = new ElaPushButton("确 定", this);
    UiHelper::stylePrimaryButton(okButton_);
    okButton_->setEnabled(false);
    connect(okButton_, &ElaPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okButton_);
    mainLayout->addLayout(btnLayout);
}

QString CreateFileDialog::fileName() const {
    return nameEdit_->text().trimmed();
}

QString CreateFileDialog::filePath() const {
    return pathEdit_->text().trimmed();
}

int CreateFileDialog::readRights() const {
    return readRightsCombo_->currentData().toInt();
}

int CreateFileDialog::writeRights() const {
    return writeRightsCombo_->currentData().toInt();
}

QByteArray CreateFileDialog::fileData() const {
    return fileData_;
}

void CreateFileDialog::onBrowseClicked() {
    QString path = QFileDialog::getOpenFileName(this, "选择要上传的文件");
    if (path.isEmpty()) return;

    pathEdit_->setText(path);
    onFilePathChanged();

    // 若文件名为空，自动填充为本地文件名
    if (nameEdit_->text().trimmed().isEmpty()) {
        QFileInfo fileInfo(path);
        nameEdit_->setText(fileInfo.fileName());
    }
}

void CreateFileDialog::onFilePathChanged() {
    QString path = pathEdit_->text().trimmed();
    fileData_.clear();
    errorLabel_->setVisible(false);

    if (path.isEmpty()) {
        sizeLabel_->setText("");
        validate();
        return;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        errorLabel_->setText("文件不存在");
        errorLabel_->setVisible(true);
        sizeLabel_->setText("");
        validate();
        return;
    }

    qint64 size = fileInfo.size();
    if (size > MAX_FILE_SIZE) {
        errorLabel_->setText(QString("文件大小 %1 字节，超出 %2 字节限制")
                                 .arg(size).arg(MAX_FILE_SIZE));
        errorLabel_->setVisible(true);
        sizeLabel_->setText("");
        validate();
        return;
    }

    // 读取文件数据
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        errorLabel_->setText("无法读取文件：" + file.errorString());
        errorLabel_->setVisible(true);
        validate();
        return;
    }
    fileData_ = file.readAll();
    file.close();

    sizeLabel_->setText(QString("%1  字节").arg(fileData_.size()));
    qDebug() << "[CreateFileDialog] 文件已加载, path:" << path << "size:" << fileData_.size();
    validate();
}

void CreateFileDialog::validate() {
    bool valid = !nameEdit_->text().trimmed().isEmpty() && !fileData_.isEmpty();
    okButton_->setEnabled(valid);
}

}  // namespace wekey
