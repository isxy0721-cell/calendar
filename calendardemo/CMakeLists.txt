cmake_minimum_required(VERSION 3.16)

project(calendardemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
    main.cpp
    cli.h cli.cpp
    task.h task.cpp
    taskmanager.h taskmanager.cpp
    usermanager.h usermanager.cpp
    reminderworker.h reminderworker.cpp
    vosktranscriber.h vosktranscriber.cpp
    mainwindow.h mainwindow.cpp
    resources.qrc
)


add_executable(calendardemo ${PROJECT_SOURCES})

target_link_libraries(calendardemo PRIVATE Qt5::Widgets Qt5::Multimedia)
