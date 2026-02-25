/**
 * @file Config.cpp
 * @brief 配置管理单例类实现
 */

#include "config/Config.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

#include "config/Defaults.h"

namespace wekey {

Config& Config::instance() {
    static Config instance;
    return instance;
}

Config::Config() : QObject(nullptr), systrayDisabled_(false) {
    initDefaults();
}

void Config::initDefaults() {
    listenPort_ = defaults::LISTEN_PORT;
    logLevel_ = defaults::LOG_LEVEL;
    errorMode_ = defaults::ERROR_MODE_SIMPLE;
    systrayDisabled_ = false;
    activedModName_.clear();
    logPath_ = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    modPaths_ = QJsonObject();

    defaultAppName_ = defaults::APP_NAME;
    defaultContainerName_ = defaults::CONTAINER_NAME;
    defaultCommonName_ = defaults::COMMON_NAME;
    defaultOrganization_ = defaults::ORGANIZATION;
    defaultUnit_ = defaults::UNIT;
    defaultRole_ = defaults::ROLE_USER;
}

QString Config::configFilePath() const {
    QString homeDir = QDir::homePath();
    return homeDir + "/" + defaults::CONFIG_FILENAME;
}

// ==================== 基本配置 Getter/Setter ====================

QString Config::listenPort() const {
    return listenPort_;
}

void Config::setListenPort(const QString& port) {
    listenPort_ = port;
}

QString Config::logLevel() const {
    return logLevel_;
}

void Config::setLogLevel(const QString& level) {
    logLevel_ = level;
}

QString Config::errorMode() const {
    return errorMode_;
}

void Config::setErrorMode(const QString& mode) {
    errorMode_ = mode;
}

bool Config::systrayDisabled() const {
    return systrayDisabled_;
}

void Config::setSystrayDisabled(bool disabled) {
    systrayDisabled_ = disabled;
}

QString Config::activedModName() const {
    return activedModName_;
}

void Config::setActivedModName(const QString& name) {
    activedModName_ = name;
}

QString Config::logPath() const {
    return logPath_;
}

void Config::setLogPath(const QString& path) {
    logPath_ = path;
}

QString Config::version() const {
    return defaults::CONFIG_VERSION;
}

// ==================== 模块路径管理 ====================

QJsonObject Config::modPaths() const {
    return modPaths_;
}

void Config::setModPath(const QString& name, const QString& path) {
    modPaths_[name] = path;
}

void Config::removeModPath(const QString& name) {
    modPaths_.remove(name);
}

// ==================== 默认应用配置 ====================

QString Config::defaultAppName() const {
    return defaultAppName_;
}

QString Config::defaultContainerName() const {
    return defaultContainerName_;
}

QString Config::defaultCommonName() const {
    return defaultCommonName_;
}

QString Config::defaultOrganization() const {
    return defaultOrganization_;
}

QString Config::defaultUnit() const {
    return defaultUnit_;
}

QString Config::defaultRole() const {
    return defaultRole_;
}

int Config::defaultRandomLength() const {
    return defaultRandomLength_;
}

void Config::setDefault(const QString& key, const QString& value) {
    if (key == "appName") {
        defaultAppName_ = value;
    } else if (key == "containerName") {
        defaultContainerName_ = value;
    } else if (key == "commonName") {
        defaultCommonName_ = value;
    } else if (key == "organization") {
        defaultOrganization_ = value;
    } else if (key == "unit") {
        defaultUnit_ = value;
    } else if (key == "role") {
        defaultRole_ = value;
    } else if (key == "randomLength") {
        defaultRandomLength_ = value.toInt();
    }
}

// ==================== 文件操作 ====================

bool Config::load() {
    QString filePath = configFilePath();
    QFile file(filePath);

    if (!file.exists()) {
        // 配置文件不存在，使用默认值
        return true;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }

    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();

    // 读取基本配置
    if (root.contains("listenPort")) {
        listenPort_ = root["listenPort"].toString();
    }
    if (root.contains("logLevel")) {
        logLevel_ = root["logLevel"].toString();
    }
    if (root.contains("errorMode")) {
        errorMode_ = root["errorMode"].toString();
    }
    if (root.contains("systrayDisabled")) {
        systrayDisabled_ = root["systrayDisabled"].toBool();
    }
    if (root.contains("activedModName")) {
        activedModName_ = root["activedModName"].toString();
    }
    if (root.contains("logPath")) {
        logPath_ = root["logPath"].toString();
    }

    // 读取模块路径
    if (root.contains("modPaths") && root["modPaths"].isObject()) {
        modPaths_ = root["modPaths"].toObject();
    }

    // 读取默认应用配置
    if (root.contains("defaults") && root["defaults"].isObject()) {
        QJsonObject defaults = root["defaults"].toObject();
        if (defaults.contains("appName")) {
            defaultAppName_ = defaults["appName"].toString();
        }
        if (defaults.contains("containerName")) {
            defaultContainerName_ = defaults["containerName"].toString();
        }
        if (defaults.contains("commonName")) {
            defaultCommonName_ = defaults["commonName"].toString();
        }
        if (defaults.contains("organization")) {
            defaultOrganization_ = defaults["organization"].toString();
        }
        if (defaults.contains("unit")) {
            defaultUnit_ = defaults["unit"].toString();
        }
        if (defaults.contains("role")) {
            defaultRole_ = defaults["role"].toString();
        }
    }

    return true;
}

bool Config::save() {
    QString filePath = configFilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QJsonObject root;

    // 写入版本
    root["version"] = defaults::CONFIG_VERSION;

    // 写入基本配置
    root["listenPort"] = listenPort_;
    root["logLevel"] = logLevel_;
    root["errorMode"] = errorMode_;
    root["systrayDisabled"] = systrayDisabled_;
    root["activedModName"] = activedModName_;
    root["logPath"] = logPath_;

    // 写入模块路径
    root["modPaths"] = modPaths_;

    // 写入默认应用配置
    QJsonObject defaultsObj;
    defaultsObj["appName"] = defaultAppName_;
    defaultsObj["containerName"] = defaultContainerName_;
    defaultsObj["commonName"] = defaultCommonName_;
    defaultsObj["organization"] = defaultOrganization_;
    defaultsObj["unit"] = defaultUnit_;
    defaultsObj["role"] = defaultRole_;
    root["defaults"] = defaultsObj;

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    emit configChanged();
    return true;
}

void Config::reset() {
    initDefaults();
    emit configChanged();
}

}  // namespace wekey
