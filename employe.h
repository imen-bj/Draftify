#ifndef EMPLOYE_H
#define EMPLOYE_H
#include<Qstring>
#include<QSqlQuery>
#include<QSqlQueryModel>
#include <QDateTime>
#include <QStringList>
#include <QVector>


class Employe
{
    int ID_EMPL;
    QString NOM;
    QString PRENOM;
    QString MAIL;
    QString POSTE;
    QString ADRESSE;
    int CIN;
    int NUM_TEL;
    int SALARY;
    QString ROLE;

public:
    Employe();
    Employe(int,QString,QString,QString,QString,QString,int,int,int,QString);
    //getters
    int getID_EMPL(){return ID_EMPL;}
    QString getNOM(){return NOM;}
    QString getPRENOM(){return PRENOM;}
    QString getMAIL(){return MAIL;}
    QString getPOSTE(){return POSTE;}
    QString getADRESSE(){return ADRESSE;}
    QString getROLE(){return ROLE;}
    int  getCIN(){return CIN;}
    int  getNUM_TEL(){return NUM_TEL;}
    int  getSALARY(){return SALARY;}

    //setters
    void setID_EMPL(int id){ID_EMPL=id;}
    void setNOM(QString N){NOM=N;}
    void setPRENOM(QString PN){PRENOM=PN;}
    void setMAIL(QString M){MAIL=M;}
    void setPOSTE(QString PST){POSTE=PST;}
    void setADRESSE(QString ADR){ADRESSE=ADR;}
    void setCIN(int cin){CIN=cin;}
    void setNUM_TEL(int num){NUM_TEL=num;}
    void setSALARY(int sl){SALARY=sl;}
    void setROLE(QString rl){ROLE= rl;}
    //fonctions
    bool ajouter();
    QSqlQueryModel * afficher();
    QSqlQueryModel * rechercher(const QString&);
    bool supprimer(int);
    //metiers classiques
    QSqlQueryModel* trier(const QString& critere);
    static bool exportCsvFromModel(const QAbstractItemModel* model,const QString& filePath,QChar sep = ';');   // use ';' (better for many Excel locales)
    static void clearWidget(QWidget* w);
    static void getSalaireParPoste(QStringList &posts, QVector<int> &totals);
    //signUp
    bool isEmailExist(const QString &email);
    bool validatePassword(const QString &password);
    bool saveSecurityQuestion(const QString &email, const QString &question, const QString &answer);
    bool savePassword(const QString &email, const QString &password);
    bool isPasswordExist(const QString &email);
    //login
    QString getPasswordByEmail(const QString &email);
    QString getRoleByEmail(const QString &email);
    //forgot password
    QString getSecurityQuestion(const QString &email);
    bool validateSecurityQuestion(const QString &email, const QString &question);
    bool validateSecurityAnswer(const QString &email, const QString &answer);
    bool updatePassword(const QString& email, const QString& newPassword);
};

#endif // EMPLOYE_H
