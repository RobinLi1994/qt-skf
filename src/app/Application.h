/**
 * @file Application.h
 * @brief 应用程序主类
 *
 * 继承 QApplication，提供单例检测、配置加载、日志初始化等功能
 */

#pragma once

#include <QApplication>
#include <QLockFile>
#include <QSharedMemory>
#include <memory>

namespace wekey {

/**
 * @brief 应用程序主类
 *
 * 负责应用程序生命周期管理，包括：
 * - 单例实例检测
 * - 配置文件加载
 * - 日志系统初始化
 */
class Application : public QApplication {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param argc 命令行参数个数
     * @param argv 命令行参数数组
     */
    Application(int& argc, char** argv);

    /**
     * @brief 析构函数
     */
    ~Application() override;

    /**
     * @brief 初始化应用程序
     *
     * 执行以下操作：
     * 1. 检测单例实例
     * 2. 加载配置文件
     * 3. 初始化日志系统
     *
     * @return 初始化成功返回 true
     */
    bool initialize();

    /**
     * @brief 关闭应用程序
     *
     * 执行清理操作
     */
    void shutdown();

    /**
     * @brief 检查是否为主实例
     * @return 是主实例返回 true
     */
    bool isPrimaryInstance() const;

signals:
    /**
     * @brief 另一个实例尝试启动时发出
     */
    void secondInstanceStarted();

private:
    /**
     * @brief 尝试获取单例锁
     * @return 成功获取锁返回 true
     */
    bool acquireSingleInstanceLock();

    /**
     * @brief 加载配置
     * @return 成功返回 true
     */
    bool loadConfig();

    /**
     * @brief 初始化日志系统
     */
    void initLogging();

    /**
     * @brief 从配置恢复已注册的插件
     */
    void loadPlugins();

    /**
     * @brief 注册内置 SKF 模块
     * @return 模块路径，未找到返回空字符串
     */
    QString registerBuiltinModule();

    std::unique_ptr<QLockFile> lockFile_;
    bool isPrimary_{false};
};

}  // namespace wekey
