# ==============================================================================
# wekey-skf Plugin Module CMakeLists.txt
# ==============================================================================

# 插件接口库 (仅头文件)
add_subdirectory(interface)

# SKF 插件
add_subdirectory(skf)

# 插件管理器
add_library(wekey_plugin STATIC
    PluginManager.cpp
)

target_include_directories(wekey_plugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_plugin PUBLIC
    Qt6::Core
    wekey_common
    wekey_plugin_interface
    wekey_skf_plugin
)
