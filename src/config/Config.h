/**
 * @file Config.h
 * @brief 配置管理单例类
 *
 * 负责读取、保存和管理应用程序配置
 * 配置文件格式为 JSON，存储在 ~/.wekeytool.json
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>

namespace wekey {

/**
 * @brief 配置管理单例类
 *
 * 提供线程安全的配置访问，支持 JSON 格式的配置文件读写
 */
class Config : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return Config 单例引用
     */
    static Config& instance();

    // 禁用拷贝和移动
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // ==================== 基本配置 ====================

    /**
     * @brief 获取监听端口
     * @return 端口字符串，如 ":9001"
     */
    QString listenPort() const;

    /**
     * @brief 设置监听端口
     * @param port 端口字符串
     */
    void setListenPort(const QString& port);

    /**
     * @brief 获取日志级别
     * @return 日志级别字符串
     */
    QString logLevel() const;

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLogLevel(const QString& level);

    /**
     * @brief 获取错误模式
     * @return "simple" 或 "detailed"
     */
    QString errorMode() const;

    /**
     * @brief 设置错误模式
     * @param mode 错误模式
     */
    void setErrorMode(const QString& mode);

    /**
     * @brief 获取系统托盘是否禁用
     * @return true 表示禁用
     */
    bool systrayDisabled() const;

    /**
     * @brief 设置系统托盘禁用状态
     * @param disabled 是否禁用
     */
    void setSystrayDisabled(bool disabled);

    /**
     * @brief 获取当前激活的模块名称
     * @return 模块名称
     */
    QString activedModName() const;

    /**
     * @brief 设置当前激活的模块名称
     * @param name 模块名称
     */
    void setActivedModName(const QString& name);

    /**
     * @brief 获取日志路径
     * @return 日志存储路径
     */
    QString logPath() const;

    /**
     * @brief 设置日志路径
     * @param path 日志存储路径
     */
    void setLogPath(const QString& path);

    /**
     * @brief 获取配置版本
     * @return 配置版本字符串
     */
    QString version() const;

    // ==================== 模块路径管理 ====================

    /**
     * @brief 获取所有模块路径
     * @return 模块名称到路径的映射
     */
    QJsonObject modPaths() const;

    /**
     * @brief 设置模块路径
     * @param name 模块名称
     * @param path 模块路径
     */
    void setModPath(const QString& name, const QString& path);

    /**
     * @brief 删除模块路径
     * @param name 模块名称
     */
    void removeModPath(const QString& name);

    // ==================== 默认应用配置 ====================

    /**
     * @brief 获取默认应用名称
     */
    QString defaultAppName() const;

    /**
     * @brief 获取默认容器名称
     */
    QString defaultContainerName() const;

    /**
     * @brief 获取默认通用名称
     */
    QString defaultCommonName() const;

    /**
     * @brief 获取默认组织
     */
    QString defaultOrganization() const;

    /**
     * @brief 获取默认单位
     */
    QString defaultUnit() const;

    /**
     * @brief 获取默认角色
     */
    QString defaultRole() const;

    /**
     * @brief 获取默认随机数长度
     */
    int defaultRandomLength() const;

    /**
     * @brief 设置默认值
     * @param key 键名 (appName, containerName, commonName, organization, unit, role, randomLength)
     * @param value 值
     */
    void setDefault(const QString& key, const QString& value);

    // ==================== 文件操作 ====================

    /**
     * @brief 从文件加载配置
     * @return 成功返回 true
     */
    bool load();

    /**
     * @brief 保存配置到文件
     * @return 成功返回 true
     */
    bool save();

    /**
     * @brief 重置为默认配置
     */
    void reset();

signals:
    /**
     * @brief 配置变更信号
     */
    void configChanged();

private:
    Config();
    ~Config() override = default;

    /**
     * @brief 获取配置文件路径
     * @return 配置文件完整路径
     */
    QString configFilePath() const;

    /**
     * @brief 初始化默认值
     */
    void initDefaults();

    // 基本配置
    QString listenPort_;
    QString logLevel_;
    QString errorMode_;
    bool systrayDisabled_;
    QString activedModName_;
    QString logPath_;

    // 模块路径
    QJsonObject modPaths_;

    // 默认应用配置
    QString defaultAppName_;
    QString defaultContainerName_;
    QString defaultCommonName_;
    QString defaultOrganization_;
    QString defaultUnit_;
    QString defaultRole_;
    int defaultRandomLength_ = 16;  // 默认随机数长度
};

}  // namespace wekey
