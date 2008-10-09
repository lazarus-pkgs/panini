/********************************************************************************
** Form generated from reading ui file 'picTypeDialog.ui'
**
** Created: Thu Oct 9 12:21:39 2008
**      by: Qt User Interface Compiler version 4.4.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_PICTYPEDIALOG_H
#define UI_PICTYPEDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>

QT_BEGIN_NAMESPACE

class Ui_picTypeDialog
{
public:
    QDialogButtonBox *buttonBox;
    QComboBox *typesBox;
    QLabel *nameLabel;

    void setupUi(QDialog *picTypeDialog)
    {
    if (picTypeDialog->objectName().isEmpty())
        picTypeDialog->setObjectName(QString::fromUtf8("picTypeDialog"));
    picTypeDialog->resize(346, 117);
    buttonBox = new QDialogButtonBox(picTypeDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(-10, 81, 341, 31));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    typesBox = new QComboBox(picTypeDialog);
    typesBox->setObjectName(QString::fromUtf8("typesBox"));
    typesBox->setGeometry(QRect(20, 50, 311, 22));
    nameLabel = new QLabel(picTypeDialog);
    nameLabel->setObjectName(QString::fromUtf8("nameLabel"));
    nameLabel->setGeometry(QRect(20, 10, 311, 21));
    nameLabel->setFrameShape(QFrame::Panel);
    nameLabel->setFrameShadow(QFrame::Sunken);

    retranslateUi(picTypeDialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), picTypeDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), picTypeDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(picTypeDialog);
    } // setupUi

    void retranslateUi(QDialog *picTypeDialog)
    {
    picTypeDialog->setWindowTitle(QApplication::translate("picTypeDialog", "pvQt - Choose Picture Type", 0, QApplication::UnicodeUTF8));
    nameLabel->setText(QApplication::translate("picTypeDialog", "TextLabel", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(picTypeDialog);
    } // retranslateUi

};

namespace Ui {
    class picTypeDialog: public Ui_picTypeDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PICTYPEDIALOG_H
