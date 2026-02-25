/**
 * @file LogModel.h
 * @brief 日志数据模型
 *
 * 供 GUI 日志查看器使用的 QAbstractTableModel 实现
 */

#pragma once

#include <QAbstractTableModel>
#include <QList>

#include "log/Logger.h"

namespace wekey {

/**
 * @brief 日志数据模型
 *
 * 将日志条目以表格形式呈现，支持过滤和搜索功能
 */
class LogModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /**
     * @brief 列索引枚举
     */
    enum class Column { Timestamp = 0, Level, Source, Message, ColumnCount };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit LogModel(QObject* parent = nullptr);

    // QAbstractTableModel 接口
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief 添加日志条目
     * @param entry 日志条目
     */
    void addEntry(const LogEntry& entry);

    /**
     * @brief 清空所有日志
     */
    void clear();

    /**
     * @brief 设置最大条目数
     * @param max 最大条目数，超出时自动删除旧条目
     */
    void setMaxEntries(int max);

    /**
     * @brief 设置日志级别过滤器
     * @param level 最低显示级别
     */
    void setFilterLevel(LogLevel level);

    /**
     * @brief 设置搜索文本过滤器
     * @param text 搜索文本，空字符串表示不过滤
     */
    void setSearchText(const QString& text);

    /**
     * @brief 连接到 Logger 单例
     *
     * 自动接收 Logger 发出的日志
     */
    void connectToLogger();

    /**
     * @brief 获取指定行的日志条目
     * @param row 行号
     * @return 日志条目的常引用
     */
    const LogEntry& entry(int row) const;

private:
    /**
     * @brief 重建过滤后的索引列表
     */
    void rebuildFilteredIndices();

    /**
     * @brief 检查条目是否通过过滤
     * @param entry 日志条目
     * @return 是否显示
     */
    bool passesFilter(const LogEntry& entry) const;

    QList<LogEntry> entries_;
    QList<int> filteredIndices_;
    int maxEntries_{10000};
    LogLevel filterLevel_{LogLevel::Debug};
    QString searchText_;
};

}  // namespace wekey
