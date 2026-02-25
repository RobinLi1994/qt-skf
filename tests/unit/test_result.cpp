/**
 * @file test_result.cpp
 * @brief Result<T> 模板单元测试
 *
 * 测试用例 M1.2.5T
 */

#include <QtTest>

#include "common/Error.h"
#include "common/Result.h"

using namespace wekey;

class TestResult : public QObject {
    Q_OBJECT

private slots:
    /**
     * @brief 测试 Result<int>::ok()
     */
    void testOkInt() {
        auto result = Result<int>::ok(42);

        QVERIFY(result.isOk());
        QVERIFY(!result.isErr());
        QCOMPARE(result.value(), 42);
    }

    /**
     * @brief 测试 Result<QString>::ok()
     */
    void testOkString() {
        auto result = Result<QString>::ok("hello");

        QVERIFY(result.isOk());
        QCOMPARE(result.value(), QString("hello"));
    }

    /**
     * @brief 测试 Result<int>::err()
     */
    void testErrInt() {
        auto result = Result<int>::err(Error(Error::Fail, "test error"));

        QVERIFY(!result.isOk());
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::Fail);
        QCOMPARE(result.error().message(), QString("test error"));
    }

    /**
     * @brief 测试 Result<void>::ok()
     */
    void testVoidOk() {
        auto result = Result<void>::ok();

        QVERIFY(result.isOk());
        QVERIFY(!result.isErr());
    }

    /**
     * @brief 测试 Result<void>::err()
     */
    void testVoidErr() {
        auto result = Result<void>::err(Error(Error::InvalidParam));

        QVERIFY(!result.isOk());
        QVERIFY(result.isErr());
        QCOMPARE(result.error().code(), Error::InvalidParam);
    }

    /**
     * @brief 测试 isOk() 和 isErr() 互斥
     */
    void testIsOkIsErr() {
        auto okResult = Result<int>::ok(1);
        auto errResult = Result<int>::err(Error(Error::Fail));

        QVERIFY(okResult.isOk() != okResult.isErr());
        QVERIFY(errResult.isOk() != errResult.isErr());
    }

    /**
     * @brief 测试 map() 转换成功值
     */
    void testMap() {
        auto result = Result<int>::ok(10);
        auto mapped = result.map([](int x) { return x * 2; });

        QVERIFY(mapped.isOk());
        QCOMPARE(mapped.value(), 20);
    }

    /**
     * @brief 测试 map() 对错误透传
     */
    void testMapOnErr() {
        auto result = Result<int>::err(Error(Error::Fail, "original error"));
        auto mapped = result.map([](int x) { return x * 2; });

        QVERIFY(mapped.isErr());
        QCOMPARE(mapped.error().code(), Error::Fail);
        QCOMPARE(mapped.error().message(), QString("original error"));
    }

    /**
     * @brief 测试 map() 类型转换
     */
    void testMapTypeConversion() {
        auto result = Result<int>::ok(42);
        auto mapped = result.map([](int x) { return QString::number(x); });

        QVERIFY(mapped.isOk());
        QCOMPARE(mapped.value(), QString("42"));
    }

    /**
     * @brief 测试 andThen() 链式调用成功
     */
    void testAndThen() {
        auto result = Result<int>::ok(10);
        auto chained = result.andThen([](int x) -> Result<int> {
            if (x > 0) {
                return Result<int>::ok(x * 2);
            }
            return Result<int>::err(Error(Error::InvalidParam));
        });

        QVERIFY(chained.isOk());
        QCOMPARE(chained.value(), 20);
    }

    /**
     * @brief 测试 andThen() 链式调用失败
     */
    void testAndThenFail() {
        auto result = Result<int>::ok(-1);
        auto chained = result.andThen([](int x) -> Result<int> {
            if (x > 0) {
                return Result<int>::ok(x * 2);
            }
            return Result<int>::err(Error(Error::InvalidParam, "value must be positive"));
        });

        QVERIFY(chained.isErr());
        QCOMPARE(chained.error().code(), Error::InvalidParam);
    }

    /**
     * @brief 测试 andThen() 对错误透传
     */
    void testAndThenOnErr() {
        auto result = Result<int>::err(Error(Error::Fail));
        auto chained = result.andThen([](int x) -> Result<int> {
            return Result<int>::ok(x * 2);
        });

        QVERIFY(chained.isErr());
        QCOMPARE(chained.error().code(), Error::Fail);
    }

    /**
     * @brief 测试右值引用移动语义
     */
    void testMoveValue() {
        QString original = "test string";
        auto result = Result<QString>::ok(std::move(original));

        // 移动后取值
        QString value = std::move(result).value();
        QCOMPARE(value, QString("test string"));
    }

    /**
     * @brief 测试 const 引用取值
     */
    void testConstRefValue() {
        auto result = Result<QString>::ok("test");
        const auto& ref = result.value();

        QCOMPARE(ref, QString("test"));
    }

    /**
     * @brief 测试复杂类型
     */
    void testComplexType() {
        struct Data {
            int id;
            QString name;
        };

        auto result = Result<Data>::ok(Data{1, "test"});

        QVERIFY(result.isOk());
        QCOMPARE(result.value().id, 1);
        QCOMPARE(result.value().name, QString("test"));
    }

    /**
     * @brief 测试 QList 类型
     */
    void testListType() {
        QList<int> list = {1, 2, 3, 4, 5};
        auto result = Result<QList<int>>::ok(list);

        QVERIFY(result.isOk());
        QCOMPARE(result.value().size(), 5);
        QCOMPARE(result.value()[0], 1);
    }
};

QTEST_MAIN(TestResult)
#include "test_result.moc"
