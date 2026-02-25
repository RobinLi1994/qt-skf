/**
 * @file MessageBox.cpp
 * @brief 自定义消息框实现
 */

#include "MessageBox.h"

#include <ElaMessageBar.h>

#include "config/Config.h"

namespace wekey {

void MessageBox::info(QWidget* parent, const QString& title, const QString& message) {
    ElaMessageBar::success(ElaMessageBarType::TopRight, title, message, 3000, parent);
}

void MessageBox::error(QWidget* parent, const QString& title, const Error& err) {
    bool detailed = Config::instance().errorMode() == "detailed";
    QString message = err.toString(detailed);
    ElaMessageBar::error(ElaMessageBarType::TopRight, title, message, 5000, parent);
}

void MessageBox::error(QWidget* parent, const QString& title, const QString& message) {
    ElaMessageBar::error(ElaMessageBarType::TopRight, title, message, 5000, parent);
}

}  // namespace wekey
