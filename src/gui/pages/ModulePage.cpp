/**
 * @file ModulePage.cpp
 * @brief 模块管理页实现
 */

#include "ModulePage.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>

#include <ElaPushButton.h>
#include <ElaScrollPageArea.h>
#include <ElaText.h>

#include "gui/UiHelper.h"
#include "config/Config.h"
#include "gui/dialogs/MessageBox.h"
#include "plugin/PluginManager.h"

namespace wekey {

ModulePage::ModulePage(QWidget* parent) : ElaScrollPage(parent) {
    setWindowTitle("模块管理");
    setTitleVisible(false);
    setupUi();
    connectSignals();
    refreshTable();
}

QTableWidget* ModulePage::table() const { return table_; }

ElaPushButton* ModulePage::addButton() const { return addButton_; }

void ModulePage::setupUi() {
    // 按钮区域（用 ElaScrollPageArea 包裹）
    auto* buttonArea = new ElaScrollPageArea(this);
    UiHelper::styleCard(buttonArea);
    auto* buttonLayout = new QHBoxLayout(buttonArea);
    auto* descText = new ElaText("管理 SKF 驱动模块的加载、激活和删除", this);
    descText->setTextStyle(ElaTextType::Body);
    descText->setWordWrap(false);
    buttonLayout->addWidget(descText);
    buttonLayout->addStretch();
    addButton_ = new ElaPushButton("添加模块", this);
    UiHelper::stylePrimaryButton(addButton_);
    buttonLayout->addWidget(addButton_);

    // 表格
    table_ = new QTableWidget(0, 4, this);
    table_->setHorizontalHeaderLabels({"名称", "路径", "状态", "操作"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    table_->horizontalHeader()->resizeSection(3, 150);
    UiHelper::styleTable(table_);

    // 组装到中心区域
    auto* centralWidget = new QWidget(this);
    auto* centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(UiHelper::kSpaceMD);
    centerLayout->addWidget(buttonArea);
    centerLayout->addWidget(table_, 1);
    addCentralWidget(centralWidget, true, true, 0);
}

void ModulePage::connectSignals() {
    auto& pm = PluginManager::instance();
    connect(addButton_, &ElaPushButton::clicked, this, &ModulePage::onAddModule);
    connect(&pm, &PluginManager::pluginRegistered, this, &ModulePage::refreshTable);
    connect(&pm, &PluginManager::pluginUnregistered, this, &ModulePage::refreshTable);
    connect(&pm, &PluginManager::activePluginChanged, this, &ModulePage::refreshTable);
}

void ModulePage::onAddModule() {
    QString filter = "SKF 库文件 (*.dll *.dylib *.so);;所有文件 (*)";
    QString filePath = QFileDialog::getOpenFileName(this, "选择 SKF 库文件", QString(), filter);
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fi(filePath);
    QString name = fi.completeBaseName();
    if (name.startsWith("lib")) {
        name = name.mid(3);
    }

    auto result = PluginManager::instance().registerPlugin(name, filePath);
    if (!result.isOk()) {
        MessageBox::error(this, "添加模块失败", result.error());
    } else {
        Config::instance().setModPath(name, filePath);
        Config::instance().save();
    }
}

void ModulePage::refreshTable() {
    table_->setRowCount(0);

    auto& pm = PluginManager::instance();
    QStringList plugins = pm.listPlugins();
    QString activeName = pm.activePluginName();

    for (const auto& name : plugins) {
        int row = table_->rowCount();
        table_->insertRow(row);

        table_->setItem(row, 0, new QTableWidgetItem(name));
        table_->setItem(row, 1, new QTableWidgetItem(pm.getPluginPath(name)));

        bool isActive = (name == activeName);
        // 状态列用 Ant Design Tag 样式
        auto* statusTag = isActive
            ? UiHelper::createSuccessTag("已激活")
            : UiHelper::createDefaultTag("未激活");
        table_->setCellWidget(row, 2, statusTag);

        // 操作链接（纯文字带颜色，无按钮边框）
        auto* actionWidget = new QWidget();
        auto* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(16);

        if (!isActive) {
            auto* activateLink = UiHelper::createActionLink(ElaIconType::CircleCheck, "激活");
            connect(activateLink, &QLabel::linkActivated, this, [this, name]() {
                onActivateModule(name);
            });
            actionLayout->addWidget(activateLink);
        }

        auto* deleteLink = UiHelper::createDangerLink(ElaIconType::TrashCan, "删除");
        connect(deleteLink, &QLabel::linkActivated, this, [this, name]() {
            onDeleteModule(name);
        });
        actionLayout->addWidget(deleteLink);

        actionLayout->addStretch();
        table_->setCellWidget(row, 3, actionWidget);
    }
}

void ModulePage::onDeleteModule(const QString& name) {
    auto result = PluginManager::instance().unregisterPlugin(name);
    if (!result.isOk()) {
        MessageBox::error(this, "删除模块失败", result.error());
    } else {
        auto& config = Config::instance();
        config.removeModPath(name);
        if (config.activedModName() == name) {
            config.setActivedModName(QString());
        }
        config.save();
    }
}

void ModulePage::onActivateModule(const QString& name) {
    auto result = PluginManager::instance().setActivePlugin(name);
    if (!result.isOk()) {
        MessageBox::error(this, "激活模块失败", result.error());
    } else {
        Config::instance().setActivedModName(name);
        Config::instance().save();
    }
}

}  // namespace wekey
