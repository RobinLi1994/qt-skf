/**
 * @file AppDetailView.cpp
 * @brief 应用详情子视图实现
 */

#include "AppDetailView.h"

#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

#include <ElaContentDialog.h>
#include <ElaLineEdit.h>
#include <ElaMessageBar.h>
#include <ElaPushButton.h>
#include <ElaText.h>

#include "gui/UiHelper.h"
#include "core/container/ContainerService.h"
#include "core/crypto/CertService.h"
#include "core/file/FileService.h"
#include "gui/dialogs/CsrDialog.h"
#include "gui/dialogs/CreateFileDialog.h"
#include "gui/dialogs/CertDetailDialog.h"
#include "gui/dialogs/ImportCertDialog.h"
#include "gui/dialogs/MessageBox.h"

namespace wekey {

AppDetailView::AppDetailView(QWidget* parent) : QWidget(parent) {
    setupUi();
    connectSignals();
}

void AppDetailView::setContext(const QString& devName, const QString& appName) {
    devName_ = devName;
    appName_ = appName;
    titleText_->setText(QString("应用详情: %1").arg(appName_));
    refreshContainers();
    refreshFiles();
}

void AppDetailView::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(UiHelper::kSpaceMD);

    // 标题栏：返回箭头 + 标题
    auto* headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(UiHelper::kSpaceSM);
    backButton_ = new QLabel("←", this);
    backButton_->setCursor(Qt::PointingHandCursor);
    backButton_->setStyleSheet(
        "QLabel { color: #000000; font-size: 20px; }"
        "QLabel:hover { color: #1677ff; }"
    );
    backButton_->installEventFilter(this);
    headerLayout->addWidget(backButton_);
    titleText_ = new ElaText("应用详情", this);
    titleText_->setTextPixelSize(20);
    headerLayout->addWidget(titleText_);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Tab 页（Ant Design 底线风格）
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setStyleSheet(
        "QTabWidget::pane { border: none; }"
        "QTabBar::tab {"
        "  background: transparent;"
        "  color: #595959;"
        "  padding: 8px 16px;"
        "  margin-right: 16px;"
        "  border: none;"
        "  border-bottom: 2px solid transparent;"
        "  font-size: 14px;"
        "}"
        "QTabBar::tab:selected {"
        "  color: #1677ff;"
        "  border-bottom: 2px solid #1677ff;"
        "}"
        "QTabBar::tab:hover {"
        "  color: #4096ff;"
        "}"
    );
    setupContainerTab();
    setupFileTab();
    mainLayout->addWidget(tabWidget_, 1);
}

void AppDetailView::setupContainerTab() {
    auto* containerWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(containerWidget);
    layout->setContentsMargins(UiHelper::kSpaceMD, UiHelper::kSpaceMD,
                               UiHelper::kSpaceMD, UiHelper::kSpaceMD);
    layout->setSpacing(UiHelper::kSpaceMD);

    // 操作栏
    auto* actionLayout = new QHBoxLayout;
    createContainerBtn_ = new ElaPushButton("创建容器", this);
    UiHelper::stylePrimaryButton(createContainerBtn_);
    actionLayout->addWidget(createContainerBtn_);
    refreshContainerBtn_ = new ElaPushButton("刷新", this);
    UiHelper::styleDefaultButton(refreshContainerBtn_);
    actionLayout->addWidget(refreshContainerBtn_);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    // 容器表格
    containerTable_ = new QTableWidget(0, 5, this);
    containerTable_->setHorizontalHeaderLabels({"容器名称", "密钥状态", "密钥类型", "证书状态", "操作"});
    containerTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    containerTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    containerTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    containerTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    containerTable_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    containerTable_->horizontalHeader()->resizeSection(4, 350);
    UiHelper::styleTable(containerTable_);
    layout->addWidget(containerTable_, 1);

    tabWidget_->addTab(containerWidget, "容器管理");
}

void AppDetailView::setupFileTab() {
    auto* fileWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(fileWidget);
    layout->setContentsMargins(UiHelper::kSpaceMD, UiHelper::kSpaceMD,
                               UiHelper::kSpaceMD, UiHelper::kSpaceMD);
    layout->setSpacing(UiHelper::kSpaceMD);

    // 操作栏
    auto* actionLayout = new QHBoxLayout;
    createFileBtn_ = new ElaPushButton("创建文件", this);
    UiHelper::stylePrimaryButton(createFileBtn_);
    actionLayout->addWidget(createFileBtn_);
    refreshFileBtn_ = new ElaPushButton("刷新", this);
    UiHelper::styleDefaultButton(refreshFileBtn_);
    actionLayout->addWidget(refreshFileBtn_);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    // 文件表格
    fileTable_ = new QTableWidget(0, 3, this);
    fileTable_->setHorizontalHeaderLabels({"文件名", "大小", "操作"});
    fileTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    fileTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    fileTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    fileTable_->setColumnWidth(1, 100);
    fileTable_->setColumnWidth(2, 160);
    UiHelper::styleTable(fileTable_);
    layout->addWidget(fileTable_, 1);

    tabWidget_->addTab(fileWidget, "文件管理");
}

void AppDetailView::connectSignals() {
    // backButton_ 是 QLabel，点击通过 eventFilter 处理
    connect(createContainerBtn_, &ElaPushButton::clicked, this, &AppDetailView::onCreateContainer);
    connect(refreshContainerBtn_, &ElaPushButton::clicked, this, &AppDetailView::refreshContainers);
    connect(createFileBtn_, &ElaPushButton::clicked, this, &AppDetailView::onCreateFile);
    connect(refreshFileBtn_, &ElaPushButton::clicked, this, &AppDetailView::refreshFiles);
}

// ============================================================
// 容器操作
// ============================================================

void AppDetailView::refreshContainers() {
    if (refreshingContainers_ || devName_.isEmpty() || appName_.isEmpty()) return;
    refreshingContainers_ = true;

    containerTable_->setRowCount(0);

    auto result = ContainerService::instance().enumContainers(devName_, appName_);
    if (!result.isOk()) {
        refreshingContainers_ = false;
        return;
    }

    for (const auto& c : result.value()) {
        int row = containerTable_->rowCount();
        containerTable_->insertRow(row);

        containerTable_->setItem(row, 0, new QTableWidgetItem(c.containerName));

        // 密钥状态 Tag
        containerTable_->setCellWidget(row, 1, c.keyGenerated
            ? UiHelper::createSuccessTag("已生成")
            : UiHelper::createDefaultTag("未生成"));

        // 密钥类型 Tag
        QString keyTypeStr;
        switch (c.keyType) {
            case ContainerInfo::KeyType::SM2: keyTypeStr = "SM2"; break;
            case ContainerInfo::KeyType::RSA: keyTypeStr = "RSA"; break;
            default: keyTypeStr = "未知"; break;
        }
        containerTable_->setCellWidget(row, 2, UiHelper::createInfoTag(keyTypeStr));

        // 证书状态 Tag
        containerTable_->setCellWidget(row, 3, c.certImported
            ? UiHelper::createSuccessTag("已导入")
            : UiHelper::createDefaultTag("未导入"));

        // 操作链接（纯文字带颜色，无按钮边框）
        auto* actionWidget = new QWidget();
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(16);

        auto* genCsrLink = UiHelper::createActionLink(ElaIconType::FileLines, "生成CSR");
        connect(genCsrLink, &QLabel::linkActivated, this,
                [this, cn = c.containerName]() { onGenCsr(cn); });
        actionLayout->addWidget(genCsrLink);

        auto* importCertLink = UiHelper::createActionLink(ElaIconType::FileImport, "导入");
        connect(importCertLink, &QLabel::linkActivated, this,
                [this, cn = c.containerName]() { onImportCert(cn); });
        actionLayout->addWidget(importCertLink);

        // 只有证书已导入时才可以查看证书
        if (c.certImported) {
            auto* exportCertLink = UiHelper::createActionLink(ElaIconType::FileCircleCheck, "查看证书");
            connect(exportCertLink, &QLabel::linkActivated, this,
                    [this, cn = c.containerName]() { onExportCert(cn); });
            actionLayout->addWidget(exportCertLink);
        } else {
            auto* exportCertLink = UiHelper::createDisabledLink(ElaIconType::FileCircleCheck, "查看证书");
            actionLayout->addWidget(exportCertLink);
        }

        auto* deleteLink = UiHelper::createDangerLink(ElaIconType::TrashCan, "删除");
        connect(deleteLink, &QLabel::linkActivated, this,
                [this, cn = c.containerName]() { onDeleteContainer(cn); });
        actionLayout->addWidget(deleteLink);

        actionLayout->addStretch();
        containerTable_->setCellWidget(row, 4, actionWidget);
    }

    refreshingContainers_ = false;
}

void AppDetailView::onCreateContainer() {
    if (devName_.isEmpty() || appName_.isEmpty()) {
        MessageBox::error(this, "创建容器失败", "请先选择设备和应用");
        return;
    }

    auto* dialog = new QDialog(this);
    dialog->setWindowTitle("创建容器");
    dialog->resize(420, 0);
    UiHelper::styleDialog(dialog);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(4);

    // ---- 容器名称（必填）----
    auto* nameLabel = new QLabel(dialog);
    nameLabel->setTextFormat(Qt::RichText);
    nameLabel->setText(
        "<span style='color:#ff4d4f; font-size:14px;'>* </span>"
        "<span style='color:#000000; font-size:14px;'>容器名称</span>");
    mainLayout->addWidget(nameLabel);

    auto* nameEdit = new ElaLineEdit(dialog);
    UiHelper::styleLineEdit(nameEdit);
    nameEdit->setPlaceholderText("请输入容器名称");
    mainLayout->addWidget(nameEdit);
    mainLayout->addSpacing(16);

    // ---- 分隔线 ----
    mainLayout->addWidget(UiHelper::createDivider(dialog));

    // ---- 按钮 ----
    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    auto* cancelBtn = new ElaPushButton("取消", dialog);
    UiHelper::styleDefaultButton(cancelBtn);
    connect(cancelBtn, &ElaPushButton::clicked, dialog, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto* okBtn = new ElaPushButton("确定", dialog);
    UiHelper::stylePrimaryButton(okBtn);
    okBtn->setEnabled(false);
    connect(okBtn, &ElaPushButton::clicked, dialog, &QDialog::accept);
    btnLayout->addWidget(okBtn);
    mainLayout->addLayout(btnLayout);

    // 输入内容变化时启用/禁用确定按钮
    connect(nameEdit, &ElaLineEdit::textChanged, dialog, [okBtn](const QString& text) {
        okBtn->setEnabled(!text.trimmed().isEmpty());
    });

    if (dialog->exec() != QDialog::Accepted || nameEdit->text().isEmpty()) {
        dialog->deleteLater();
        return;
    }
    QString name = nameEdit->text();
    dialog->deleteLater();

    auto result = ContainerService::instance().createContainer(devName_, appName_, name);
    if (!result.isOk()) {
        MessageBox::error(this, "创建容器失败", result.error());
    } else {
        refreshContainers();
    }
}

void AppDetailView::onDeleteContainer(const QString& containerName) {
    auto* confirmDialog = new ElaContentDialog(this);
    confirmDialog->setWindowTitle("删除确认");
    confirmDialog->setMinimumWidth(500);
    confirmDialog->setLeftButtonText("取消");
    confirmDialog->setRightButtonText("确定");

    // 隐藏中间按钮
    confirmDialog->setMiddleButtonText("");
    auto buttons = confirmDialog->findChildren<ElaPushButton*>();
    if (buttons.size() >= 2) {
        buttons[1]->setVisible(false);
    }

    // 创建中心内容：图标 + 提示文字
    auto* centralWidget = new QWidget(confirmDialog);
    auto* layout = new QHBoxLayout(centralWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);

    // 警告图标
    auto* iconLabel = new QLabel("⚠️", centralWidget);
    iconLabel->setStyleSheet("font-size: 24px;");
    layout->addWidget(iconLabel);

    // 提示文字
    auto* textLabel = new QLabel(QString("确定要删除容器 %1 吗？").arg(containerName), centralWidget);
    textLabel->setStyleSheet("font-size: 14px; color: #000000;");
    textLabel->setWordWrap(true);
    layout->addWidget(textLabel, 1);

    confirmDialog->setCentralWidget(centralWidget);

    connect(confirmDialog, &ElaContentDialog::rightButtonClicked, this,
            [this, containerName]() {
                auto result = ContainerService::instance().deleteContainer(devName_, appName_, containerName);
                if (!result.isOk()) {
                    MessageBox::error(this, "删除容器失败", result.error());
                } else {
                    refreshContainers();
                }
            });

    confirmDialog->exec();
}

void AppDetailView::onGenCsr(const QString& containerName) {
    CsrDialog dialog(devName_, appName_, containerName, this);
    dialog.exec();

    // CSR 生成成功后刷新容器列表
    if (dialog.isGenerated()) {
        refreshContainers();
    }
}

void AppDetailView::onImportCert(const QString& containerName) {
    ImportCertDialog dialog(containerName, this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    // 使用 importKeyCert 统一导入签名证书、加密证书、加密私钥
    auto result = CertService::instance().importKeyCert(
        devName_, appName_, containerName,
        dialog.sigCertData(), dialog.encCertData(), dialog.encPrivateData(), dialog.isNonGM());
    if (!result.isOk()) {
        qWarning() << "[onImportCert] 导入失败:" << result.error().message();
        MessageBox::error(this, "导入证书失败", result.error());
    } else {
        qDebug() << "[onImportCert] 导入成功, container:" << containerName;
        MessageBox::info(this, "成功", "证书和密钥已导入");
        refreshContainers();
    }
}

void AppDetailView::onExportCert(const QString& containerName) {
    CertDetailDialog dialog(devName_, appName_, containerName, this);
    dialog.exec();
}

// ============================================================
// 文件操作
// ============================================================

void AppDetailView::refreshFiles() {
    if (devName_.isEmpty() || appName_.isEmpty()) return;

    fileTable_->setRowCount(0);

    auto result = FileService::instance().enumFiles(devName_, appName_);
    if (result.isErr()) {
        return;
    }

    auto files = result.value();
    fileTable_->setRowCount(files.size());

    for (int i = 0; i < files.size(); ++i) {
        // 文件名
        fileTable_->setItem(i, 0, new QTableWidgetItem(files[i]));

        // 大小：读取文件数据获取实际大小
        QString sizeText = "--";
        auto readResult = FileService::instance().readFile(devName_, appName_, files[i]);
        if (readResult.isOk()) {
            qint64 sz = readResult.value().size();
            if (sz < 1024) {
                sizeText = QString("%1 B").arg(sz);
            } else {
                sizeText = QString("%1 KB").arg(QString::number(sz / 1024.0, 'f', 1));
            }
        }
        auto* sizeItem = new QTableWidgetItem(sizeText);
        sizeItem->setTextAlignment(Qt::AlignCenter);
        fileTable_->setItem(i, 1, sizeItem);

        // 操作链接（纯文字带颜色，无按钮边框）
        auto* buttonWidget = new QWidget(this);
        auto* buttonLayout = new QHBoxLayout(buttonWidget);
        buttonLayout->setContentsMargins(4, 2, 4, 2);
        buttonLayout->setSpacing(16);

        auto* readLink = UiHelper::createActionLink(ElaIconType::FileLines, "读取", buttonWidget);
        connect(readLink, &QLabel::linkActivated, this,
                [this, fn = files[i]]() { onReadFile(fn); });
        buttonLayout->addWidget(readLink);

        auto* deleteLink = UiHelper::createDangerLink(ElaIconType::TrashCan, "删除", buttonWidget);
        connect(deleteLink, &QLabel::linkActivated, this,
                [this, fn = files[i]]() { onDeleteFile(fn); });
        buttonLayout->addWidget(deleteLink);

        buttonLayout->addStretch();
        fileTable_->setCellWidget(i, 2, buttonWidget);
    }
}

void AppDetailView::onCreateFile() {
    CreateFileDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;

    auto fileName = dialog.fileName();
    auto fileData = dialog.fileData();

    qDebug() << "[onCreateFile] fileName:" << fileName << "dataSize:" << fileData.size()
             << "readRights:" << Qt::hex << dialog.readRights()
             << "writeRights:" << Qt::hex << dialog.writeRights();

    auto result = FileService::instance().writeFile(
        devName_, appName_, fileName, fileData,
        dialog.readRights(), dialog.writeRights());

    if (result.isErr()) {
        MessageBox::error(this, "上传文件失败", result.error());
        return;
    }

    ElaMessageBar::success(ElaMessageBarType::TopRight, "成功", "文件上传成功", 2000, this);
    refreshFiles();
}

void AppDetailView::onReadFile(const QString& fileName) {
    QString savePath = QFileDialog::getSaveFileName(this, "保存文件", fileName);
    if (savePath.isEmpty()) {
        return;
    }

    auto result = FileService::instance().readFile(devName_, appName_, fileName);
    if (result.isErr()) {
        MessageBox::error(this, "读取文件失败", result.error());
        return;
    }

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        MessageBox::error(this, "保存文件失败", result.error());
        return;
    }

    file.write(result.value());
    file.close();

    ElaMessageBar::success(ElaMessageBarType::TopRight, "成功",
                           "文件已保存到：" + savePath, 3000, this);
}

void AppDetailView::onDeleteFile(const QString& fileName) {
    auto* confirmDialog = new ElaContentDialog(this);
    confirmDialog->setWindowTitle("删除确认");
    confirmDialog->setMinimumWidth(500);
    confirmDialog->setLeftButtonText("取消");
    confirmDialog->setMiddleButtonText("");
    confirmDialog->setRightButtonText("确定");

    // 隐藏中间按钮
    auto buttons = confirmDialog->findChildren<ElaPushButton*>();
    if (buttons.size() >= 2) {
        buttons[1]->setVisible(false);
    }

    // 创建中心内容：图标 + 提示文字
    auto* centralWidget = new QWidget(confirmDialog);
    auto* layout = new QHBoxLayout(centralWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);

    // 警告图标
    auto* iconLabel = new QLabel("⚠️", centralWidget);
    iconLabel->setStyleSheet("font-size: 24px;");
    layout->addWidget(iconLabel);

    // 提示文字
    auto* textLabel = new QLabel(QString("确定要删除文件 %1 吗？").arg(fileName), centralWidget);
    textLabel->setStyleSheet("font-size: 14px; color: #000000;");
    textLabel->setWordWrap(true);
    layout->addWidget(textLabel, 1);

    confirmDialog->setCentralWidget(centralWidget);

    connect(confirmDialog, &ElaContentDialog::rightButtonClicked, this,
            [this, fileName]() {
                auto result = FileService::instance().deleteFile(devName_, appName_, fileName);
                if (result.isErr()) {
                    MessageBox::error(this, "删除文件失败", result.error());
                    return;
                }
                ElaMessageBar::success(ElaMessageBarType::TopRight, "成功", "文件已删除", 2000, this);
                refreshFiles();
            });

    confirmDialog->exec();
}

bool AppDetailView::eventFilter(QObject* obj, QEvent* event) {
    if (obj == backButton_ && event->type() == QEvent::MouseButtonRelease) {
        emit backRequested();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

}  // namespace wekey
