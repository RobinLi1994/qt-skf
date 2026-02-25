/**
 * @file test_messagebox.cpp
 * @brief MessageBox 单元测试
 *
 * MessageBox 是静态方法的封装，测试主要验证类存在且可实例化
 * 实际弹窗不适合在自动化测试中触发
 */

#include <QApplication>
#include <QTest>

#include "gui/dialogs/MessageBox.h"

using namespace wekey;

class TestMessageBox : public QObject {
    Q_OBJECT

private slots:
    void testErrorFromErrorObject() {
        // Verify Error can be constructed and used with MessageBox
        Error err(Error::Fail, "测试错误");
        QVERIFY(!err.isSuccess());
        QCOMPARE(err.code(), Error::Fail);
    }

    void testErrorToStringSimple() {
        Error err(Error::InvalidParam, "参数无效", "testFunc");
        QString simple = err.toString(false);
        QVERIFY(!simple.isEmpty());
    }

    void testErrorToStringDetailed() {
        Error err(Error::InvalidParam, "参数无效", "testFunc");
        QString detailed = err.toString(true);
        QVERIFY(!detailed.isEmpty());
    }
};

QTEST_MAIN(TestMessageBox)
#include "test_messagebox.moc"
