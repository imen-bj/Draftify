#include "draftify.h"
#include <QApplication>
#include "connection.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Call your friend's connection (even though it has a bug)
    Connection& c = Connection::createInstance();
    c.createconnect();

    // Now create a proper connection that will persist
    // Use a named connection to avoid conflicts
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "working_connection");
    db.setDatabaseName("draftify");
    db.setUserName("optime");
    db.setPassword("123");
    bool test = db.open();

    if (!test) {
        qDebug() << "Connection failed:" << db.lastError().text();
    }

    Draftify w;
    QPixmap Draftify(":/resources/Draftify.png");
    QIcon fb(":/res/facebook.png");
    QIcon insta(":/res/instagram.png");
    QIcon tt(":/res/tiktok.png");
    QIcon twit(":/res/twitter.png");
    QPixmap pod(":/res/pod.png");
    QPixmap profile(":/res/profile.png");
    QPixmap welcm(":/image/welcome.png");

    if(test) {
        w.show();
        QMessageBox::information(nullptr, QObject::tr("database is open"),
                                 QObject::tr("connection successful.\n"
                                             "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else {
        QMessageBox::critical(nullptr, QObject::tr("database is not open"),
                              QObject::tr("connection failed.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    }

    return a.exec();
}
