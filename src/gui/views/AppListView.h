/**
 * @file AppListView.h
 * @brief 应用列表子视图
 *
 * 在设备管理页面中，点击设备序列号后显示该设备的应用列表。
 * 支持创建、登录、修改PIN、解锁PIN、查看详情、删除等操作。
 */

#pragma once

#include <QLabel>
#include <QTableWidget>
#include <QWidget>

class ElaPushButton;
class ElaText;

namespace wekey {

class AppListView : public QWidget {
    Q_OBJECT

public:
    explicit AppListView(QWidget* parent = nullptr);

    /**
     * @brief 设置当前设备并刷新应用列表
     * @param devName 设备名称
     */
    void setDevice(const QString& devName);

    /**
     * @brief 刷新应用列表
     */
    void refreshApps();

signals:
    /// 返回设备列表
    void backRequested();
    /// 请求查看应用详情（容器+文件）
    void detailRequested(const QString& devName, const QString& appName);

private:
    void setupUi();
    void connectSignals();
    void onCreateApp();
    void onDeleteApp(const QString& appName);
    void onLogin(const QString& appName);
    void onLogout(const QString& appName);
    void onChangePin(const QString& appName);
    void onUnlockPin(const QString& appName);

    QString devName_;
    bool eventFilter(QObject* obj, QEvent* event) override;

    ElaText* titleText_ = nullptr;
    QLabel* backButton_ = nullptr;
    ElaPushButton* createButton_ = nullptr;
    ElaPushButton* refreshButton_ = nullptr;
    QTableWidget* table_ = nullptr;
    bool refreshing_ = false;
};

}  // namespace wekey
