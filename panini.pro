## qmake project for panini ##
TEMPLATE = app
VERSION = 0.72.0
TARGET = panini
CONFIG += debug_and_release
QT = gui core opengl
LIBS += -lz -lGLU

# We want Qt5 now
lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

## Directories ##
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

## Platform specific build settings.... ##

## TODO: add option to make Mac OS unversal binaries;
##       add scripts to build distribution packages

win32 {
# this is just for zlib, change if necessary...
    INCLUDEPATH += c:/MinGW/GnuWin32/include
# this sets the program icon the Windows way...
    RC_FILE = ui/paniniWin.rc
}

## Source Files ##
FORMS = ui/mainwindow.ui
HEADERS = src/pvQtPic.h \
    src/CubeLimit_dialog.h
SOURCES = src/main.cpp \
    src/pvQtPic.cpp
HEADERS += src/pvQtView.h \
    src/MainWindow.h \
    src/GLwindow.h
SOURCES += src/pvQtView.cpp \
    src/MainWindow.cpp \
    src/GLwindow.cpp
HEADERS += src/pvQt_QTVR.h
SOURCES += src/pvQt_QTVR.cpp
FORMS += ui/picTypeDialog.ui
HEADERS += src/picTypeDialog.h
SOURCES += src/picTypeDialog.cpp \
    src/pictureTypes.cpp
FORMS += ui/About.ui
HEADERS += src/About.h
HEADERS += src/panosurface.h
SOURCES += src/panosurface.cpp
HEADERS += src/panosphere.h
SOURCES += src/panosphere.cpp
HEADERS += src/panocylinder.h
SOURCES += src/panocylinder.cpp
FORMS += ui/ShowText.ui
HEADERS += src/pvQtMouseModes.h
FORMS += ui/TurnDialog.ui
HEADERS += src/TurnDialog.h
SOURCES += src/TurnDialog.cpp
RESOURCES = ui/PaniniIcon.qrc
FORMS += ui/CubeLimit_dialog.ui
SOURCES += src/About.cpp

## Version Number ##
DEFINES += VERSION=\\\"$$VERSION\\\"

## Install Files ##

# Location
isEmpty( PREFIX ) {
    PREFIX = /usr
}

# binary to /usr/bin
target.path = $$PREFIX$$/bin
INSTALLS += target

linux-g++* {
    ## Desktop File ##
    desktopfile.path  = $$PREFIX$$/share/applications/
    desktopfile.files = linux/*.desktop
    INSTALLS += desktopfile

    # Icon File
    iconfile.path  = $$PREFIX$$/share/pixmaps/
    iconfile.files = linux/panini.png
    INSTALLS += iconfile

    # Appdata file
    appdatafile.path = $$PREFIX$$/$$DATADIR$$/metainfo/
    appdatafile.files = linux/panini.appdata.xml
    INSTALLS += appdatafile
}
