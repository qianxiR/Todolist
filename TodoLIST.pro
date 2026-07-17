QT += widgets svg
CONFIG += c++17
TEMPLATE = app
TARGET = Todolist
QMAKE_CXXFLAGS += /utf-8
RC_FILE = Todolist.rc

SOURCES += \
    ApplicationStyle.cpp \
    main.cpp \
    MainWindow.cpp \
    QuadrantListWidget.cpp \
    TaskCard.cpp \
    TaskEditorDialog.cpp \
    TodoStorage.cpp \
    TodoTask.cpp

HEADERS += \
    ApplicationStyle.h \
    MainWindow.h \
    QuadrantListWidget.h \
    TaskCard.h \
    TaskEditorDialog.h \
    TodoStorage.h \
    TodoTask.h

RESOURCES += resources.qrc
