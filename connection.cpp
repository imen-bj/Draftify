#include "connection.h"

Connection::Connection()
{
}

Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

Connection& Connection::createInstance()
{
    static Connection instance;
    return instance; //retourne ref de cette instance
}


bool Connection::createconnect(){

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    bool test=false;
    db.setDatabaseName("draftify");//inserer le nom de la source de données
    db.setUserName("optime");//inserer nom de l'utilisateur
    db.setPassword("123");//inserer mot de passe de cet utilisateur

    if (db.open()){
         qDebug() << "conenction etablie";
        test=true;

    }
    else{
    qDebug() << "echec de la connection :" << db.lastError().text();
        return  test;}
}
