/**
 * @file Defaults.h
 * @brief 默认值常量定义
 */

#pragma once

#include <QString>

namespace wekey {
namespace defaults {

// 配置文件
constexpr const char* CONFIG_FILENAME = ".wekeytool.json";
constexpr const char* CONFIG_VERSION = "1.0.0";

// 网络
constexpr const char* LISTEN_PORT = ":9001";

// 日志
constexpr const char* LOG_LEVEL = "info";

// 错误模式
constexpr const char* ERROR_MODE_SIMPLE = "simple";
constexpr const char* ERROR_MODE_DETAILED = "detailed";

// 默认应用配置
constexpr const char* APP_NAME = "TAGM";
constexpr const char* CONTAINER_NAME = "TrustAsia";
constexpr const char* COMMON_NAME = "TrustAsia";
constexpr const char* ORGANIZATION = "TrustAsia Technologies, Inc.";
constexpr const char* UNIT = "GMCA";
constexpr const char* ROLE_USER = "user";
constexpr const char* ROLE_ADMIN = "admin";

// PIN 重试次数
constexpr int ADMIN_PIN_RETRY_COUNT = 6;
constexpr int USER_PIN_RETRY_COUNT = 6;

}  // namespace defaults
}  // namespace wekey
