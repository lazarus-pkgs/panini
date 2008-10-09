/********************************************************************************
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Thu Oct 9 12:21:39 2008
**      by: Qt User Interface Compiler version 4.4.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action_Home;
    QAction *actionZoom_In;
    QAction *actionZoom_Out;
    QAction *actionPan_Left;
    QAction *actionPan_Right;
    QAction *actionTilt_Up;
    QAction *actionTilt_Down;
    QAction *actionRoll_Left;
    QAction *actionRoll_Right;
    QAction *actionEye_In;
    QAction *actionEye_Out;
    QAction *actionFullFrame;
    QAction *action_SuperFish;
    QAction *actionReset;
    QAction *actionRectilinear;
    QAction *actionFisheye;
    QAction *actionCylindrical;
    QAction *actionEquirectangular;
    QAction *actionHemispherical;
    QAction *actionCube_faces;
    QAction *actionQuit;
    QAction *actionQTVR;
    QAction *actionPT_script;
    QWidget *centralwidget;
    QMenuBar *menubar;
    QMenu *menu_View;
    QMenu *menuLoad;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(800, 224);
    action_Home = new QAction(MainWindow);
    action_Home->setObjectName(QString::fromUtf8("action_Home"));
    actionZoom_In = new QAction(MainWindow);
    actionZoom_In->setObjectName(QString::fromUtf8("actionZoom_In"));
    actionZoom_Out = new QAction(MainWindow);
    actionZoom_Out->setObjectName(QString::fromUtf8("actionZoom_Out"));
    actionPan_Left = new QAction(MainWindow);
    actionPan_Left->setObjectName(QString::fromUtf8("actionPan_Left"));
    actionPan_Right = new QAction(MainWindow);
    actionPan_Right->setObjectName(QString::fromUtf8("actionPan_Right"));
    actionTilt_Up = new QAction(MainWindow);
    actionTilt_Up->setObjectName(QString::fromUtf8("actionTilt_Up"));
    actionTilt_Down = new QAction(MainWindow);
    actionTilt_Down->setObjectName(QString::fromUtf8("actionTilt_Down"));
    actionRoll_Left = new QAction(MainWindow);
    actionRoll_Left->setObjectName(QString::fromUtf8("actionRoll_Left"));
    actionRoll_Right = new QAction(MainWindow);
    actionRoll_Right->setObjectName(QString::fromUtf8("actionRoll_Right"));
    actionEye_In = new QAction(MainWindow);
    actionEye_In->setObjectName(QString::fromUtf8("actionEye_In"));
    actionEye_Out = new QAction(MainWindow);
    actionEye_Out->setObjectName(QString::fromUtf8("actionEye_Out"));
    actionFullFrame = new QAction(MainWindow);
    actionFullFrame->setObjectName(QString::fromUtf8("actionFullFrame"));
    action_SuperFish = new QAction(MainWindow);
    action_SuperFish->setObjectName(QString::fromUtf8("action_SuperFish"));
    actionReset = new QAction(MainWindow);
    actionReset->setObjectName(QString::fromUtf8("actionReset"));
    actionRectilinear = new QAction(MainWindow);
    actionRectilinear->setObjectName(QString::fromUtf8("actionRectilinear"));
    actionFisheye = new QAction(MainWindow);
    actionFisheye->setObjectName(QString::fromUtf8("actionFisheye"));
    actionCylindrical = new QAction(MainWindow);
    actionCylindrical->setObjectName(QString::fromUtf8("actionCylindrical"));
    actionEquirectangular = new QAction(MainWindow);
    actionEquirectangular->setObjectName(QString::fromUtf8("actionEquirectangular"));
    actionHemispherical = new QAction(MainWindow);
    actionHemispherical->setObjectName(QString::fromUtf8("actionHemispherical"));
    actionCube_faces = new QAction(MainWindow);
    actionCube_faces->setObjectName(QString::fromUtf8("actionCube_faces"));
    actionQuit = new QAction(MainWindow);
    actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
    actionQTVR = new QAction(MainWindow);
    actionQTVR->setObjectName(QString::fromUtf8("actionQTVR"));
    actionPT_script = new QAction(MainWindow);
    actionPT_script->setObjectName(QString::fromUtf8("actionPT_script"));
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    MainWindow->setCentralWidget(centralwidget);
    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 800, 22));
    menu_View = new QMenu(menubar);
    menu_View->setObjectName(QString::fromUtf8("menu_View"));
    menuLoad = new QMenu(menubar);
    menuLoad->setObjectName(QString::fromUtf8("menuLoad"));
    MainWindow->setMenuBar(menubar);
    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    MainWindow->setStatusBar(statusbar);

    menubar->addAction(menuLoad->menuAction());
    menubar->addAction(menu_View->menuAction());
    menu_View->addAction(actionZoom_In);
    menu_View->addAction(actionZoom_Out);
    menu_View->addAction(actionPan_Left);
    menu_View->addAction(actionPan_Right);
    menu_View->addAction(actionTilt_Up);
    menu_View->addAction(actionTilt_Down);
    menu_View->addAction(actionRoll_Left);
    menu_View->addAction(actionRoll_Right);
    menu_View->addAction(action_Home);
    menu_View->addAction(actionEye_In);
    menu_View->addAction(actionEye_Out);
    menu_View->addAction(actionFullFrame);
    menu_View->addAction(action_SuperFish);
    menu_View->addAction(actionReset);
    menuLoad->addAction(actionQTVR);
    menuLoad->addAction(actionRectilinear);
    menuLoad->addAction(actionFisheye);
    menuLoad->addAction(actionCylindrical);
    menuLoad->addAction(actionEquirectangular);
    menuLoad->addAction(actionHemispherical);
    menuLoad->addAction(actionCube_faces);
    menuLoad->addAction(actionPT_script);
    menuLoad->addSeparator();
    menuLoad->addAction(actionQuit);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "pvQt", 0, QApplication::UnicodeUTF8));
    action_Home->setText(QApplication::translate("MainWindow", "Home Y,P,R", 0, QApplication::UnicodeUTF8));
    action_Home->setShortcut(QApplication::translate("MainWindow", "Home", 0, QApplication::UnicodeUTF8));
    actionZoom_In->setText(QApplication::translate("MainWindow", "Zoom &In", 0, QApplication::UnicodeUTF8));
    actionZoom_In->setShortcut(QApplication::translate("MainWindow", "Ctrl+=", 0, QApplication::UnicodeUTF8));
    actionZoom_Out->setText(QApplication::translate("MainWindow", "Zoom &Out", 0, QApplication::UnicodeUTF8));
    actionZoom_Out->setShortcut(QApplication::translate("MainWindow", "Ctrl+-", 0, QApplication::UnicodeUTF8));
    actionPan_Left->setText(QApplication::translate("MainWindow", "Yaw &Left", 0, QApplication::UnicodeUTF8));
    actionPan_Left->setShortcut(QApplication::translate("MainWindow", "Left", 0, QApplication::UnicodeUTF8));
    actionPan_Right->setText(QApplication::translate("MainWindow", "Yaw &Right", 0, QApplication::UnicodeUTF8));
    actionPan_Right->setShortcut(QApplication::translate("MainWindow", "Right", 0, QApplication::UnicodeUTF8));
    actionTilt_Up->setText(QApplication::translate("MainWindow", "Pitch &Up", 0, QApplication::UnicodeUTF8));
    actionTilt_Up->setShortcut(QApplication::translate("MainWindow", "Up", 0, QApplication::UnicodeUTF8));
    actionTilt_Down->setText(QApplication::translate("MainWindow", "Pitch &Down", 0, QApplication::UnicodeUTF8));
    actionTilt_Down->setShortcut(QApplication::translate("MainWindow", "Down", 0, QApplication::UnicodeUTF8));
    actionRoll_Left->setText(QApplication::translate("MainWindow", "Roll Left", 0, QApplication::UnicodeUTF8));
    actionRoll_Left->setShortcut(QApplication::translate("MainWindow", "Ctrl+Left", 0, QApplication::UnicodeUTF8));
    actionRoll_Right->setText(QApplication::translate("MainWindow", "Roll Right", 0, QApplication::UnicodeUTF8));
    actionRoll_Right->setShortcut(QApplication::translate("MainWindow", "Ctrl+Right", 0, QApplication::UnicodeUTF8));
    actionEye_In->setText(QApplication::translate("MainWindow", "Eye In", 0, QApplication::UnicodeUTF8));
    actionEye_In->setShortcut(QApplication::translate("MainWindow", "Ctrl+Up", 0, QApplication::UnicodeUTF8));
    actionEye_Out->setText(QApplication::translate("MainWindow", "Eye Out", 0, QApplication::UnicodeUTF8));
    actionEye_Out->setShortcut(QApplication::translate("MainWindow", "Ctrl+Down", 0, QApplication::UnicodeUTF8));
    actionFullFrame->setText(QApplication::translate("MainWindow", "Fullframe", 0, QApplication::UnicodeUTF8));
    actionFullFrame->setShortcut(QApplication::translate("MainWindow", "PgDown", 0, QApplication::UnicodeUTF8));
    action_SuperFish->setText(QApplication::translate("MainWindow", "SuperFish", 0, QApplication::UnicodeUTF8));
    action_SuperFish->setShortcut(QApplication::translate("MainWindow", "PgUp", 0, QApplication::UnicodeUTF8));
    actionReset->setText(QApplication::translate("MainWindow", "Reset", 0, QApplication::UnicodeUTF8));
    actionReset->setShortcut(QApplication::translate("MainWindow", "End", 0, QApplication::UnicodeUTF8));
    actionRectilinear->setText(QApplication::translate("MainWindow", "rectilinear", 0, QApplication::UnicodeUTF8));
    actionFisheye->setText(QApplication::translate("MainWindow", "fisheye", 0, QApplication::UnicodeUTF8));
    actionCylindrical->setText(QApplication::translate("MainWindow", "cylindrical", 0, QApplication::UnicodeUTF8));
    actionEquirectangular->setText(QApplication::translate("MainWindow", "equirectangular", 0, QApplication::UnicodeUTF8));
    actionHemispherical->setText(QApplication::translate("MainWindow", "hemispheres", 0, QApplication::UnicodeUTF8));
    actionCube_faces->setText(QApplication::translate("MainWindow", "cube faces", 0, QApplication::UnicodeUTF8));
    actionQuit->setText(QApplication::translate("MainWindow", "Quit", 0, QApplication::UnicodeUTF8));
    actionQTVR->setText(QApplication::translate("MainWindow", "QTVR", 0, QApplication::UnicodeUTF8));
    actionPT_script->setText(QApplication::translate("MainWindow", "PT script", 0, QApplication::UnicodeUTF8));
    menu_View->setTitle(QApplication::translate("MainWindow", "&View", 0, QApplication::UnicodeUTF8));
    menuLoad->setTitle(QApplication::translate("MainWindow", "&Picture", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
