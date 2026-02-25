/**
 * @file Result.h
 * @brief Result<T> 模板类
 *
 * 遵循项目宪法 §3.1：所有错误都必须被显式处理。
 * 使用 Result<T> 替代异常，强制调用方处理错误。
 */

#pragma once

#include "Error.h"

#include <variant>

namespace wekey {

/**
 * @brief 操作结果类型
 *
 * 用于表示可能失败的操作的返回值。
 *
 * 用法示例:
 * @code
 * Result<int> divide(int a, int b) {
 *     if (b == 0) {
 *         return Result<int>::err(Error(Error::InvalidParam, "除数不能为零"));
 *     }
 *     return Result<int>::ok(a / b);
 * }
 *
 * auto result = divide(10, 2);
 * if (result.isOk()) {
 *     qDebug() << "结果:" << result.value();
 * } else {
 *     qDebug() << "错误:" << result.error().friendlyMessage();
 * }
 * @endcode
 *
 * @tparam T 成功时的值类型
 */
template <typename T>
class Result {
public:
    /**
     * @brief 创建成功结果
     * @param value 成功值
     * @return Result 对象
     */
    static Result ok(T value) { return Result(std::move(value)); }

    /**
     * @brief 创建失败结果
     * @param error 错误对象
     * @return Result 对象
     */
    static Result err(Error error) { return Result(std::move(error)); }

    /**
     * @brief 判断是否成功
     */
    [[nodiscard]] bool isOk() const { return std::holds_alternative<T>(data_); }

    /**
     * @brief 判断是否失败
     */
    [[nodiscard]] bool isErr() const { return !isOk(); }

    /**
     * @brief 获取成功值（const 引用）
     * @warning 调用前必须确保 isOk() 返回 true
     */
    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }

    /**
     * @brief 获取成功值（非 const 引用）
     * @warning 调用前必须确保 isOk() 返回 true
     */
    [[nodiscard]] T& value() & { return std::get<T>(data_); }

    /**
     * @brief 获取成功值（右值引用，用于移动）
     * @warning 调用前必须确保 isOk() 返回 true
     */
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }

    /**
     * @brief 获取错误对象
     * @warning 调用前必须确保 isErr() 返回 true
     */
    [[nodiscard]] const Error& error() const { return std::get<Error>(data_); }

    /**
     * @brief 转换成功值
     *
     * 如果是成功状态，应用函数 f 转换值；如果是失败状态，透传错误。
     *
     * @tparam F 转换函数类型
     * @param f 转换函数
     * @return 转换后的 Result
     */
    template <typename F>
    auto map(F&& f) -> Result<decltype(f(std::declval<T>()))> {
        using U = decltype(f(std::declval<T>()));
        if (isOk()) {
            return Result<U>::ok(f(value()));
        }
        return Result<U>::err(error());
    }

    /**
     * @brief 链式调用
     *
     * 如果是成功状态，应用函数 f 返回新的 Result；如果是失败状态，透传错误。
     *
     * @tparam F 返回 Result 的函数类型
     * @param f 链式函数
     * @return 链式调用的结果
     */
    template <typename F>
    auto andThen(F&& f) -> decltype(f(std::declval<T>())) {
        if (isOk()) {
            return f(value());
        }
        return decltype(f(std::declval<T>()))::err(error());
    }

private:
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(Error error) : data_(std::move(error)) {}

    std::variant<T, Error> data_;
};

/**
 * @brief Result<void> 特化
 *
 * 用于不返回值但可能失败的操作。
 */
template <>
class Result<void> {
public:
    /**
     * @brief 创建成功结果
     */
    static Result ok() { return Result(true); }

    /**
     * @brief 创建失败结果
     * @param error 错误对象
     */
    static Result err(Error error) { return Result(std::move(error)); }

    /**
     * @brief 判断是否成功
     */
    [[nodiscard]] bool isOk() const { return success_; }

    /**
     * @brief 判断是否失败
     */
    [[nodiscard]] bool isErr() const { return !success_; }

    /**
     * @brief 获取错误对象
     * @warning 调用前必须确保 isErr() 返回 true
     */
    [[nodiscard]] const Error& error() const { return error_; }

private:
    explicit Result(bool) : success_(true) {}
    explicit Result(Error e) : success_(false), error_(std::move(e)) {}

    bool success_ = false;
    Error error_;
};

}  // namespace wekey
