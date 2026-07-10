cmake_minimum_required(VERSION 3.9)

project(MouseClick VERSION 3.0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(QWINDOWKIT_ROOT "${CMAKE_SOURCE_DIR}/deps/qwindowkit")
set(QWindowKit_DIR "${QWINDOWKIT_ROOT}/lib/cmake/QWindowKit")
find_package(QWindowKit REQUIRED)

set(QHOTKEY_ROOT "${CMAKE_SOURCE_DIR}/deps/qhotkey")
set(QHotkey_DIR "${QHOTKEY_ROOT}/lib/cmake/QHotkey")
find_package(QHotkey REQUIRED)

set(QWK_WINDOWBAR_FILES
    src/qwk_window_bar/windowbar.h      src/qwk_window_bar/windowbar.cpp
    src/qwk_window_bar/windowbutton.h   src/qwk_window_bar/windowbutton.cpp
    src/qwk_window_bar/private/windowbar_p.h
    src/qwk_window_bar/private/windowbutton_p.h
)

set(TRANSLATIONS_FILES
    i18n/MouseClick_zh_CN.ts
    i18n/MouseClick_zh_TW.ts
    i18n/MouseClick_en_US.ts
)

set(QRC_FILES assets/assets.qrc)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.h                            src/mainwindow.cpp
        src/modules/nav_pages/navpage.h             src/modules/nav_pages/navpage.cpp
        src/modules/nav_pages/mouseclickpage.h      src/modules/nav_pages/mouseclickpage.cpp
        src/modules/nav_pages/beautifycursorpage.h  src/modules/nav_pages/beautifycursorpage.cpp
        src/modules/nav_pages/settingspage.h        src/modules/nav_pages/settingspage.cpp
        src/modules/settingsagent.h                 src/modules/settingsagent.cpp
        src/modules/translationmanager.h            src/modules/translationmanager.cpp
        src/modules/shared.h                        src/modules/shared.cpp
        src/modules/hotkeylineedit.h                src/modules/hotkeylineedit.cpp
        src/modules/messagebox.h                    src/modules/messagebox.cpp
        src/utils/clicker.h                         src/utils/clicker.cpp
        src/utils/logger.h                          src/utils/logger.cpp
        src/utils/cursormanager.h                   src/utils/cursormanager.cpp
        ${CMAKE_BINARY_DIR}/resources.rc
        ${QWK_WINDOWBAR_FILES}
        ${TRANSLATIONS_FILES}
        ${QRC_FILES}
)

# 通过 configure_file 将 manifest 的绝对路径注入 .rc 文件（windres 需要绝对路径）
configure_file(
    ${CMAKE_SOURCE_DIR}/src/resources.rc.in
    ${CMAKE_BINARY_DIR}/resources.rc
    @ONLY
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    # qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS_FILES})
    qt_add_translations(${PROJECT_NAME} TS_FILES ${TRANSLATIONS_FILES})
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS_FILES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(${PROJECT_NAME} PRIVATE "${QWINDOWKIT_ROOT}/include/QWindowKit::Widgets")
target_link_libraries(${PROJECT_NAME} PRIVATE QWindowKit::Widgets)

target_include_directories(${PROJECT_NAME} PRIVATE "${QHOTKEY_ROOT}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE qhotkey)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.seaepoch.MouseClick)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()


# 构建打包环境 dist 目录
# 1. 复制 QWindowKit 的 dll 文件到 dist 目录
add_custom_command(
    TARGET MouseClick
    PRE_BUILD  # 在构建之前执行
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/deps/qwindowkit/bin"
        "${CMAKE_BINARY_DIR}/dist"
    COMMENT "Copy the dll files of QWindowKit to the dist directory!"
)

# 2. 复制生成的可执行文件到 dist 目录
add_custom_command(
    TARGET MouseClick
    POST_BUILD  # 在可执行文件生成之后执行
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:MouseClick>"  # 目标文件，即生成的 MouseClick.exe
        "${CMAKE_BINARY_DIR}/dist"
    COMMENT "Copy the generated executable to the dist directory!"
)

# 复制已有鼠标资源到程序环境中
# 1. 复制一份到构建目录中
add_custom_command(
    TARGET MouseClick
    PRE_BUILD  # 在构建之前执行
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets/cursors"
        "${CMAKE_BINARY_DIR}/cursors"
    COMMENT "Copy the cursors files to the build directory!"
)

# 2. 复制一份到 dist 目录中
add_custom_command(
    TARGET MouseClick
    PRE_BUILD  # 在构建之前执行
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets/cursors"
        "${CMAKE_BINARY_DIR}/dist/cursors"
    COMMENT "Copy the cursors files to the dist directory!"
)
