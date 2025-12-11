/********************************************************************************
** Form generated from reading UI file 'clientidialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENTIDIALOG_H
#define UI_CLIENTIDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_ClientIDialog
{
public:
    QLineEdit *lineEditId;
    QLabel *labelInfo;

    void setupUi(QDialog *ClientIDialog)
    {
        if (ClientIDialog->objectName().isEmpty())
            ClientIDialog->setObjectName("ClientIDialog");
        ClientIDialog->resize(529, 300);
        ClientIDialog->setStyleSheet(QString::fromUtf8(" background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"                                stop:0 #C4B5FD,\n"
"                                stop:1 #93C5FD);"));
        lineEditId = new QLineEdit(ClientIDialog);
        lineEditId->setObjectName("lineEditId");
        lineEditId->setGeometry(QRect(70, 190, 371, 61));
        lineEditId->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #FFFFFF;       /* white input background */\n"
"    color: #1E293B;                  /* dark text for clarity */\n"
"    font-size: 13px;\n"
"    padding: 6px 10px;\n"
"\n"
"    border: 1px solid #C4B5FD;       /* thin violet border */\n"
"    border-radius: 8px;              /* smooth rounded corners */\n"
"}\n"
""));
        labelInfo = new QLabel(ClientIDialog);
        labelInfo->setObjectName("labelInfo");
        labelInfo->setGeometry(QRect(50, 40, 411, 131));

        retranslateUi(ClientIDialog);

        QMetaObject::connectSlotsByName(ClientIDialog);
    } // setupUi

    void retranslateUi(QDialog *ClientIDialog)
    {
        ClientIDialog->setWindowTitle(QCoreApplication::translate("ClientIDialog", "Dialog", nullptr));
        labelInfo->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ClientIDialog: public Ui_ClientIDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENTIDIALOG_H
