# ==============================================================================
# wekey-skf - TrustAsia SKF Device Management Tool
# ==============================================================================
cmake_minimum_required(VERSION 3.20)

project(wekey-skf
    VERSION 1.0.0
    DESCRIPTION "TrustAsia SKF Device Management Tool"
    LANGUAGES CXX
)

# ==============================================================================
# C++ 标准设置
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# Qt 自动化设置
# ==============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ==============================================================================
# 构建选项
# ==============================================================================
option(ENABLE_ASAN "Enable AddressSanitizer for memory error detection" OFF)
option(ENABLE_TESTING "Enable testing" ON)

# ==============================================================================
# AddressSanitizer 配置
# ==============================================================================
if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

# ==============================================================================
# 编译器警告设置
# ==============================================================================
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# ==============================================================================
# Qt 依赖查找
# ==============================================================================
# 消除 Qt 私有模块警告（ElaWidgetTools 依赖 WidgetsPrivate）
set(QT_NO_PRIVATE_MODULE_WARNING ON CACHE BOOL "Suppress Qt private module ABI warning")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    WidgetsPrivate
    Network
    HttpServer
)

if(ENABLE_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()

message(STATUS "Qt6 version: ${Qt6_VERSION}")

# ==============================================================================
# OpenSSL 依赖
# ==============================================================================
find_package(OpenSSL REQUIRED)
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")

# ==============================================================================
# 第三方依赖 - ElaWidgetTools (Fluent-UI 风格 Qt 组件库)
# ==============================================================================
# 版本锁定说明：
# - Commit: 6d46c5a4fd95cc2ad76099f849e3fe2465dff5a3
# - 锁定日期: 2026-02-11
# - 原因: 确保构建可重复性，避免上游破坏性变更
# - 更新方式: 定期检查上游更新，测试后更新此 commit hash
include(FetchContent)

# 临时保持使用 FetchContent_Populate（而非 MakeAvailable）
# 原因：需要跳过 ElaWidgetTools 顶层 CMakeLists.txt 的 Qt 版本检查和 Example 编译
cmake_policy(SET CMP0169 OLD)

FetchContent_Declare(
    ElaWidgetTools
    GIT_REPOSITORY https://github.com/Liniyous/ElaWidgetTools.git
    GIT_TAG        6d46c5a4fd95cc2ad76099f849e3fe2465dff5a3
    GIT_SHALLOW    FALSE  # 使用 commit hash 需要完整克隆
)

# 构建为静态库，避免运行时需要分发动态库
set(ELAWIDGETTOOLS_BUILD_STATIC_LIB ON CACHE BOOL "Build ElaWidgetTools as static library" FORCE)

# ElaWidgetTools 内部使用 QT_VERSION_MAJOR 变量来链接对应 Qt 版本
# 由于我们跳过了其顶层 CMakeLists.txt，需要手动设置此变量
set(QT_VERSION_MAJOR ${Qt6_VERSION_MAJOR})

# 手动 Populate，跳过顶层 CMakeLists.txt 的 Qt 版本检查和 Example 目录
# 只编译 ElaWidgetTools 库本身
FetchContent_GetProperties(ElaWidgetTools)
if(NOT elawidgettools_POPULATED)
    FetchContent_Populate(ElaWidgetTools)

    # 第三方库编译时关闭 -Werror，避免警告导致编译失败
    set(SAVED_COMPILE_OPTIONS_WERROR OFF)
    if(MSVC)
        # MSVC: 暂时不处理，保持原样
    else()
        # GCC/Clang: 对 ElaWidgetTools 目标单独关闭 -Werror
    endif()

    add_subdirectory(
        ${elawidgettools_SOURCE_DIR}/ElaWidgetTools
        ${elawidgettools_BINARY_DIR}/ElaWidgetTools
    )

    # 对 ElaWidgetTools 目标关闭 -Werror，避免第三方代码警告导致编译失败
    if(TARGET ElaWidgetTools)
        target_compile_options(ElaWidgetTools PRIVATE
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error>
        )
    endif()
endif()

# ==============================================================================
# 全局包含目录
# ==============================================================================
include_directories(${CMAKE_SOURCE_DIR}/src)

# ==============================================================================
# 子目录
# ==============================================================================
add_subdirectory(src)

if(ENABLE_TESTING)
    enable_testing()
    #add_subdirectory(tests)
endif()

# ==============================================================================
# 安装配置
# ==============================================================================
# 注意: 只有当 wekey-skf 目标存在时才配置安装
# TODO: M1.6 完成后启用
# install(TARGETS wekey-skf
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
# )

# ==============================================================================
# 打印配置摘要
# ==============================================================================
message(STATUS "")
message(STATUS "=== wekey-skf Configuration Summary ===")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt version:     ${Qt6_VERSION}")
message(STATUS "  ASAN:           ${ENABLE_ASAN}")
message(STATUS "  Testing:        ${ENABLE_TESTING}")
message(STATUS "========================================")
message(STATUS "")
