/**
 * @file FileService.h
 * @brief 文件与随机数服务
 *
 * 封装文件操作和随机数生成，委托给激活的驱动插件
 */

#pragma once

#include <QObject>

#include "common/Result.h"

namespace wekey {

class FileService : public QObject {
    Q_OBJECT

public:
    static FileService& instance();

    FileService(const FileService&) = delete;
    FileService& operator=(const FileService&) = delete;

    Result<QStringList> enumFiles(const QString& devName, const QString& appName);
    Result<QByteArray> readFile(const QString& devName, const QString& appName, const QString& fileName);
    Result<void> writeFile(const QString& devName, const QString& appName, const QString& fileName,
                           const QByteArray& data, int readRights = 0xFF, int writeRights = 0x01);
    Result<void> deleteFile(const QString& devName, const QString& appName, const QString& fileName);
    Result<QByteArray> generateRandom(const QString& devName, int count);

private:
    FileService();
    ~FileService() override = default;
};

}  // namespace wekey
