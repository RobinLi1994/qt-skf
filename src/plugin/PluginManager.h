/**
 * @file PluginManager.h
 * @brief 插件管理器
 *
 * 管理驱动插件的注册、卸载和切换
 */

#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

#include "common/Result.h"
#include "plugin/interface/IDriverPlugin.h"

namespace wekey {

/**
 * @brief 插件管理器单例
 *
 * 负责管理所有驱动插件的生命周期：
 * - 注册/卸载插件（通过库路径创建 SkfPlugin 实例）
 * - 查询/切换激活插件
 * - 列出所有已注册插件
 */
class PluginManager : public QObject {
    Q_OBJECT

public:
    static PluginManager& instance();

    // 禁用拷贝和移动
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    PluginManager(PluginManager&&) = delete;
    PluginManager& operator=(PluginManager&&) = delete;

    /**
     * @brief 注册插件
     * @param name 插件名称
     * @param libPath SKF 库路径
     * @param emitSignals 是否发出信号（默认 true）
     * @return 操作结果
     */
    Result<void> registerPlugin(const QString& name, const QString& libPath, bool emitSignals = true);

    /**
     * @brief 注册已有插件实例（用于测试注入）
     * @param name 插件名称
     * @param plugin 插件实例
     * @return 操作结果
     */
    Result<void> registerPluginInstance(const QString& name, std::shared_ptr<IDriverPlugin> plugin);

    /**
     * @brief 卸载插件
     * @param name 插件名称
     * @param emitSignals 是否发出信号（默认 true）
     * @return 操作结果
     */
    Result<void> unregisterPlugin(const QString& name, bool emitSignals = true);

    /**
     * @brief 获取插件实例
     * @param name 插件名称
     * @return 插件指针（未找到返回 nullptr）
     */
    IDriverPlugin* getPlugin(const QString& name) const;

    /**
     * @brief 获取插件库路径
     * @param name 插件名称
     * @return 库路径（未找到返回空字符串）
     */
    QString getPluginPath(const QString& name) const;

    /**
     * @brief 获取当前激活的插件
     * @return 插件指针（无激活返回 nullptr）
     */
    IDriverPlugin* activePlugin() const;

    /**
     * @brief 获取当前激活的插件名称
     * @return 插件名称（无激活返回空字符串）
     */
    QString activePluginName() const;

    /**
     * @brief 设置激活插件
     * @param name 插件名称
     * @param emitSignals 是否发出信号（默认 true）
     * @return 操作结果
     */
    Result<void> setActivePlugin(const QString& name, bool emitSignals = true);

    /**
     * @brief 列出所有已注册插件名称
     * @return 插件名称列表
     */
    QStringList listPlugins() const;

signals:
    void pluginRegistered(const QString& name);
    void pluginUnregistered(const QString& name);
    void activePluginChanged(const QString& name);

private:
    PluginManager();
    ~PluginManager() override = default;

    struct PluginEntry {
        QString libPath;
        std::shared_ptr<IDriverPlugin> plugin;
    };

    QMap<QString, PluginEntry> plugins_;
    QString activePluginName_;
};

}  // namespace wekey
