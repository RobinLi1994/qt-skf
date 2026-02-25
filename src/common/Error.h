/**
 * @file Error.h
 * @brief 错误码定义
 *
 * 遵循项目宪法 §3.1：所有错误都必须被显式处理，错误必须携带上下文信息。
 */

#pragma once

#include <QString>

#include <cstdint>

namespace wekey {

/**
 * @brief 错误类
 *
 * 统一的错误表示，支持应用层错误码和 SKF 硬件错误码映射。
 */
class Error {
public:
    /**
     * @brief 错误码枚举
     */
    enum Code : uint32_t {
        // 应用层错误码 (0x00 - 0xFF)
        Success = 0x00,
        Fail = 0x01,
        InvalidParam = 0x03,
        NoActiveModule = 0x04,
        NotLoggedIn = 0x09,
        NotAuthorized = 0x0B,
        PortInUse = 0x10,
        PluginLoadFailed = 0x11,
        AlreadyExists = 0x12,
        NotFound = 0x13,

        // SKF 错误码 (0x0A000000+)
        SkfOk = 0x00000000,
        SkfFail = 0x0A000001,
        SkfUnknown = 0x0A000002,
        SkfNotSupported = 0x0A000003,
        SkfFileError = 0x0A000004,
        SkfInvalidHandle = 0x0A000005,
        SkfInvalidParam = 0x0A000006,
        SkfReadFileError = 0x0A000007,
        SkfWriteFileError = 0x0A000008,
        SkfNameLenError = 0x0A000009,
        SkfKeyUsageError = 0x0A00000A,
        SkfModulusLenError = 0x0A00000B,
        SkfNotInitialized = 0x0A00000C,
        SkfObjConflict = 0x0A00000D,
        SkfDeviceRemoved = 0x0A000023,
        SkfPinIncorrect = 0x0A000024,
        SkfPinLocked = 0x0A000025,
        SkfUserNotLogin = 0x0A00002D,
        SkfAppNotExists = 0x0A00002E,
    };

    /**
     * @brief 默认构造函数
     *
     * 创建一个表示成功的 Error 对象
     */
    Error() = default;

    /**
     * @brief 构造函数
     * @param code 错误码
     * @param message 错误消息
     * @param context 上下文信息（如函数名、设备名）
     */
    explicit Error(Code code, QString message = {}, QString context = {})
        : code_(code), message_(std::move(message)), context_(std::move(context)) {}

    /**
     * @brief 获取错误码
     */
    [[nodiscard]] Code code() const { return code_; }

    /**
     * @brief 获取错误消息
     */
    [[nodiscard]] const QString& message() const { return message_; }

    /**
     * @brief 获取上下文信息
     */
    [[nodiscard]] const QString& context() const { return context_; }

    /**
     * @brief 获取用户友好的错误描述
     * @return 友好的错误描述
     */
    [[nodiscard]] QString friendlyMessage() const;

    /**
     * @brief 转换为字符串
     * @param detailed 是否包含详细信息（错误码、上下文）
     * @return 错误字符串
     */
    [[nodiscard]] QString toString(bool detailed = false) const;

    /**
     * @brief 从 SKF 返回值构造错误
     * @param skfResult SKF 函数返回值
     * @param function 调用的函数名（可选）
     * @return Error 对象
     */
    static Error fromSkf(uint32_t skfResult, const QString& function = {});

    /**
     * @brief 判断是否为成功状态
     */
    [[nodiscard]] bool isSuccess() const { return code_ == Success || code_ == SkfOk; }

private:
    Code code_ = Success;
    QString message_;
    QString context_;
};

}  // namespace wekey
