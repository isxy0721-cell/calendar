cmake_minimum_required(VERSION 3.5)

project(calendardemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt组件：新增 Multimedia(音乐提醒)、Network(语音识别) 模块
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Network)

# 项目源码列表：后续每新增一个类，都把 .h 和 .cpp 加到这里
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        task.h
        task.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(calendardemo
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(calendardemo
        ${PROJECT_SOURCES}
    )
endif()

# 链接Qt库
target_link_libraries(calendardemo PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Network
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(calendardemo)
endif()
