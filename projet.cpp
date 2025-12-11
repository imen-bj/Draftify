#include "projet.h"
#include <QSqlError>
#include <QDebug>
#include<QDate>
#include<QMap>

Projet::Projet() : ID_PROJ(0), ID_CL(0) {}

Projet::Projet(int id, const QString &type, const QString &deadline, const QString &statut, int id_cl)
    : ID_PROJ(id), TYPE_PROJET(type), DEADLINE(deadline), STATUT(statut), ID_CL(id_cl) {}

// Getters
int Projet::getID_PROJ() const { return ID_PROJ; }
QString Projet::getTYPE_PROJET() const { return TYPE_PROJET; }
QString Projet::getDEADLINE() const { return DEADLINE; }
QString Projet::getSTATUT() const { return STATUT; }
int Projet::getID_CL() const { return ID_CL; }

// Setters
void Projet::setID_PROJ(int id) { ID_PROJ = id; }
void Projet::setTYPE_PROJET(const QString& t) { TYPE_PROJET = t; }
void Projet::setDEADLINE(const QString& d) { DEADLINE = d; }
void Projet::setSTATUT(const QString& s) { STATUT = s; }
void Projet::setID_CL(int cl) { ID_CL = cl; }

// AJOUTER
bool Projet::ajouter()
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO PROJET (ID_PROJ, TYPE_PROJET, DEADLINE, STATUT, ID_CL)
        VALUES (:id, :type, TO_DATE(:deadline, 'YYYY-MM-DD'), :statut, :id_cl)
    )");

    query.bindValue(":id", ID_PROJ);
    query.bindValue(":type", TYPE_PROJET);
    query.bindValue(":deadline", DEADLINE);
    query.bindValue(":statut", STATUT);
    query.bindValue(":id_cl", ID_CL);

    bool success = query.exec();
    if (!success)
    {
        qDebug() << "❌ Erreur ajout projet:" << query.lastError().text();
    }
    return success;
}

// AFFICHER
QSqlQueryModel* Projet::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(R"(
        SELECT ID_PROJ, TYPE_PROJET, TO_CHAR(DEADLINE, 'YYYY-MM-DD') AS DEADLINE, STATUT, ID_CL
        FROM PROJET ORDER BY ID_PROJ
    )");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Deadline"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Statut"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Client ID"));

    return model;
}

// ⭐⭐ CORRECTION ICI - RECHERCHER PAR TYPE ⭐⭐
/*QSqlQueryModel* Projet::rechercherParType(const QString &type)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr = QString(
                           "SELECT ID_PROJ, TYPE_PROJET, TO_CHAR(DEADLINE, 'YYYY-MM-DD') AS DEADLINE, STATUT, ID_CL "
                           "FROM PROJET WHERE LOWER(TYPE_PROJET) LIKE LOWER('%%1%')"
                           ).arg(type);

    model->setQuery(queryStr);
    return model;
}*/

// SUPPRIMER
bool Projet::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM PROJET WHERE ID_PROJ = :id");
    query.bindValue(":id", id);
    return query.exec();
}

// MODIFIER
bool Projet::modifier(int id)
{
    // Convertir la deadline QString → QDate
    QDate dateDeadline = QDate::fromString(DEADLINE, "yyyy-MM-dd");

    // Liste des mots qui signifient "Terminé"
    QString statutLower = STATUT.toLower();
    bool estFini = (statutLower == "finished" ||
                    statutLower == "terminé" ||
                    statutLower == "terminée" ||
                    statutLower == "done" ||
                    statutLower == "completed");

    // 🔥 RÈGLE IMPORTANTE : interdiction si deadline future
    if (estFini && dateDeadline > QDate::currentDate()) {
        qDebug() << "ERREUR : impossible de mettre Finished avec deadline future !";
        return false;
    }

    QSqlQuery query;
    query.prepare(R"(
        UPDATE PROJET
        SET TYPE_PROJET = :type,
            DEADLINE = TO_DATE(:deadline, 'YYYY-MM-DD'),
            STATUT = :statut,
            ID_CL = :id_cl
        WHERE ID_PROJ = :id
    )");

    query.bindValue(":type", TYPE_PROJET);
    query.bindValue(":deadline", DEADLINE);
    query.bindValue(":statut", STATUT);
    query.bindValue(":id_cl", ID_CL);
    query.bindValue(":id", id);

    return query.exec();
}


//**pour stats ( eli ml crud mch eli ml calendrier )**//
QMap<QString, int> Projet::getStatistiquesStatut()
{
    QMap<QString, int> stats;
    QSqlQuery query;

    query.prepare("SELECT STATUT, COUNT(*) FROM PROJET GROUP BY STATUT ORDER BY COUNT(*) DESC");

    if (query.exec()) {
        while (query.next()) {
            QString statut = query.value(0).toString();
            int count = query.value(1).toInt();
            stats[statut] = count;
        }
    }

    return stats;
}

int Projet::getTotalProjets()
{
    QSqlQuery query("SELECT COUNT(*) FROM PROJET");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int Projet::getProjetsEnRetard()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM PROJET WHERE DEADLINE < SYSDATE AND "
                  "(STATUT != 'Terminé' AND STATUT != 'Finished' AND STATUT != 'Complété')");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}


// for liste non volatile

bool Projet::updateTaskCompletionStatus(int taskId, bool completed) {
    QSqlQuery query;
    query.prepare("UPDATE PROJET_CHECKLIST SET EST_TERMINE = :est_termine WHERE ID_CHECK = :id_check");
    query.bindValue(":est_termine", completed ? 1 : 0);
    query.bindValue(":id_check", taskId);

    bool success = query.exec();
    qDebug() << "=== DEBUG == DB Update - Task:" << taskId << "Success:" << success;
    return success;
}

// =====================
// CHARGER PAR ID
// =====================

bool Projet::chargerDepuisId(int id)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT ID_PROJ,
               TYPE_PROJET,
               TO_CHAR(DEADLINE, 'YYYY-MM-DD') AS DEADLINE,
               STATUT,
               ID_CL
        FROM PROJET
        WHERE ID_PROJ = :id
    )");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur chargerDepuisId:" << query.lastError().text();
        return false;
    }

    if (!query.next())
        return false;

    ID_PROJ      = query.value("ID_PROJ").toInt();
    TYPE_PROJET  = query.value("TYPE_PROJET").toString();
    DEADLINE     = query.value("DEADLINE").toString();
    STATUT       = query.value("STATUT").toString();
    ID_CL        = query.value("ID_CL").toInt();

    return true;
}

// =====================
// RECHERCHE & TRI (STATIC)
// =====================

bool Projet::existeId(int id)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM PROJET WHERE ID_PROJ = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        qDebug() << "Erreur existeId:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() > 0;
}

QSqlQueryModel* Projet::rechercherParId(int id)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare(R"(
        SELECT ID_PROJ,
               TYPE_PROJET,
               TO_CHAR(DEADLINE, 'YYYY-MM-DD') AS DEADLINE,
               STATUT,
               ID_CL
        FROM PROJET
        WHERE ID_PROJ = :id
    )");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur rechercherParId:" << query.lastError().text();
    }

    model->setQuery(query);
    return model;
}

QSqlQueryModel* Projet::rechercherParType(const QString &type)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare(R"(
        SELECT ID_PROJ,
               TYPE_PROJET,
               TO_CHAR(DEADLINE, 'YYYY-MM-DD') AS DEADLINE,
               STATUT,
               ID_CL
        FROM PROJET
        WHERE LOWER(TYPE_PROJET) LIKE LOWER(:type)
    )");
    query.bindValue(":type", "%" + type + "%");

    if (!query.exec()) {
        qDebug() << "Erreur rechercherParType:" << query.lastError().text();
    }

    model->setQuery(query);
    return model;
}

QSqlQueryModel* Projet::trierParDeadlineAsc()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(R"(
        SELECT ID_PROJ,
               TYPE_PROJET,
               TO_CHAR(DEADLINE,'YYYY-MM-DD') AS DEADLINE,
               STATUT,
               ID_CL
        FROM PROJET
        ORDER BY DEADLINE ASC
    )");

    return model;
}

QSqlQueryModel* Projet::trierParDeadlineDesc()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(R"(
        SELECT ID_PROJ,
               TYPE_PROJET,
               TO_CHAR(DEADLINE,'YYYY-MM-DD') AS DEADLINE,
               STATUT,
               ID_CL
        FROM PROJET
        ORDER BY DEADLINE DESC
    )");

    return model;
}

