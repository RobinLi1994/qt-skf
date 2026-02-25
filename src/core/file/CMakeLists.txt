# ==============================================================================
# wekey-skf Core File CMakeLists.txt
# ==============================================================================

add_library(wekey_core_file STATIC
    FileService.cpp
)

target_include_directories(wekey_core_file PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_core_file PUBLIC
    Qt6::Core
    wekey_common
    wekey_plugin
)
