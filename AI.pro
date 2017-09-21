#-------------------------------------------------
#
# Project
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT       += multimedia

TARGET = AI
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    graph.cpp \
    graphicswidget.cpp \
    ai.cpp \
    menu.cpp

HEADERS  += mainwindow.h \
    graph.h \
    graphicswidget.h \
    ai.h \
    menu.h


RESOURCES += \
    resources.qrc

# Extra optimization flags
QMAKE_CXXFLAGS += -msse -mfpmath=sse -ffast-math

LIBS +=  -L$$PWD/DS/ -lDS

#win32:CONFIG(release, debug|release): LIBS +=  -L$$PWD/../DS/release/ -lDS
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../DS/debug/ -lDS

INCLUDEPATH += $$PWD/DS
#INCLUDEPATH += $$PWD/../DS
#DEPENDPATH += $$PWD/../DS



