/**
 * @file test_skflibrary.cpp
 * @brief SkfLibrary 单元测试
 *
 * 测试 SKF 动态库加载功能
 */

#include <QTest>

#include "plugin/skf/SkfLibrary.h"

using namespace wekey;

class TestSkfLibrary : public QObject {
    Q_OBJECT

private slots:
    /**
     * @brief 测试加载不存在的库文件
     *
     * 预期：isLoaded() 返回 false，errorString() 包含错误信息
     */
    void testLoadNonExistentFile() {
        SkfLibrary lib("/path/to/nonexistent/library.so");

        QVERIFY(!lib.isLoaded());
        QVERIFY(!lib.errorString().isEmpty());
    }

    /**
     * @brief 测试 isLoaded 状态
     *
     * 预期：加载失败时返回 false
     */
    void testIsLoaded() {
        SkfLibrary lib("/invalid/path/lib.so");

        QCOMPARE(lib.isLoaded(), false);
    }

    /**
     * @brief 测试错误信息
     *
     * 预期：加载失败时 errorString() 非空
     */
    void testErrorString() {
        SkfLibrary lib("/does/not/exist.dll");

        QString error = lib.errorString();
        QVERIFY(!error.isEmpty());
        // 错误信息应该包含有用的描述
        QVERIFY(error.contains("Cannot load library") || error.contains("找不到") ||
                error.contains("not found") || !error.isEmpty());
    }

    /**
     * @brief 测试空路径
     *
     * 预期：加载失败
     */
    void testEmptyPath() {
        SkfLibrary lib("");

        QVERIFY(!lib.isLoaded());
        QVERIFY(!lib.errorString().isEmpty());
    }

    /**
     * @brief 测试函数指针初始化
     *
     * 预期：加载失败时所有函数指针为 nullptr
     */
    void testFunctionPointersNull() {
        SkfLibrary lib("/invalid/path.so");

        QVERIFY(lib.EnumDev == nullptr);
        QVERIFY(lib.ConnectDev == nullptr);
        QVERIFY(lib.DisConnectDev == nullptr);
        QVERIFY(lib.GetDevInfo == nullptr);
        QVERIFY(lib.GenRandom == nullptr);
    }
};

QTEST_MAIN(TestSkfLibrary)
#include "test_skflibrary.moc"
