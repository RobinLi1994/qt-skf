/**
 * @file LogModel.cpp
 * @brief 日志数据模型实现
 */

#include "log/LogModel.h"

namespace wekey {

LogModel::LogModel(QObject* parent) : QAbstractTableModel(parent), filterLevel_(LogLevel::Debug) {}

int LogModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return filteredIndices_.size();
}

int LogModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(Column::ColumnCount);
}

QVariant LogModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= filteredIndices_.size()) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    int realIndex = filteredIndices_.at(index.row());
    const LogEntry& entry = entries_.at(realIndex);

    switch (static_cast<Column>(index.column())) {
        case Column::Timestamp:
            return entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
        case Column::Level:
            return logLevelToString(entry.level);
        case Column::Source: {
            // source 有值时直接用；为空时尝试从消息 [xxx] 前缀解析
            if (!entry.source.isEmpty()) return entry.source;
            if (entry.message.startsWith('[')) {
                int end = entry.message.indexOf(']');
                if (end > 1) return entry.message.mid(1, end - 1);
            }
            return {};
        }
        case Column::Message: {
            // source 为空且消息有 [xxx] 前缀时，消息列去掉该前缀
            if (entry.source.isEmpty() && entry.message.startsWith('[')) {
                int end = entry.message.indexOf(']');
                if (end > 1) {
                    QString rest = entry.message.mid(end + 1);
                    if (rest.startsWith(' ')) rest = rest.mid(1);
                    return rest;
                }
            }
            return entry.message;
        }
        default:
            return {};
    }
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (static_cast<Column>(section)) {
        case Column::Timestamp:
            return tr("时间");
        case Column::Level:
            return tr("级别");
        case Column::Source:
            return tr("来源");
        case Column::Message:
            return tr("消息");
        default:
            return {};
    }
}

void LogModel::addEntry(const LogEntry& entry) {
    // 检查是否超过最大条目数
    while (entries_.size() >= maxEntries_) {
        // 删除最旧的条目
        entries_.removeFirst();
    }

    // 添加新条目
    entries_.append(entry);

    // 检查是否通过过滤
    if (passesFilter(entry)) {
        int newRow = filteredIndices_.size();
        beginInsertRows({}, newRow, newRow);
        filteredIndices_.append(entries_.size() - 1);
        endInsertRows();
    }

    // 如果删除了旧条目，需要更新过滤索引
    if (entries_.size() == maxEntries_) {
        rebuildFilteredIndices();
    }
}

void LogModel::clear() {
    beginResetModel();
    entries_.clear();
    filteredIndices_.clear();
    endResetModel();
}

void LogModel::setMaxEntries(int max) {
    maxEntries_ = max;

    // 如果当前条目超过新限制，删除旧条目
    while (entries_.size() > maxEntries_) {
        entries_.removeFirst();
    }

    rebuildFilteredIndices();
}

void LogModel::setFilterLevel(LogLevel level) {
    if (filterLevel_ == level) {
        return;
    }

    filterLevel_ = level;
    rebuildFilteredIndices();
}

void LogModel::setSearchText(const QString& text) {
    if (searchText_ == text) {
        return;
    }

    searchText_ = text;
    rebuildFilteredIndices();
}

void LogModel::connectToLogger() {
    connect(&Logger::instance(), &Logger::logAdded, this, &LogModel::addEntry);
}

const LogEntry& LogModel::entry(int row) const {
    int realIndex = filteredIndices_.at(row);
    return entries_.at(realIndex);
}

void LogModel::rebuildFilteredIndices() {
    beginResetModel();
    filteredIndices_.clear();

    for (int i = 0; i < entries_.size(); ++i) {
        if (passesFilter(entries_.at(i))) {
            filteredIndices_.append(i);
        }
    }

    endResetModel();
}

bool LogModel::passesFilter(const LogEntry& entry) const {
    // 级别过滤
    if (entry.level < filterLevel_) {
        return false;
    }

    // 搜索文本过滤
    if (!searchText_.isEmpty()) {
        bool found = entry.message.contains(searchText_, Qt::CaseInsensitive) ||
                     entry.source.contains(searchText_, Qt::CaseInsensitive);
        if (!found) {
            return false;
        }
    }

    return true;
}

}  // namespace wekey
