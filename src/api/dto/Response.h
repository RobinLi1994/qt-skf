/**
 * @file Response.h
 * @brief API 响应 DTO 定义 (M4.1.3I)
 *
 * 定义标准 API 响应格式和转换辅助函数
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QString>

#include "common/Error.h"
#include "common/Result.h"
#include "plugin/interface/PluginTypes.h"

namespace wekey {
namespace api {

/**
 * @brief API 响应模板类
 *
 * 标准响应格式：
 * {
 *   "code": 0,
 *   "message": "success",
 *   "data": { ... }
 * }
 *
 * @tparam T 数据类型
 */
template <typename T>
class ApiResponse {
public:
    /**
     * @brief 创建成功响应
     * @param data 响应数据
     * @return ApiResponse 对象
     */
    static ApiResponse success(const T& data) {
        ApiResponse resp;
        resp.success_ = true;
        resp.code_ = 0;
        resp.message_ = "success";
        resp.data_ = data;
        return resp;
    }

    /**
     * @brief 创建错误响应
     * @param error 错误对象
     * @return ApiResponse 对象
     */
    static ApiResponse error(const Error& error) {
        ApiResponse resp;
        resp.success_ = false;
        resp.code_ = static_cast<int>(error.code());
        resp.message_ = error.friendlyMessage();
        return resp;
    }

    /**
     * @brief 从 Result<T> 创建响应
     * @param result Result 对象
     * @return ApiResponse 对象
     */
    static ApiResponse fromResult(const Result<T>& result) {
        if (result.isOk()) {
            return success(result.value());
        } else {
            return error(result.error());
        }
    }

    /**
     * @brief 判断是否成功
     */
    bool isSuccess() const { return success_; }

    /**
     * @brief 获取错误码
     */
    int code() const { return code_; }

    /**
     * @brief 获取消息
     */
    QString message() const { return message_; }

    /**
     * @brief 获取数据
     * @warning 仅在 isSuccess() 为 true 时有效
     */
    const T& data() const { return data_; }

    /**
     * @brief 转换为 JSON 对象
     * @param dataConverter 数据转换函数，将 T 转换为 QJsonValue
     * @return JSON 对象
     */
    template <typename Converter>
    QJsonObject toJson(Converter dataConverter) const {
        QJsonObject obj;
        obj["code"] = code_;
        obj["message"] = message_;

        if (success_) {
            obj["data"] = dataConverter(data_);
        } else {
            obj["data"] = QJsonValue::Null;
        }

        return obj;
    }

private:
    bool success_ = false;
    int code_ = 0;
    QString message_;
    T data_;
};

/**
 * @brief ApiResponse<void> 特化
 *
 * 用于不返回数据的操作
 */
template <>
class ApiResponse<void> {
public:
    /**
     * @brief 创建成功响应
     */
    static ApiResponse success() {
        ApiResponse resp;
        resp.success_ = true;
        resp.code_ = 0;
        resp.message_ = "success";
        return resp;
    }

    /**
     * @brief 创建错误响应
     */
    static ApiResponse error(const Error& error) {
        ApiResponse resp;
        resp.success_ = false;
        resp.code_ = static_cast<int>(error.code());
        resp.message_ = error.friendlyMessage();
        return resp;
    }

    /**
     * @brief 从 Result<void> 创建响应
     */
    static ApiResponse fromResult(const Result<void>& result) {
        if (result.isOk()) {
            return success();
        } else {
            return error(result.error());
        }
    }

    /**
     * @brief 判断是否成功
     */
    bool isSuccess() const { return success_; }

    /**
     * @brief 获取错误码
     */
    int code() const { return code_; }

    /**
     * @brief 获取消息
     */
    QString message() const { return message_; }

    /**
     * @brief 转换为 JSON 对象
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["code"] = code_;
        obj["message"] = message_;
        obj["data"] = QJsonValue::Null;
        return obj;
    }

private:
    bool success_ = false;
    int code_ = 0;
    QString message_;
};

// ==============================================================================
// 转换辅助函数
// ==============================================================================

/**
 * @brief 将 DeviceInfo 转换为 JSON 对象
 */
QJsonObject deviceInfoToJson(const DeviceInfo& info);

/**
 * @brief 将 DeviceInfo 列表转换为 JSON 数组
 */
QJsonArray deviceInfoListToJson(const QList<DeviceInfo>& devices);

/**
 * @brief 将 AppInfo 转换为 JSON 对象
 */
QJsonObject appInfoToJson(const AppInfo& info);

/**
 * @brief 将 AppInfo 列表转换为 JSON 数组
 */
QJsonArray appInfoListToJson(const QList<AppInfo>& apps);

/**
 * @brief 将 ContainerInfo 转换为 JSON 对象
 */
QJsonObject containerInfoToJson(const ContainerInfo& info);

/**
 * @brief 将 ContainerInfo 列表转换为 JSON 数组
 */
QJsonArray containerInfoListToJson(const QList<ContainerInfo>& containers);

/**
 * @brief 将 CertInfo 转换为 JSON 对象
 */
QJsonObject certInfoToJson(const CertInfo& info);

/**
 * @brief 将 CertInfo 列表转换为 JSON 数组
 */
QJsonArray certInfoListToJson(const QList<CertInfo>& certs);

}  // namespace api
}  // namespace wekey
