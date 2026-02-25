# ==============================================================================
# wekey-skf Core Application CMakeLists.txt
# ==============================================================================

add_library(wekey_core_application STATIC
    AppService.cpp
)

target_include_directories(wekey_core_application PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_core_application PUBLIC
    Qt6::Core
    wekey_common
    wekey_plugin
)
