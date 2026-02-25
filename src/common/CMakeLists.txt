# ==============================================================================
# wekey-skf Common Module CMakeLists.txt
# ==============================================================================

add_library(wekey_common STATIC
    Error.cpp
)

target_include_directories(wekey_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_common PUBLIC
    Qt6::Core
)
