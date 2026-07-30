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


add_executable(myschedule ${PROJECT_SOURCES})

target_link_libraries(myschedule PRIVATE Qt5::Widgets Qt5::Multimedia)

# 源文件中含有中文界面和命令行提示。明确以 UTF-8 读取源代码，避免 Linux
# 编译环境按单字节编码处理中文，最终在终端输出双重乱码。
if (MSVC)
    target_compile_options(myschedule PRIVATE /utf-8)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(myschedule PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
