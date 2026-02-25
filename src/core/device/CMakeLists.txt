# ==============================================================================
# wekey-skf Core Device CMakeLists.txt
# ==============================================================================

add_library(wekey_core_device STATIC
    DeviceService.cpp
)

target_include_directories(wekey_core_device PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_core_device PUBLIC
    Qt6::Core
    wekey_common
    wekey_plugin
)
