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
if(WIN32)
    # Windows 平台添加资源文件（包含图标）
    set(APP_RESOURCES ${CMAKE_SOURCE_DIR}/resources/app.rc)
    add_executable(wekey-skf
        main.cpp
        ${APP_RESOURCES}
    )
else()
    add_executable(wekey-skf
        main.cpp
    )
endif()

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
# 开发阶段：拷贝到 build/lib/，Application::findBuiltinLibPath() 会搜索此路径
# 打包阶段：由打包脚本将库嵌入 .app bundle 的 Frameworks/ 或 exe 同目录
if(APPLE)
    # macOS 按目标架构选择对应的 SKF 库
    # arm64 用 libgm3000.dylib，x86_64 用 libgm3000.1.0_x86.dylib
    # CMAKE_OSX_ARCHITECTURES 可能包含 "arm64;x86_64"（universal）或单一架构
    if(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64" AND NOT CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        # 仅 x86_64
        set(BUILTIN_SKF_SRC "${CMAKE_SOURCE_DIR}/resources/lib/mac/libgm3000.1.0_x86.dylib")
    else()
        # arm64 或 universal（universal 优先用 arm64 版本，运行时按实际架构加载）
        set(BUILTIN_SKF_SRC "${CMAKE_SOURCE_DIR}/resources/lib/mac/libgm3000.dylib")
    endif()
    set(BUILTIN_SKF_DST "${CMAKE_BINARY_DIR}/lib/libgm3000.dylib")
elseif(WIN32)
    set(BUILTIN_SKF_SRC "${CMAKE_SOURCE_DIR}/resources/lib/win/mtoken_gm3000.dll")
    set(BUILTIN_SKF_DST "${CMAKE_BINARY_DIR}/lib/mtoken_gm3000.dll")
endif()

if(BUILTIN_SKF_SRC AND EXISTS "${BUILTIN_SKF_SRC}")
    add_custom_command(TARGET wekey-skf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${BUILTIN_SKF_SRC}" "${BUILTIN_SKF_DST}"
        COMMENT "Copying built-in SKF library to build/lib/"
    )
    message(STATUS "Built-in SKF library: ${BUILTIN_SKF_SRC}")
else()
    message(WARNING "Built-in SKF library not found: ${BUILTIN_SKF_SRC}")
endif()

# Windows 特定设置
if(WIN32)
    set_target_properties(wekey-skf PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# macOS 特定设置
if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE app.icns)
    set(APP_ICON_MACOS ${CMAKE_SOURCE_DIR}/resources/icons/app.icns)
    set_source_files_properties(${APP_ICON_MACOS} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(wekey-skf PRIVATE ${APP_ICON_MACOS})
    
    set_target_properties(wekey-skf PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "WekeySkf"
        MACOSX_BUNDLE_EXECUTABLE_NAME "wekey-skf"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist.in
        MACOSX_BUNDLE_ICON_FILE ${MACOSX_BUNDLE_ICON_FILE}
    )
endif()
