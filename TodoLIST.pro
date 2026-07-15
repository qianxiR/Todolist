QT += widgets svg
CONFIG += c++17
TEMPLATE = app
TARGET = Todolist
QMAKE_CXXFLAGS += /utf-8
RC_FILE = Todolist.rc

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    QuadrantListWidget.cpp \
    TaskCard.cpp \
    TodoStorage.cpp \
    TodoTask.cpp

HEADERS += \
    MainWindow.h \
    QuadrantListWidget.h \
    TaskCard.h \
    TodoStorage.h \
    TodoTask.h

RESOURCES += resources.qrc
