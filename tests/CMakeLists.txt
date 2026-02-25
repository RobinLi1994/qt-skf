# ==============================================================================
# wekey-skf Tests CMakeLists.txt
# ==============================================================================

# ==============================================================================
# 测试通用设置
# ==============================================================================
include(CTest)

# Qt Test 模块
find_package(Qt6 REQUIRED COMPONENTS Test)

# ==============================================================================
# 测试辅助函数
# ==============================================================================
# 添加单元测试的便捷函数
function(add_unit_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_common
        wekey_config
        wekey_log
        wekey_plugin_interface
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    # 设置测试属性
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# 添加集成测试的便捷函数
function(add_integration_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        integration/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Test
        wekey_common
        wekey_config
        wekey_log
        wekey_plugin_interface
        wekey_plugin
        wekey_core
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    # 设置测试属性
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 60
        LABELS "integration"
    )
endfunction()

# ==============================================================================
# 基础测试辅助函数 (仅依赖 common)
# ==============================================================================
function(add_basic_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_common
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# Config 测试辅助函数 (依赖 common + config)
# ==============================================================================
function(add_config_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_common
        wekey_config
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# Log 测试辅助函数 (依赖 common + config + log)
# ==============================================================================
function(add_log_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_common
        wekey_config
        wekey_log
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# Plugin Interface 测试辅助函数 (依赖 common + plugin_interface)
# ==============================================================================
function(add_plugin_interface_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_common
        wekey_plugin_interface
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# App 测试辅助函数 (依赖 app 库)
# ==============================================================================
function(add_app_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Test
        wekey_app
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# SKF 测试辅助函数 (依赖 common + plugin_interface + skf_plugin)
# ==============================================================================
function(add_skf_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_common
        wekey_plugin_interface
        wekey_skf_plugin
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# Plugin Manager 测试辅助函数 (依赖 common + plugin_interface + skf_plugin + plugin)
# ==============================================================================
function(add_plugin_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_common
        wekey_plugin_interface
        wekey_skf_plugin
        wekey_plugin
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# Core Service 测试辅助函数 (依赖 core + plugin)
# ==============================================================================
function(add_core_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        wekey_core
        wekey_plugin
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# API 测试辅助函数 (依赖 common + plugin_interface + api_dto)
# ==============================================================================
function(add_api_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Test
        Qt6::HttpServer
        wekey_common
        wekey_plugin_interface
        wekey_api_dto
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# API Server 测试辅助函数 (依赖 api_server)
# ==============================================================================
function(add_api_server_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Test
        Qt6::HttpServer
        wekey_common
        wekey_plugin_interface
        wekey_api_dto
        wekey_api_server
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# GUI 测试辅助函数 (依赖 gui + core + plugin)
# ==============================================================================
function(add_gui_test TEST_NAME)
    set(TEST_TARGET test_${TEST_NAME})

    add_executable(${TEST_TARGET}
        unit/${TEST_TARGET}.cpp
    )

    target_link_libraries(${TEST_TARGET} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Test
        wekey_common
        wekey_config
        wekey_log
        wekey_plugin_interface
        wekey_plugin
        wekey_core
        wekey_gui
    )

    target_include_directories(${TEST_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )

    # 关闭 ElaWidgetTools 的警告,避免第三方库警告导致编译失败
    target_compile_options(${TEST_TARGET} PRIVATE
        -Wno-error=extra-semi
        -Wno-extra-semi
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ==============================================================================
# 单元测试
# ==============================================================================
# M1 基础模块测试
add_basic_test(error)
add_basic_test(result)
add_config_test(config)
add_log_test(logger)
add_log_test(logmodel)
add_plugin_interface_test(plugintypes)
add_app_test(application)

# M2 插件测试
add_skf_test(skflibrary)
add_skf_test(skfplugin)
add_plugin_test(pluginmanager)

# M3 核心服务测试
add_core_test(deviceservice)
add_core_test(appservice)
add_core_test(containerservice)
add_core_test(certservice)
add_core_test(fileservice)

# M4 API 测试
# M4.1 DTO 测试
add_api_test(httptypes)
add_api_test(response)
add_api_test(request)

# M4.2+ 测试
add_api_server_test(httpserver)
add_api_server_test(apirouter)
add_api_server_test(publichandlers)
add_api_server_test(businesshandlers)
#add_api_server_test(adminhandlers)
# add_unit_test(businesshandlers_device)
# add_unit_test(businesshandlers_login)
# add_unit_test(businesshandlers_csr)
# add_unit_test(businesshandlers_cert)
# add_unit_test(businesshandlers_sign)
# add_unit_test(adminhandlers_mod)
# add_unit_test(adminhandlers_dev)
# add_unit_test(adminhandlers_app)
# add_unit_test(adminhandlers_container)
# add_unit_test(adminhandlers_file)
# add_unit_test(adminhandlers_settings)

# M5 GUI 测试
add_gui_test(mainwindow)
add_gui_test(systemtray)
add_gui_test(modulepage)
add_gui_test(devicepage)
#add_gui_test(apppage)
#add_gui_test(containerpage)
#add_gui_test(filepage)
add_gui_test(configpage)
add_gui_test(logpage)
add_gui_test(logindialog)
add_gui_test(csrdialog)
add_gui_test(importcertdialog)
add_gui_test(messagebox)
add_gui_test(createfiledialog)

# ==============================================================================
# 集成测试
# ==============================================================================
add_integration_test(e2e)
add_integration_test(hotplug)
add_integration_test(pin)
