QT += widgets multimedia
CONFIG += c++17
TEMPLATE = app
TARGET = myschedule

SOURCES += \
    main.cpp \
    cli.cpp \
    mainwindow.cpp \
    reminderworker.cpp \
    vosktranscriber.cpp \
    task.cpp \
    taskmanager.cpp \
    usermanager.cpp

HEADERS += \
    cli.h \
    mainwindow.h \
    reminderworker.h \
    vosktranscriber.h \
    task.h \
    taskmanager.h \
    usermanager.h

RESOURCES += resources.qrc
