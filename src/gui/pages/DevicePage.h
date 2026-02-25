/**
 * @file DevicePage.h
 * @brief 设备管理页
 *
 * 三级钻取导航：设备列表 → 应用列表 → 应用详情（容器+文件）
 */

#pragma once

#include <QList>
#include <QStackedWidget>
#include <QTableWidget>

#include <ElaPushButton.h>
#include <ElaScrollPage.h>
#include <ElaScrollPageArea.h>
#include <ElaText.h>

#include "plugin/interface/PluginTypes.h"

namespace wekey {

class AppListView;
class AppDetailView;

class DevicePage : public ElaScrollPage {
    Q_OBJECT

public:
    explicit DevicePage(QWidget* parent = nullptr);

    void refreshTable();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUi();
    void connectSignals();
    void onSetLabel(const QString& devName);
    void onChangeAuth(const QString& devName);
    void updateDetails(int row);

    /// 导航到应用列表
    void navigateToAppList(const QString& devName);
    /// 导航到应用详情
    void navigateToAppDetail(const QString& devName, const QString& appName);
    /// 返回设备列表
    void navigateToDeviceList();

    QStackedWidget* stack_ = nullptr;

    // 视图 0: 设备列表
    QWidget* deviceListWidget_ = nullptr;
    QTableWidget* table_ = nullptr;
    ElaPushButton* refreshButton_ = nullptr;
    ElaScrollPageArea* detailsGroup_ = nullptr;
    ElaText* manufacturerValue_ = nullptr;
    ElaText* hwVersionValue_ = nullptr;
    ElaText* fwVersionValue_ = nullptr;

    // 视图 1: 应用列表
    AppListView* appListView_ = nullptr;

    // 视图 2: 应用详情（容器+文件 Tab）
    AppDetailView* appDetailView_ = nullptr;

    QList<DeviceInfo> devices_;
    bool refreshing_ = false;
};

}  // namespace wekey
