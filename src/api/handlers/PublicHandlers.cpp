/**
 * @file PublicHandlers.cpp
 * @brief 公共接口处理器实现 (M4.4.4I)
 */

#include "PublicHandlers.h"

#include <QJsonObject>

#include "config/Defaults.h"

namespace wekey {
namespace api {

PublicHandlers::PublicHandlers(QObject* parent) : QObject(parent) {}

HttpResponse PublicHandlers::handleHealth(const HttpRequest& /*request*/) {
    QJsonObject data;
    data["status"] = "ok";
    data["version"] = defaults::CONFIG_VERSION;

    HttpResponse resp;
    resp.setSuccess(data);
    return resp;
}

HttpResponse PublicHandlers::handleExit(const HttpRequest& /*request*/) {
    HttpResponse resp;
    resp.setSuccess();

    emit exitRequested();
    return resp;
}

}  // namespace api
}  // namespace wekey
