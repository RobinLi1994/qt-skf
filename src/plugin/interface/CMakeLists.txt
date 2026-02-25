# ==============================================================================
# wekey-skf Plugin Interface CMakeLists.txt
# ==============================================================================

# 仅头文件的接口库
add_library(wekey_plugin_interface INTERFACE)

target_include_directories(wekey_plugin_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_plugin_interface INTERFACE
    Qt6::Core
)
