/**
 * @file CertDetailDialog.h
 * @brief 证书详情对话框
 *
 * 展示容器中签名证书和加密证书的详细信息。
 * 使用 ElaWidgetTools 组件统一风格。
 */

#pragma once

#include <QDialog>
#include <QVBoxLayout>

#include "plugin/interface/PluginTypes.h"

class ElaPushButton;
class QLabel;
class QTextEdit;

namespace wekey {

class CertDetailDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param devName 设备名称
     * @param appName 应用名称
     * @param containerName 容器名称
     * @param parent 父窗口
     */
    explicit CertDetailDialog(const QString& devName, const QString& appName,
                              const QString& containerName, QWidget* parent = nullptr);

private:
    void setupUi(const QString& containerName);

    /// 添加一个证书信息折叠区域
    void addCertSection(QVBoxLayout* layout, const CertInfo& info, bool isSignCert);

    /// 添加一行 key-value 信息
    void addInfoRow(QVBoxLayout* layout, const QString& label, const QString& value);

    /// 添加一行 key-widget 信息
    void addInfoRow(QVBoxLayout* layout, const QString& label, QWidget* widget);

    /// 创建证书类型 Tag
    QWidget* createCertTypeTag(bool isSignCert);

    QString devName_;
    QString appName_;
    QString containerName_;
};

}  // namespace wekey
