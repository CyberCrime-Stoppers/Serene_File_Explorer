cmake_minimum_required(VERSION 3.21)
project(SereneFileExplorer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

set(SEREFE_SOURCES
    src/main.cpp
    src/core/filesystem.cpp
    src/core/clipboard.cpp
    src/core/settings.cpp
    src/ui/theme.cpp
    src/ui/sidebar.cpp
    src/ui/mainwindow.cpp
    src/models/filemodel.cpp
    src/jobs/listjob.cpp
    src/jobs/searchjob.cpp
    src/transfer/transfermanager.cpp
    src/views/directoryview.cpp
)

set(SEREFE_HEADERS
    src/core/types.h
    src/core/filesystem.h
    src/core/clipboard.h
    src/core/settings.h
    src/ui/theme.h
    src/ui/sidebar.h
    src/ui/mainwindow.h
    src/models/filemodel.h
    src/jobs/listjob.h
    src/jobs/searchjob.h
    src/transfer/transfermanager.h
    src/views/directoryview.h
)

qt_add_executable(serene-file-explorer
    ${SEREFE_SOURCES}
    ${SEREFE_HEADERS}
    src/resources.qrc
)

target_link_libraries(serene-file-explorer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_include_directories(serene-file-explorer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

install(TARGETS serene-file-explorer RUNTIME DESTINATION bin)
