# ==============================================================================
# wekey-skf Core Crypto CMakeLists.txt
# ==============================================================================

add_library(wekey_core_crypto STATIC
    CertService.cpp
)

target_include_directories(wekey_core_crypto PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_core_crypto PUBLIC
    Qt6::Core
    wekey_common
    wekey_plugin
)
