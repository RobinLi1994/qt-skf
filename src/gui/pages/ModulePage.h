/**
 * @file ModulePage.h
 * @brief 模块管理页 (M5.4)
 */

#pragma once

#include <QTableWidget>

#include <ElaScrollPage.h>
#include <ElaPushButton.h>

namespace wekey {

class ModulePage : public ElaScrollPage {
    Q_OBJECT

public:
    explicit ModulePage(QWidget* parent = nullptr);

    QTableWidget* table() const;
    ElaPushButton* addButton() const;

    void refreshTable();

private:
    void setupUi();
    void connectSignals();
    void onAddModule();
    void onDeleteModule(const QString& name);
    void onActivateModule(const QString& name);

    QTableWidget* table_ = nullptr;
    ElaPushButton* addButton_ = nullptr;
};

}  // namespace wekey
