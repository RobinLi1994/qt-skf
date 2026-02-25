/**
 * @file AppDetailView.h
 * @brief 应用详情子视图
 *
 * 在应用列表中点击"详情"后显示，包含容器管理和文件管理两个 Tab。
 */

#pragma once

#include <QLabel>
#include <QTableWidget>
#include <QTabWidget>
#include <QWidget>

class ElaPushButton;
class ElaText;

namespace wekey {

class AppDetailView : public QWidget {
    Q_OBJECT

public:
    explicit AppDetailView(QWidget* parent = nullptr);

    /**
     * @brief 设置当前设备和应用，并刷新数据
     * @param devName 设备名称
     * @param appName 应用名称
     */
    void setContext(const QString& devName, const QString& appName);

    /// 刷新容器列表
    void refreshContainers();
    /// 刷新文件列表
    void refreshFiles();

signals:
    /// 返回应用列表
    void backRequested();

private:
    void setupUi();
    void setupContainerTab();
    void setupFileTab();
    void connectSignals();

    // 容器操作
    void onCreateContainer();
    void onDeleteContainer(const QString& containerName);
    void onGenCsr(const QString& containerName);
    void onImportCert(const QString& containerName);
    void onExportCert(const QString& containerName);

    // 文件操作
    void onCreateFile();
    void onReadFile(const QString& fileName);
    void onDeleteFile(const QString& fileName);

    QString devName_;
    QString appName_;

    bool eventFilter(QObject* obj, QEvent* event) override;

    ElaText* titleText_ = nullptr;
    QLabel* backButton_ = nullptr;
    QTabWidget* tabWidget_ = nullptr;

    // 容器 Tab
    QTableWidget* containerTable_ = nullptr;
    ElaPushButton* createContainerBtn_ = nullptr;
    ElaPushButton* refreshContainerBtn_ = nullptr;
    bool refreshingContainers_ = false;

    // 文件 Tab
    QTableWidget* fileTable_ = nullptr;
    ElaPushButton* createFileBtn_ = nullptr;
    ElaPushButton* refreshFileBtn_ = nullptr;
};

}  // namespace wekey
