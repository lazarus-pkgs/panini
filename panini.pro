# # qmake project for panini 0.63 ##
TEMPLATE = app
TARGET = Panini
CONFIG += debug_and_release
QT = gui core opengl
LIBS += -lz -lGLU

# # Directories ##
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

HEADERS += version0.h SVNnoVersion.h version1.h
HEADERS += src/pvQtVersion.h
SOURCES += src/About.cpp

## Version Number ##

# You can edit major.minor rev numbers in version0.h
# here we set patch = SVN version number, or contents
# of SVNnoVersion.h if the svnversion command fails
# 1) capture SVN revision number in a file
!svn:system(svnversion -n > SVNversion.h ):CONFIG += svn
!svn { # no SVN version available, use default
    message(could not run svnversion -- is it installed?)
    win32:system(copy SVNnoVersion.h SVNversion.h)
    else:system(cp SVNnoVersion.h SVNversion.h)
}
# 2) generate src/pvQtVersion.h
win32:system(wbin\cat Version0.h SVNversion.h Version1.h > src\pvQtVersion.h)
else:system(cat Version0.h SVNversion.h Version1.h > src/pvQtVersion.h)
 

