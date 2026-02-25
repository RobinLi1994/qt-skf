# ==============================================================================
# API 模块 CMakeLists.txt (M4)
# ==============================================================================

# DTO 库 (M4.1)
add_library(wekey_api_dto STATIC
    dto/HttpTypes.cpp
    dto/Response.cpp
    dto/Request.cpp
)

target_include_directories(wekey_api_dto PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_api_dto PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
    wekey_common
    wekey_plugin_interface
)

# 设置编译选项
target_compile_features(wekey_api_dto PUBLIC cxx_std_17)
target_compile_options(wekey_api_dto PRIVATE
    -Wall -Wextra -Werror
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# HTTP 服务器库 (M4.2 ~ M4.5)
add_library(wekey_api_server STATIC
    HttpServer.cpp
    ApiRouter.cpp
    handlers/PublicHandlers.cpp
    handlers/BusinessHandlers.cpp
)

target_include_directories(wekey_api_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(wekey_api_server PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
    wekey_common
    wekey_config
    wekey_api_dto
    wekey_core
    wekey_plugin
)

target_compile_features(wekey_api_server PUBLIC cxx_std_17)
target_compile_options(wekey_api_server PRIVATE
    -Wall -Wextra -Werror
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)
