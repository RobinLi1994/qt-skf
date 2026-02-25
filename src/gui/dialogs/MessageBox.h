/**
 * @file MessageBox.h
 * @brief 自定义消息框 (M5.10)
 *
 * 支持简洁/详细两种错误提示模式
 */

#pragma once

#include <QString>
#include <QWidget>

#include <ElaMessageBar.h>

#include "common/Error.h"

namespace wekey {

class MessageBox {
public:
    static void info(QWidget* parent, const QString& title, const QString& message);
    static void error(QWidget* parent, const QString& title, const Error& err);
    static void error(QWidget* parent, const QString& title, const QString& message);
};

}  // namespace wekey
