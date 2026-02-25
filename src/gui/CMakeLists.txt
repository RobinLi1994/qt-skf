# ==============================================================================
# wekey-skf GUI Module CMakeLists.txt
# ==============================================================================

add_library(wekey_gui STATIC
    MainWindow.cpp
    SystemTray.cpp
    pages/ModulePage.cpp
    pages/DevicePage.cpp
    views/AppListView.cpp
    views/AppDetailView.cpp
    pages/ConfigPage.cpp
    pages/LogPage.cpp
    dialogs/LoginDialog.cpp
    dialogs/CsrDialog.cpp
    dialogs/ImportCertDialog.cpp
    dialogs/CertDetailDialog.cpp
    dialogs/CreateAppDialog.cpp
    dialogs/CreateFileDialog.cpp
    dialogs/MessageBox.cpp
)

target_include_directories(wekey_gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wekey_gui PUBLIC
    Qt6::Core
    Qt6::Widgets
    ElaWidgetTools
    wekey_common
    wekey_config
    wekey_log
    wekey_plugin_interface
    wekey_plugin
    wekey_core
)

# ElaWidgetTools 头文件中的 Q_Q_CREATE 宏会产生多余分号，
# 关闭 -Wextra-semi 避免 -Werror 导致编译失败
target_compile_options(wekey_gui PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-extra-semi>
)

# Qt 资源文件
set(ICON_FILE ${CMAKE_SOURCE_DIR}/resources/icons/app.png)
set_source_files_properties(${ICON_FILE} PROPERTIES
    QT_RESOURCE_ALIAS "icons/app.png"
)

set(LOGO_FILE ${CMAKE_SOURCE_DIR}/resources/icons/logo.svg)
set_source_files_properties(${LOGO_FILE} PROPERTIES QT_RESOURCE_ALIAS "icons/logo.svg")

set(NAV_MODULE ${CMAKE_SOURCE_DIR}/resources/icons/nav_module.svg)
set(NAV_DEVICE ${CMAKE_SOURCE_DIR}/resources/icons/nav_device.svg)
set(NAV_APP ${CMAKE_SOURCE_DIR}/resources/icons/nav_app.svg)
set(NAV_CONTAINER ${CMAKE_SOURCE_DIR}/resources/icons/nav_container.svg)
set(NAV_FILE ${CMAKE_SOURCE_DIR}/resources/icons/nav_file.svg)
set(NAV_CONFIG ${CMAKE_SOURCE_DIR}/resources/icons/nav_config.svg)
set(NAV_LOG ${CMAKE_SOURCE_DIR}/resources/icons/nav_log.svg)

set_source_files_properties(${NAV_MODULE} PROPERTIES QT_RESOURCE_ALIAS "icons/nav_module.svg")
set_source_files_properties(${NAV_DEVICE} PROPERTIES QT_RESOURCE_ALIAS "icons/nav_device.svg")
set_source_files_properties(${NAV_APP} PROPERTIES QT_RESOURCE_ALIAS "icons/nav_app.svg")
set_source_files_properties(${NAV_CONTAINER} PROPERTIES QT_RESOURCE_ALIAS "icons/nav_container.svg")
set_source_files_properties(${NAV_FILE} PROPERTIES QT_RESOURCE_ALIAS "icons/nav_file.svg")
set_source_files_properties(${NAV_CONFIG} PROPERTIES QT_RESOURCE_ALIAS "icons/nav_config.svg")
set_source_files_properties(${NAV_LOG} PROPERTIES QT_RESOURCE_ALIAS "icons/nav_log.svg")

qt_add_resources(wekey_gui "resources"
    PREFIX "/"
    FILES
        ${ICON_FILE}
        ${LOGO_FILE}
        ${NAV_MODULE}
        ${NAV_DEVICE}
        ${NAV_APP}
        ${NAV_CONTAINER}
        ${NAV_FILE}
        ${NAV_CONFIG}
        ${NAV_LOG}
)
