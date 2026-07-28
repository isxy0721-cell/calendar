cmake_minimum_required(VERSION 3.16)

project(calendardemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
    main.cpp
    cli.h cli.cpp
    task.h task.cpp
    taskmanager.h taskmanager.cpp
    usermanager.h usermanager.cpp
    reminderworker.h reminderworker.cpp
    vosktranscriber.h vosktranscriber.cpp
    mainwindow.h mainwindow.cpp
)


if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(calendardemo MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(calendardemo ${PROJECT_SOURCES})
endif()

target_link_libraries(calendardemo PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(calendardemo)
endif()
