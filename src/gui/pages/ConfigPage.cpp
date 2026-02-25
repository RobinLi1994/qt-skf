/**
 * @file ConfigPage.cpp
 * @brief 配置管理页实现
 */

#include "ConfigPage.h"

#include <QButtonGroup>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <ElaComboBox.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaRadioButton.h>
#include <ElaScrollPageArea.h>
#include <ElaSpinBox.h>
#include <ElaText.h>

#include "gui/UiHelper.h"
#include "config/Config.h"
#include "log/Logger.h"

namespace wekey {

ConfigPage::ConfigPage(QWidget* parent) : ElaScrollPage(parent) {
    setWindowTitle("配置管理");
    setTitleVisible(false);
    setupUi();
    loadFromConfig();
}

ElaLineEdit* ConfigPage::appNameEdit() const { return appNameEdit_; }
ElaLineEdit* ConfigPage::containerNameEdit() const { return containerNameEdit_; }
ElaLineEdit* ConfigPage::commonNameEdit() const { return commonNameEdit_; }
ElaLineEdit* ConfigPage::organizationEdit() const { return organizationEdit_; }
ElaLineEdit* ConfigPage::unitEdit() const { return unitEdit_; }
ElaRadioButton* ConfigPage::roleUserRadio() const { return roleUserRadio_; }
ElaRadioButton* ConfigPage::roleAdminRadio() const { return roleAdminRadio_; }
ElaSpinBox* ConfigPage::portSpin() const { return portSpin_; }
ElaComboBox* ConfigPage::logLevelCombo() const { return logLevelCombo_; }
ElaRadioButton* ConfigPage::errorSimpleRadio() const { return errorSimpleRadio_; }
ElaRadioButton* ConfigPage::errorDetailedRadio() const { return errorDetailedRadio_; }
ElaPushButton* ConfigPage::saveButton() const { return saveButton_; }
ElaPushButton* ConfigPage::resetButton() const { return resetButton_; }

void ConfigPage::setupUi() {
    // 默认值设置
    auto* defaultsGroup = new ElaScrollPageArea(this);
    UiHelper::styleCard(defaultsGroup);
    auto* defaultsVLayout = new QVBoxLayout(defaultsGroup);
    auto* defaultsTitle = new ElaText("默认值设置", this);
    defaultsTitle->setTextStyle(ElaTextType::BodyStrong);
    defaultsVLayout->addWidget(defaultsTitle);
    auto* defaultsLayout = new QFormLayout;
    defaultsLayout->setSpacing(UiHelper::kSpaceMD);
    defaultsLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    defaultsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    defaultsVLayout->addLayout(defaultsLayout);

    appNameEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(appNameEdit_);
    appNameEdit_->setMaximumWidth(300);
    defaultsLayout->addRow("默认应用名:", appNameEdit_);

    containerNameEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(containerNameEdit_);
    containerNameEdit_->setMaximumWidth(300);
    defaultsLayout->addRow("默认容器名:", containerNameEdit_);

    commonNameEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(commonNameEdit_);
    commonNameEdit_->setMaximumWidth(300);
    defaultsLayout->addRow("默认通用名:", commonNameEdit_);

    organizationEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(organizationEdit_);
    organizationEdit_->setMaximumWidth(300);
    defaultsLayout->addRow("默认组织:", organizationEdit_);

    unitEdit_ = new ElaLineEdit(this);
    UiHelper::styleLineEdit(unitEdit_);
    unitEdit_->setMaximumWidth(300);
    defaultsLayout->addRow("默认部门:", unitEdit_);

    auto* roleLayout = new QHBoxLayout;
    roleUserRadio_ = new ElaRadioButton("用户", this);
    roleAdminRadio_ = new ElaRadioButton("管理员", this);
    auto* roleGroup = new QButtonGroup(this);
    roleGroup->addButton(roleUserRadio_);
    roleGroup->addButton(roleAdminRadio_);
    roleLayout->addWidget(roleUserRadio_);
    roleLayout->addWidget(roleAdminRadio_);
    roleLayout->addStretch();
    defaultsLayout->addRow("默认角色:", roleLayout);

    // 系统设置
    auto* systemGroup = new ElaScrollPageArea(this);
    UiHelper::styleCard(systemGroup);
    auto* systemVLayout = new QVBoxLayout(systemGroup);
    auto* systemTitle = new ElaText("系统设置", this);
    systemTitle->setTextStyle(ElaTextType::BodyStrong);
    systemVLayout->addWidget(systemTitle);
    auto* systemLayout = new QFormLayout;
    systemLayout->setSpacing(UiHelper::kSpaceMD);
    systemLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    systemLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    systemVLayout->addLayout(systemLayout);

    portSpin_ = new ElaSpinBox(this);
    portSpin_->setRange(1024, 65535);
    portSpin_->setFixedWidth(120);
    systemLayout->addRow("HTTP 端口:", portSpin_);

    logLevelCombo_ = new ElaComboBox(this);
    UiHelper::styleComboBox(logLevelCombo_);
    logLevelCombo_->addItem("Debug", "debug");
    logLevelCombo_->addItem("Info",  "info");
    logLevelCombo_->setFixedWidth(120);
    systemLayout->addRow("日志级别:", logLevelCombo_);

    auto* errorLayout = new QHBoxLayout;
    errorSimpleRadio_ = new ElaRadioButton("简洁", this);
    errorDetailedRadio_ = new ElaRadioButton("详细", this);
    auto* errorGroup = new QButtonGroup(this);
    errorGroup->addButton(errorSimpleRadio_);
    errorGroup->addButton(errorDetailedRadio_);
    errorLayout->addWidget(errorSimpleRadio_);
    errorLayout->addWidget(errorDetailedRadio_);
    errorLayout->addStretch();
    systemLayout->addRow("错误提示:", errorLayout);

    // 按钮区域
    auto* buttonArea = new ElaScrollPageArea(this);
    UiHelper::styleCard(buttonArea);
    auto* buttonLayout = new QHBoxLayout(buttonArea);
    buttonLayout->addStretch();
    resetButton_ = new ElaPushButton("恢复默认", this);
    UiHelper::styleDefaultButton(resetButton_);
    buttonLayout->addWidget(resetButton_);
    saveButton_ = new ElaPushButton("保存", this);
    UiHelper::stylePrimaryButton(saveButton_);
    buttonLayout->addWidget(saveButton_);

    // 组装到中心区域
    auto* centralWidget = new QWidget(this);
    auto* centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(UiHelper::kSpaceMD);
    centerLayout->addWidget(defaultsGroup);
    centerLayout->addWidget(systemGroup);
    centerLayout->addWidget(buttonArea);
    centerLayout->addStretch();
    addCentralWidget(centralWidget, true, true, 0);

    connect(saveButton_, &ElaPushButton::clicked, this, [this]() {
        auto& cfg = Config::instance();
        cfg.setDefault("appName", appNameEdit_->text());
        cfg.setDefault("containerName", containerNameEdit_->text());
        cfg.setDefault("commonName", commonNameEdit_->text());
        cfg.setDefault("organization", organizationEdit_->text());
        cfg.setDefault("unit", unitEdit_->text());
        cfg.setDefault("role", roleUserRadio_->isChecked() ? "user" : "admin");
        cfg.setListenPort(":" + QString::number(portSpin_->value()));
        cfg.setErrorMode(errorSimpleRadio_->isChecked() ? "simple" : "detailed");
        QString logLevel = logLevelCombo_->currentData().toString();
        cfg.setLogLevel(logLevel);
        // 立即更新 Logger 运行时级别，无需重启
        Logger::instance().setLevel(stringToLogLevel(logLevel));
        cfg.save();
    });

    connect(resetButton_, &ElaPushButton::clicked, this, [this]() {
        Config::instance().reset();
        loadFromConfig();
    });
}

void ConfigPage::loadFromConfig() {
    auto& cfg = Config::instance();
    appNameEdit_->setText(cfg.defaultAppName());
    containerNameEdit_->setText(cfg.defaultContainerName());
    commonNameEdit_->setText(cfg.defaultCommonName());
    organizationEdit_->setText(cfg.defaultOrganization());
    unitEdit_->setText(cfg.defaultUnit());

    if (cfg.defaultRole() == "admin") {
        roleAdminRadio_->setChecked(true);
    } else {
        roleUserRadio_->setChecked(true);
    }

    QString portStr = cfg.listenPort();
    portStr.remove(':');
    portSpin_->setValue(portStr.toInt());

    // 日志级别
    QString level = cfg.logLevel().toLower();
    int levelIdx = logLevelCombo_->findData(level);
    logLevelCombo_->setCurrentIndex(levelIdx >= 0 ? levelIdx : 0);

    if (cfg.errorMode() == "detailed") {
        errorDetailedRadio_->setChecked(true);
    } else {
        errorSimpleRadio_->setChecked(true);
    }
}

}  // namespace wekey
