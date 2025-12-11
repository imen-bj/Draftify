#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Client
{
private:
    int ID_CL;
    QString NOM;
    QString PRENOM;
    QString EMAIL;
    QString PHONE;
    QString SOCIAL;
    QString TYPE;
    QString FOLLOWERS;
    QString PHOTO;

public:
    // Constructeurs
    Client();
    Client(int, QString, QString, QString, QString, QString, QString, QString, QString);

    // Getters
    int getID_CL() const { return ID_CL; }
    QString getNOM() const { return NOM; }
    QString getPRENOM() const { return PRENOM; }
    QString getEMAIL() const { return EMAIL; }
    QString getPHONE() const { return PHONE; }
    QString getSOCIAL() const { return SOCIAL; }
    QString getTYPE() const { return TYPE; }
    QString getFOLLOWERS() const { return FOLLOWERS; }

    // Setters
    void setID_CL(int id) { ID_CL = id; }
    void setNOM(QString n) { NOM = n; }
    void setPRENOM(QString p) { PRENOM = p; }
    void setEMAIL(QString e) { EMAIL = e; }
    void setPHONE(QString ph) { PHONE = ph; }
    void setSOCIAL(QString s) { SOCIAL = s; }
    void setTYPE(QString t) { TYPE = t; }
    void setFOLLOWERS(QString f) { FOLLOWERS = f; }
    void setPHOTO(QString p) { PHOTO = p; }

    // Getters
    QString getPHOTO() const { return PHOTO; }

    // Fonctions CRUD
    bool ajouter();
    QSqlQueryModel *afficher();
    QSqlQueryModel *rechercher(const QString &critere);
    bool supprimer(int id);
    bool modifier();
};

#endif // CLIENT_H
