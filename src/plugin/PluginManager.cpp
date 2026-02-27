/**
 * @file PluginManager.cpp
 * @brief 插件管理器实现
 */

#include "PluginManager.h"

#include "plugin/skf/SkfPlugin.h"

namespace wekey {

PluginManager& PluginManager::instance() {
    static PluginManager instance;
    return instance;
}

PluginManager::PluginManager() : QObject(nullptr) {}

Result<void> PluginManager::registerPlugin(const QString& name, const QString& libPath, bool emitSignals) {
    if (name.isEmpty() || libPath.isEmpty()) {
        return Result<void>::err(
            Error(Error::InvalidParam, "插件名称和路径不能为空", "PluginManager::registerPlugin"));
    }

    if (plugins_.contains(name)) {
        return Result<void>::err(
            Error(Error::AlreadyExists, "插件已注册：" + name, "PluginManager::registerPlugin"));
    }

    auto plugin = std::make_shared<SkfPlugin>();
    plugin->initialize(libPath);

    // 即使初始化失败也注册（路径可能指向尚未就绪的设备驱动），
    // 但保留插件实例以便后续重试或路径查询
    PluginEntry entry;
    entry.libPath = libPath;
    entry.plugin = plugin;
    plugins_.insert(name, entry);

    if (emitSignals) {
        emit pluginRegistered(name);
    }
    return Result<void>::ok();
}

Result<void> PluginManager::registerPluginInstance(const QString& name, std::shared_ptr<IDriverPlugin> plugin) {
    if (name.isEmpty() || !plugin) {
        return Result<void>::err(
            Error(Error::InvalidParam, "插件名称和实例不能为空",
                  "PluginManager::registerPluginInstance"));
    }

    if (plugins_.contains(name)) {
        return Result<void>::err(
            Error(Error::AlreadyExists, "插件已注册：" + name,
                  "PluginManager::registerPluginInstance"));
    }

    PluginEntry entry;
    entry.libPath = QStringLiteral("<injected>");
    entry.plugin = std::move(plugin);
    plugins_.insert(name, entry);

    emit pluginRegistered(name);
    return Result<void>::ok();
}

Result<void> PluginManager::unregisterPlugin(const QString& name, bool emitSignals) {
    if (!plugins_.contains(name)) {
        return Result<void>::err(
            Error(Error::NotFound, "插件未找到：" + name, "PluginManager::unregisterPlugin"));
    }

    plugins_.remove(name);

    // 如果卸载的是激活插件，清空激活状态
    if (activePluginName_ == name) {
        activePluginName_.clear();
    }

    if (emitSignals) {
        emit pluginUnregistered(name);
    }
    return Result<void>::ok();
}

IDriverPlugin* PluginManager::getPlugin(const QString& name) const {
    auto it = plugins_.find(name);
    if (it == plugins_.end()) {
        return nullptr;
    }
    return it->plugin.get();
}

QString PluginManager::getPluginPath(const QString& name) const {
    auto it = plugins_.find(name);
    if (it == plugins_.end()) {
        return {};
    }
    return it->libPath;
}

IDriverPlugin* PluginManager::activePlugin() const {
    if (activePluginName_.isEmpty()) {
        return nullptr;
    }
    return getPlugin(activePluginName_);
}

QString PluginManager::activePluginName() const {
    return activePluginName_;
}

Result<void> PluginManager::setActivePlugin(const QString& name, bool emitSignals) {
    if (!plugins_.contains(name)) {
        return Result<void>::err(
            Error(Error::NotFound, "插件未找到：" + name, "PluginManager::setActivePlugin"));
    }

    activePluginName_ = name;
    if (emitSignals) {
        emit activePluginChanged(name);
    }
    return Result<void>::ok();
}

QStringList PluginManager::listPlugins() const {
    return plugins_.keys();
}

}  // namespace wekey
