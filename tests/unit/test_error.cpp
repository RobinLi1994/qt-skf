/**
 * @file test_error.cpp
 * @brief Error 类单元测试
 *
 * 测试用例 M1.2.2T
 */

#include <QtTest>

#include "common/Error.h"

using namespace wekey;

class TestError : public QObject {
    Q_OBJECT

private slots:
    /**
     * @brief 测试默认构造
     * 默认构造的 Error 应该是 Success
     */
    void testDefaultConstruction() {
        Error error;
        QCOMPARE(error.code(), Error::Success);
        QVERIFY(error.message().isEmpty());
        QVERIFY(error.context().isEmpty());
    }

    /**
     * @brief 测试指定错误码构造
     */
    void testCodeConstruction() {
        Error error(Error::Fail);
        QCOMPARE(error.code(), Error::Fail);
    }

    /**
     * @brief 测试错误码 + 消息构造
     */
    void testWithMessage() {
        Error error(Error::InvalidParam, "参数不能为空");
        QCOMPARE(error.code(), Error::InvalidParam);
        QCOMPARE(error.message(), QString("参数不能为空"));
    }

    /**
     * @brief 测试错误码 + 消息 + 上下文构造
     */
    void testWithContext() {
        Error error(Error::SkfPinIncorrect, "PIN 错误", "SKF_VerifyPIN");
        QCOMPARE(error.code(), Error::SkfPinIncorrect);
        QCOMPARE(error.message(), QString("PIN 错误"));
        QCOMPARE(error.context(), QString("SKF_VerifyPIN"));
    }

    /**
     * @brief 测试从 SKF 错误码构造
     */
    void testFromSkf_data() {
        QTest::addColumn<uint32_t>("skfCode");
        QTest::addColumn<Error::Code>("expectedCode");

        QTest::newRow("SAR_OK") << 0x00000000u << Error::SkfOk;
        QTest::newRow("SAR_FAIL") << 0x0A000001u << Error::SkfFail;
        QTest::newRow("SAR_PIN_INCORRECT") << 0x0A000024u << Error::SkfPinIncorrect;
        QTest::newRow("SAR_PIN_LOCKED") << 0x0A000025u << Error::SkfPinLocked;
        QTest::newRow("SAR_DEVICE_REMOVED") << 0x0A000023u << Error::SkfDeviceRemoved;
    }

    void testFromSkf() {
        QFETCH(uint32_t, skfCode);
        QFETCH(Error::Code, expectedCode);

        Error error = Error::fromSkf(skfCode, "TestFunction");
        QCOMPARE(error.code(), expectedCode);
        QCOMPARE(error.context(), QString("TestFunction"));
    }

    /**
     * @brief 测试获取友好错误描述
     */
    void testFriendlyMessage_data() {
        QTest::addColumn<Error::Code>("code");
        QTest::addColumn<QString>("expectedMessage");

        QTest::newRow("Success") << Error::Success << QString("操作成功");
        QTest::newRow("Fail") << Error::Fail << QString("操作失败");
        QTest::newRow("InvalidParam") << Error::InvalidParam << QString("参数无效");
        QTest::newRow("PinIncorrect") << Error::SkfPinIncorrect << QString("PIN 码错误");
        QTest::newRow("PinLocked") << Error::SkfPinLocked << QString("PIN 码已锁定");
    }

    void testFriendlyMessage() {
        QFETCH(Error::Code, code);
        QFETCH(QString, expectedMessage);

        Error error(code);
        QCOMPARE(error.friendlyMessage(), expectedMessage);
    }

    /**
     * @brief 测试简洁模式字符串
     */
    void testToStringSimple() {
        Error error(Error::SkfPinIncorrect);
        QString str = error.toString(false);

        QVERIFY(str.contains("PIN"));
        QVERIFY(!str.contains("0x"));  // 简洁模式不包含错误码
    }

    /**
     * @brief 测试详细模式字符串
     */
    void testToStringDetailed() {
        Error error(Error::SkfPinIncorrect, "PIN 错误", "SKF_VerifyPIN");
        QString str = error.toString(true);

        QVERIFY(str.contains("PIN"));
        QVERIFY(str.contains("0x0a000024") || str.contains("0x0A000024"));  // 包含错误码
        QVERIFY(str.contains("SKF_VerifyPIN"));  // 包含上下文
    }
};

QTEST_MAIN(TestError)
#include "test_error.moc"
