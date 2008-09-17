TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .
HEADERS += GLview.h \
 GLwindow.h \
 MainWindow.h \
 ui_MainWindow.h \
 pvQt.h
FORMS += MainWindow.ui
SOURCES += GLview.cpp \
 GLwindow.cpp \
 MainWindow.cpp \
 main.cpp \
 pvQt.cpp
QT += opengl
CONFIG += debug
