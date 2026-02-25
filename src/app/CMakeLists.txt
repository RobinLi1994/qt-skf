# ==============================================================================
# wekey-skf Application Entry CMakeLists.txt
# ==============================================================================

# Application 库 (用于测试)
add_library(wekey_app STATIC
    Application.cpp
)

target_include_directories(wekey_app PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(wekey_app PUBLIC
    Qt6::Core
    Qt6::Widgets
    wekey_common
    wekey_config
    wekey_log
    wekey_plugin
)

# 可执行文件
add_executable(wekey-skf
    main.cpp
)

target_include_directories(wekey-skf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey-skf PRIVATE
    wekey_app
    wekey_plugin
    wekey_core
    Qt6::Network
    Qt6::HttpServer
    wekey_api_server
    wekey_gui
)

# ElaWidgetTools 头文件中的宏会产生多余分号警告
target_compile_options(wekey-skf PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-extra-semi>
)

# 拷贝内置 SKF 库到构建目录
if(APPLE)
    set(BUILTIN_SKF_SRC "${CMAKE_SOURCE_DIR}/resources/lib/mac/libgm3000.dylib")
    set(BUILTIN_SKF_DST "${CMAKE_BINARY_DIR}/lib/libgm3000.dylib")
elseif(WIN32)
    set(BUILTIN_SKF_SRC "${CMAKE_SOURCE_DIR}/resources/lib/win/mtoken_gm3000.dll")
    set(BUILTIN_SKF_DST "${CMAKE_BINARY_DIR}/lib/mtoken_gm3000.dll")
endif()

if(BUILTIN_SKF_SRC AND EXISTS "${BUILTIN_SKF_SRC}")
    add_custom_command(TARGET wekey-skf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${BUILTIN_SKF_SRC}" "${BUILTIN_SKF_DST}"
        COMMENT "Copying built-in SKF library"
    )
endif()

# Windows 特定设置
if(WIN32)
    set_target_properties(wekey-skf PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# macOS 特定设置
if(APPLE)
    set_target_properties(wekey-skf PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "WekeySkf"
        MACOSX_BUNDLE_EXECUTABLE_NAME "wekey-skf"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist.in
    )
endif()
