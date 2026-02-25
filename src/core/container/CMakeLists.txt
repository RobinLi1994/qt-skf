# ==============================================================================
# wekey-skf Core Container CMakeLists.txt
# ==============================================================================

add_library(wekey_core_container STATIC
    ContainerService.cpp
)

target_include_directories(wekey_core_container PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_core_container PUBLIC
    Qt6::Core
    wekey_common
    wekey_plugin
)
