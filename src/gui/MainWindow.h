/**
 * @file MainWindow.h
 * @brief 主窗口 (M5.2)
 *
 * 左侧导航栏 + 右侧内容区 + 状态栏
 */

#pragma once

#include <ElaWindow.h>

class ElaContentDialog;

namespace wekey {

class SystemTray;

class MainWindow : public ElaWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupNavigation();

    SystemTray* systemTray_ = nullptr;
    ElaContentDialog* closeDialog_ = nullptr;
};

}  // namespace wekey
