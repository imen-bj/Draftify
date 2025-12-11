#ifndef PROJET_H
#define PROJET_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QMap>  // AJOUTÉ POUR LES STATISTIQUES

class Projet
{
public:
    Projet();
    Projet(int id, const QString& type, const QString& deadline, const QString& statut, int id_cl);

    // Getters / Setters
    int getID_PROJ() const;
    QString getTYPE_PROJET() const;
    QString getDEADLINE() const;
    QString getSTATUT() const;
    int getID_CL() const;

    void setID_PROJ(int id);
    void setTYPE_PROJET(const QString& t);
    void setDEADLINE(const QString& d);
    void setSTATUT(const QString& s);
    void setID_CL(int cl);

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();                       // affiche tous
    static QSqlQueryModel* rechercherParType(const QString &type);
    bool supprimer(int id);
    bool modifier(int id); // met à jour le projet d'id
    // STATISTIQUES - AJOUTÉES ICI
    static QMap<QString, int> getStatistiquesStatut();
    static int getTotalProjets();
    static int getProjetsEnRetard();
    bool updateTaskCompletionStatus(int idCheck, bool isCompleted);
    bool chargerDepuisId(int id);
    static bool existeId(int id);
    static QSqlQueryModel* rechercherParId(int id);
    static QSqlQueryModel* trierParDeadlineAsc();
    static QSqlQueryModel* trierParDeadlineDesc();



private:
    int ID_PROJ;
    QString TYPE_PROJET;
    QString DEADLINE;     // Format "YYYY-MM-DD" en interne
    QString STATUT;
    int ID_CL;

};

#endif // PROJET_H
