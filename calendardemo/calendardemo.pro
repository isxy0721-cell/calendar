QT += widgets multimedia
CONFIG += c++17
TEMPLATE = app
TARGET = myschedule

SOURCES += \
    main.cpp \
    cli.cpp \
    mainwindow.cpp \
    reminderworker.cpp \
    task.cpp \
    taskmanager.cpp \
    usermanager.cpp

HEADERS += \
    cli.h \
    mainwindow.h \
    reminderworker.h \
    task.h \
    taskmanager.h \
    usermanager.h
