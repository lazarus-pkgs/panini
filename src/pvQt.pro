TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .
HEADERS += GLview.h \
 GLwindow.h \
 MainWindow.h \
 ui_MainWindow.h \
 pvQt.h \
 ui_PicSpec.h \
 picSpec.h
FORMS += MainWindow.ui PicSpec.ui
SOURCES += GLview.cpp \
 GLwindow.cpp \
 MainWindow.cpp \
 main.cpp \
 pvQt.cpp \
 picSpec.cpp
QT += opengl
CONFIG += debug
