# ==============================================================================
# wekey-skf Core Module CMakeLists.txt
# ==============================================================================

add_subdirectory(device)
add_subdirectory(application)
add_subdirectory(container)
add_subdirectory(crypto)
add_subdirectory(file)

# 核心库聚合
add_library(wekey_core INTERFACE)

target_link_libraries(wekey_core INTERFACE
    wekey_core_device
    wekey_core_application
    wekey_core_container
    wekey_core_crypto
    wekey_core_file
)
