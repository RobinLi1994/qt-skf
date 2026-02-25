/**
 * @file LogPage.h
 * @brief 日志查看页 (M5.9)
 */

#pragma once

#include <ElaComboBox.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaScrollPage.h>
#include <ElaTableView.h>

#include "log/LogModel.h"

namespace wekey {

class LogPage : public ElaScrollPage {
    Q_OBJECT

public:
    explicit LogPage(QWidget* parent = nullptr);

    ElaLineEdit* searchEdit() const;
    ElaComboBox* levelCombo() const;
    ElaPushButton* clearButton() const;
    ElaTableView* tableView() const;
    LogModel* logModel() const;

private:
    void setupUi();

    ElaLineEdit* searchEdit_ = nullptr;
    ElaComboBox* levelCombo_ = nullptr;
    ElaPushButton* clearButton_ = nullptr;
    ElaTableView* tableView_ = nullptr;
    LogModel* logModel_ = nullptr;
};

}  // namespace wekey
