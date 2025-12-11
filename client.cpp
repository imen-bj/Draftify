#include "client.h"
#include <QSqlQuery>
#include <QVariant>
#include <QObject>
#include <QDebug>
#include <QSqlError>


Client::Client() {}

// Nouveau constructeur avec 9 paramètres
Client::Client(int id, QString n, QString p, QString email, QString phone, QString social, QString type, QString followers, QString photo)
{
    ID_CL = id;
    NOM = n;
    PRENOM = p;
    EMAIL = email;
    PHONE = phone;
    SOCIAL = social;
    TYPE = type;
    FOLLOWERS = followers;
    PHOTO = photo;
}

bool Client::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO CLIENT (ID_CL, NOM, PRENOM, EMAIL, PHONE, SOCIAL, TYPE, FOLLOWERS, PHOTO_CL) "
                  "VALUES (:id, :nom, :prenom, :email, :phone, :social, :type, :followers, :photo)");

    query.bindValue(":id", ID_CL);
    query.bindValue(":nom", NOM);
    query.bindValue(":prenom", PRENOM);
    query.bindValue(":email", EMAIL);
    query.bindValue(":phone", PHONE);
    query.bindValue(":social", SOCIAL);
    query.bindValue(":type", TYPE);
    query.bindValue(":followers", FOLLOWERS);
    query.bindValue(":photo", PHOTO);
    if (!query.exec()) {
        qDebug() << "Erreur SQL lors de l'ajout du client:" << query.lastError().text();
        return false;
    }
    return true;

    return query.exec();
}

QSqlQueryModel *Client::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM CLIENT");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Réseau social"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Followers"));

    return model;
}

QSqlQueryModel *Client::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr =
        QString("SELECT * FROM CLIENT WHERE NOM LIKE '%%1%' OR PRENOM LIKE '%%1%' "
                "OR EMAIL LIKE '%%1%' OR PHONE LIKE '%%1%' OR SOCIAL LIKE '%%1%' "
                "OR TYPE LIKE '%%1%' OR FOLLOWERS LIKE '%%1%'")
            .arg(critere);

    model->setQuery(queryStr);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Réseau social"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Followers"));

    return model;
}

bool Client::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CLIENT WHERE ID_CL = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool Client::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE CLIENT SET NOM = :nom, PRENOM = :prenom, EMAIL = :email, PHONE = :phone, "
                  "SOCIAL = :social, TYPE = :type, FOLLOWERS = :followers, PHOTO_CL = :photo WHERE ID_CL = :id");

    query.bindValue(":id", ID_CL);
    query.bindValue(":nom", NOM);
    query.bindValue(":prenom", PRENOM);
    query.bindValue(":email", EMAIL);
    query.bindValue(":phone", PHONE);
    query.bindValue(":social", SOCIAL);
    query.bindValue(":type", TYPE);
    query.bindValue(":followers", FOLLOWERS);
    query.bindValue(":photo", PHOTO);

    return query.exec();
}
