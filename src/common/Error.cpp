/**
 * @file Error.cpp
 * @brief 错误码实现
 */

#include "Error.h"

#include <QHash>

namespace wekey {

namespace {

/**
 * @brief 获取错误码对应的友好消息
 */
const QHash<Error::Code, QString>& getFriendlyMessages() {
    static const QHash<Error::Code, QString> messages = {
        // 应用层错误
        {Error::Success, "操作成功"},
        {Error::Fail, "操作失败"},
        {Error::InvalidParam, "参数无效"},
        {Error::NoActiveModule, "未激活驱动模块"},
        {Error::NotLoggedIn, "未登录"},
        {Error::NotAuthorized, "未授权"},
        {Error::PortInUse, "端口已被占用"},
        {Error::PluginLoadFailed, "插件加载失败"},
        {Error::AlreadyExists, "已存在"},
        {Error::NotFound, "未找到"},

        // SKF 错误
        {Error::SkfOk, "操作成功"},
        {Error::SkfFail, "操作失败"},
        {Error::SkfUnknown, "未知错误"},
        {Error::SkfNotSupported, "不支持此操作"},
        {Error::SkfFileError, "文件操作错误"},
        {Error::SkfInvalidHandle, "无效句柄"},
        {Error::SkfInvalidParam, "参数无效"},
        {Error::SkfReadFileError, "读取文件错误"},
        {Error::SkfWriteFileError, "写入文件错误"},
        {Error::SkfNameLenError, "名称长度错误"},
        {Error::SkfKeyUsageError, "密钥用途错误"},
        {Error::SkfModulusLenError, "模数长度错误"},
        {Error::SkfNotInitialized, "未初始化"},
        {Error::SkfObjConflict, "对象冲突"},
        {Error::SkfDeviceRemoved, "设备已移除"},
        {Error::SkfPinIncorrect, "PIN 码错误"},
        {Error::SkfPinLocked, "PIN 码已锁定"},
        {Error::SkfUserNotLogin, "用户未登录"},
        {Error::SkfAppNotExists, "应用不存在"},
    };
    return messages;
}

}  // namespace

QString Error::friendlyMessage() const {
    // 如果有自定义消息，优先使用
    if (!message_.isEmpty()) {
        return message_;
    }

    const auto& messages = getFriendlyMessages();
    auto it = messages.find(code_);
    if (it != messages.end()) {
        return it.value();
    }

    return "未知错误";
}

QString Error::toString(bool detailed) const {
    QString result = friendlyMessage();

    if (detailed) {
        result += QString("\n错误码: 0x%1").arg(static_cast<uint32_t>(code_), 8, 16, QChar('0'));

        if (!context_.isEmpty()) {
            result += QString("\n上下文: %1").arg(context_);
        }

        if (!message_.isEmpty() && message_ != friendlyMessage()) {
            result += QString("\n详情: %1").arg(message_);
        }
    }

    return result;
}

Error Error::fromSkf(uint32_t skfResult, const QString& function) {
    return Error(static_cast<Code>(skfResult), {}, function);
}

}  // namespace wekey
