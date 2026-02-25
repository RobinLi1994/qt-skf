# ==============================================================================
# wekey-skf Log Module CMakeLists.txt
# ==============================================================================

add_library(wekey_log STATIC
    Logger.cpp
    LogModel.cpp
)

target_include_directories(wekey_log PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_log PUBLIC
    Qt6::Core
    wekey_common
    wekey_config
)
