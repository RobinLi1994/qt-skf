# ==============================================================================
# wekey-skf Config Module CMakeLists.txt
# ==============================================================================

add_library(wekey_config STATIC
    Config.cpp
)

target_include_directories(wekey_config PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_config PUBLIC
    Qt6::Core
    wekey_common
)
