# ==============================================================================
# wekey-skf SKF Plugin CMakeLists.txt
# ==============================================================================

add_library(wekey_skf_plugin STATIC
    SkfLibrary.cpp
    SkfPlugin.cpp
)

target_include_directories(wekey_skf_plugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_skf_plugin PUBLIC
    Qt6::Core
    OpenSSL::SSL
    OpenSSL::Crypto
    wekey_common
    wekey_plugin_interface
)

# 复制插件元数据
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/skf.json
    ${CMAKE_CURRENT_BINARY_DIR}/skf.json
    COPYONLY
)
