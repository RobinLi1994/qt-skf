/**
 * @file ConfigPage.h
 * @brief 配置管理页 (M5.8)
 */

#pragma once

#include <ElaComboBox.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaRadioButton.h>
#include <ElaScrollPage.h>
#include <ElaSpinBox.h>

namespace wekey {

class ConfigPage : public ElaScrollPage {
    Q_OBJECT

public:
    explicit ConfigPage(QWidget* parent = nullptr);

    ElaLineEdit* appNameEdit() const;
    ElaLineEdit* containerNameEdit() const;
    ElaLineEdit* commonNameEdit() const;
    ElaLineEdit* organizationEdit() const;
    ElaLineEdit* unitEdit() const;
    ElaRadioButton* roleUserRadio() const;
    ElaRadioButton* roleAdminRadio() const;
    ElaSpinBox* portSpin() const;
    ElaComboBox* logLevelCombo() const;
    ElaRadioButton* errorSimpleRadio() const;
    ElaRadioButton* errorDetailedRadio() const;
    ElaPushButton* saveButton() const;
    ElaPushButton* resetButton() const;

private:
    void setupUi();
    void loadFromConfig();

    ElaLineEdit* appNameEdit_ = nullptr;
    ElaLineEdit* containerNameEdit_ = nullptr;
    ElaLineEdit* commonNameEdit_ = nullptr;
    ElaLineEdit* organizationEdit_ = nullptr;
    ElaLineEdit* unitEdit_ = nullptr;
    ElaRadioButton* roleUserRadio_ = nullptr;
    ElaRadioButton* roleAdminRadio_ = nullptr;
    ElaSpinBox* portSpin_ = nullptr;
    ElaComboBox* logLevelCombo_ = nullptr;
    ElaRadioButton* errorSimpleRadio_ = nullptr;
    ElaRadioButton* errorDetailedRadio_ = nullptr;
    ElaPushButton* saveButton_ = nullptr;
    ElaPushButton* resetButton_ = nullptr;
};

}  // namespace wekey
