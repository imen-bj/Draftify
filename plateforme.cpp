#include "plateforme.h"
#include "connection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QComboBox>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QSqlDatabase>

plateforme::plateforme() {}

plateforme::plateforme(int idplat, const QString& nomPlat,
                       const QString& typePlat, const QString& photo,
                       int idCl, int nbAb, const QString& username)
{
    ID_PLAT = idplat;
    NOM_PLAT = nomPlat;
    TYPE_PLAT = typePlat;
    PHOTO = photo;
    ID_CL = idCl;
    NB_AB = nbAb;
    USERNAME = username;
}

static QSqlDatabase getConnection() {
    // Use the named connection we created in main.cpp
    QSqlDatabase db = QSqlDatabase::database("working_connection");

    // If the named connection doesn't exist, try default
    if (!db.isValid()) {
        db = QSqlDatabase::database();
    }

    return db;
}




bool plateforme::ajouterplat()
{
    QString name = NOM_PLAT.trimmed();
    QString type = TYPE_PLAT.trimmed();
    QString photo = PHOTO.trimmed();

    if (name.isEmpty()) {
        qDebug() << "ajouterplat: NOM_PLAT empty";
        return false;
    }

    QRegularExpression re("^[A-Za-z]+$");
    if (!re.match(name).hasMatch()) {
        qDebug() << "ajouterplat: NOM_PLAT invalid (letters only)";
        return false;
    }

    if (type.isEmpty()) {
        qDebug() << "ajouterplat: TYPE_PLAT empty";
        return false;
    }

    if (photo.isEmpty()) {
        qDebug() << "ajouterplat: PHOTO empty";
        return false;
    }

    QSqlQuery checkName(getConnection());
    checkName.prepare("SELECT COUNT(*) FROM PLATEFORME WHERE NOM_PLAT = :name");
    checkName.bindValue(":name", name);

    if (!checkName.exec()) {
        qDebug() << "ajouterplat: error checkName exec:" << checkName.lastError().text();
        return false;
    }

    checkName.next();
    if (checkName.value(0).toInt() > 0) {
        qDebug() << "ajouterplat: name already exists:" << name;
        return false;
    }

    QSqlQuery checkID(getConnection());
    checkID.prepare("SELECT COUNT(*) FROM PLATEFORME WHERE ID_PLAT = :id");
    checkID.bindValue(":id", ID_PLAT);

    if (!checkID.exec()) {
        qDebug() << "ajouterplat: error checkID exec:" << checkID.lastError().text();
        return false;
    }

    checkID.next();
    if (checkID.value(0).toInt() > 0) {
        qDebug() << "ajouterplat: ID already exists:" << ID_PLAT;
        return false;
    }

    QSqlQuery query(getConnection());
    query.prepare("INSERT INTO PLATEFORME (ID_PLAT, NOM_PLAT, TYPE_PLAT, PHOTO) "
                  "VALUES (:id, :nom, :type, :photo)");
    query.bindValue(":id", ID_PLAT);
    query.bindValue(":nom", name);
    query.bindValue(":type", type);
    query.bindValue(":photo", photo);

    if (!query.exec()) {
        qDebug() << "ajouterplat: INSERT failed:" << query.lastError().text();
        return false;
    }

    return true;
}

int plateforme::supprimerplat(int id, QString &deletedName)
{
    deletedName.clear();

    //  Use working connection instead of default
    QSqlQuery queryCheck(getConnection());
    queryCheck.prepare("SELECT NOM_PLAT FROM PLATEFORME WHERE ID_PLAT = :id");
    queryCheck.bindValue(":id", id);

    if (!queryCheck.exec()) {
        qDebug() << "supprimerplat: error SELECT:" << queryCheck.lastError().text();
        return -1;
    }

    if (!queryCheck.next()) {
        qDebug() << "supprimerplat: id not found:" << id;
        return 1;
    }

    deletedName = queryCheck.value(0).toString();

    QSqlQuery queryDelete(getConnection());
    queryDelete.prepare("DELETE FROM PLATEFORME WHERE ID_PLAT = :id");
    queryDelete.bindValue(":id", id);

    if (!queryDelete.exec()) {
        qDebug() << "supprimerplat: error DELETE PLATEFORME:" << queryDelete.lastError().text();
        deletedName.clear();
        return -1;
    }

    if (queryDelete.numRowsAffected() == 0) {
        qDebug() << "supprimerplat: DELETE affected 0 rows";
        deletedName.clear();
        return 1;
    }

    return 0;
}

bool plateforme::modifierplat(int id,
                              const QString &newName,
                              const QString &newType,
                              const QString &newPhoto)
{
    if (id <= 0) {
        qDebug() << "modifierplat: id invalide";
        return false;
    }

    QString nameTrim = newName.trimmed();
    QString typeTrim = newType.trimmed();
    QString photoTrim = newPhoto.trimmed();

    if (!nameTrim.isEmpty()) {
        QRegularExpression re("^[A-Za-z]+$");
        if (!re.match(nameTrim).hasMatch()) {
            qDebug() << "modifierplat: nom invalide (lettres uniquement)";
            return false;
        }

        //  Use working connection instead of default
        QSqlQuery checkName(getConnection());
        checkName.prepare("SELECT COUNT(*) FROM PLATEFORME WHERE NOM_PLAT = :name AND ID_PLAT <> :id");
        checkName.bindValue(":name", nameTrim);
        checkName.bindValue(":id", id);

        if (!checkName.exec()) {
            qDebug() << "modifierplat: erreur checkName:" << checkName.lastError().text();
            return false;
        }

        checkName.next();
        if (checkName.value(0).toInt() > 0) {
            qDebug() << "modifierplat: nom déjà utilisé";
            return false;
        }
    }

    QStringList setClauses;
    if (!nameTrim.isEmpty())
        setClauses << "NOM_PLAT = :nom";
    if (!typeTrim.isEmpty())
        setClauses << "TYPE_PLAT = :type";
    if (!photoTrim.isEmpty())
        setClauses << "PHOTO = :photo";

    if (setClauses.isEmpty()) {
        qDebug() << "modifierplat: aucun champ à modifier";
        return false;
    }

    QString sql = "UPDATE PLATEFORME SET " + setClauses.join(", ") + " WHERE ID_PLAT = :id";

    //  Use working connection instead of default
    QSqlQuery q(getConnection());
    q.prepare(sql);

    if (!nameTrim.isEmpty())
        q.bindValue(":nom", nameTrim);
    if (!typeTrim.isEmpty())
        q.bindValue(":type", typeTrim);
    if (!photoTrim.isEmpty())
        q.bindValue(":photo", photoTrim);
    q.bindValue(":id", id);

    if (!q.exec()) {
        qDebug() << "modifierplat: erreur UPDATE:" << q.lastError().text();
        return false;
    }

    if (q.numRowsAffected() == 0) {
        qDebug() << "modifierplat: aucun enregistrement mis à jour";
        return false;
    }

    return true;
}


QSqlQueryModel* plateforme::afficherplat()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    QSqlQuery query(getConnection());
    query.exec("SELECT ID_PLAT, NOM_PLAT, TYPE_PLAT, PHOTO FROM PLATEFORME");
    model->setQuery(query);

    return model;
}


bool plateforme::chargerParId(int id, QString &name, QString &type, QString &photo)
{
    name.clear();
    type.clear();
    photo.clear();

    QSqlQuery q(getConnection());
    q.prepare("SELECT NOM_PLAT, TYPE_PLAT, PHOTO FROM PLATEFORME WHERE ID_PLAT = :id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        qDebug() << "chargerParId: SQL error:" << q.lastError().text();
        return false;
    }

    if (!q.next()) {
        qDebug() << "chargerParId: no platform with id" << id;
        return false;
    }

    name = q.value(0).toString();
    type = q.value(1).toString();
    photo = q.value(2).toString();

    return true;
}


bool plateforme::isUserOnPlatform(int clientId, int platId)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT COUNT(*) FROM INSCRIPTION WHERE ID_CL = :clientId AND ID_PLAT = :platId");
    query.bindValue(":clientId", clientId);
    query.bindValue(":platId", platId);

    if (!query.exec()) {
        qDebug() << "Error checking if user is on platform:" << query.lastError().text();
        return false;
    }

    query.next();
    return query.value(0).toInt() > 0;
}


bool plateforme::addUserToPlatform(int clientId, int platId, const QString &username, int subCount)
{
    if (isUserOnPlatform(clientId, platId)) {
        return false;
    }

    QSqlQuery query(getConnection());
    query.prepare("INSERT INTO INSCRIPTION (ID_CL, ID_PLAT, USERNAME, NB_AB) "
                  "VALUES (:clientId, :platId, :username, :subCount)");
    query.bindValue(":clientId", clientId);
    query.bindValue(":platId", platId);
    query.bindValue(":username", username);
    query.bindValue(":subCount", subCount);

    if (!query.exec()) {
        qDebug() << "Error adding user to platform:" << query.lastError().text();
        return false;
    }

    return true;
}

int plateforme::generateUniqueID()
{
    while (true) {
        int id = QRandomGenerator::global()->bounded(100000000, 999999999);
        QSqlQuery q(getConnection());
        q.prepare("SELECT COUNT(*) FROM PLATEFORME WHERE ID_PLAT = :id");
        q.bindValue(":id", id);

        if (q.exec()) {
            q.next();
            if (q.value(0).toInt() == 0)
                return id;
        }
    }
    return -1;
}

int plateforme::getPlatformIdByName(const QString &platName)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName");
    query.bindValue(":platName", platName);

    if (!query.exec() || !query.next()) {
        qDebug() << "Error fetching platform ID:" << query.lastError().text();
        return -1;
    }

    return query.value(0).toInt();
}


void plateforme::populatePlatformComboBox(QComboBox *comboBox)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT NOM_PLAT FROM PLATEFORME");

    if (!query.exec()) {
        qDebug() << "Error fetching platform names:" << query.lastError().text();
        return;
    }

    comboBox->clear();
    while (query.next()) {
        QString platName = query.value(0).toString();
        comboBox->addItem(platName);
    }
}


void plateforme::populateClientComboBox(QComboBox *comboBox)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_CL FROM CLIENT");

    if (!query.exec()) {
        qDebug() << "Error fetching client IDs:" << query.lastError().text();
        return;
    }

    comboBox->clear();
    while (query.next()) {
        comboBox->addItem(query.value(0).toString());
    }
}


void plateforme::populateClientsTableView(int platId, QSqlQueryModel *model)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_CL, USERNAME, NB_AB FROM INSCRIPTION WHERE ID_PLAT = :platId");
    query.bindValue(":platId", platId);

    if (!query.exec()) {
        qDebug() << "Error fetching clients:" << query.lastError().text();
        return;
    }

    model->setQuery(query);
}


void plateforme::populateClientComboBoxForPlatform(QComboBox *comboBox, const QString &platName)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_CL FROM INSCRIPTION WHERE ID_PLAT = (SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName)");
    query.bindValue(":platName", platName);

    if (!query.exec()) {
        qDebug() << "Error fetching client IDs:" << query.lastError().text();
        return;
    }

    comboBox->clear();
    while (query.next()) {
        comboBox->addItem(query.value(0).toString());
    }
}


bool plateforme::updateClientOnPlatform(int clientId, const QString &platName, const QString &username, int subCount)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName");
    query.bindValue(":platName", platName);

    if (!query.exec() || !query.next()) {
        qDebug() << "Error fetching platform ID:" << query.lastError().text();
        return false;
    }

    int platId = query.value(0).toInt();

    QString updateQuery = "UPDATE INSCRIPTION SET ";
    bool first = true;

    if (!username.isEmpty()) {
        if (!first) updateQuery += ", ";
        updateQuery += "USERNAME = :username";
        first = false;
    }

    if (subCount > 0) {
        if (!first) updateQuery += ", ";
        updateQuery += "NB_AB = :subCount";
        first = false;
    }

    updateQuery += " WHERE ID_CL = :clientId AND ID_PLAT = :platId";

    query.prepare(updateQuery);
    if (!username.isEmpty()) query.bindValue(":username", username);
    if (subCount > 0) query.bindValue(":subCount", subCount);
    query.bindValue(":clientId", clientId);
    query.bindValue(":platId", platId);

    if (!query.exec()) {
        qDebug() << "Error executing update query:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<int> plateforme::getClientIdsByPlatform(const QString& platName)
{
    QList<int> clientIds;

    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_CL FROM INSCRIPTION WHERE ID_PLAT = (SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName)");
    query.bindValue(":platName", platName);

    if (!query.exec()) {
        qDebug() << "Error fetching client IDs for platform" << platName << ":" << query.lastError().text();
        return clientIds;
    }

    while (query.next()) {
        clientIds.append(query.value(0).toInt());
    }

    return clientIds;
}

void plateforme::getAllPlatforms(QList<QString>& platforms)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT NOM_PLAT FROM PLATEFORME");

    if (!query.exec()) {
        qDebug() << "Error fetching platform names:" << query.lastError().text();
        return;
    }

    platforms.clear();
    while (query.next()) {
        platforms.append(query.value(0).toString());
    }
}


void plateforme::getClientsForPlatform(const QString& platName, QList<QString>& clientIds)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_CL FROM INSCRIPTION WHERE ID_PLAT = (SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName)");
    query.bindValue(":platName", platName);

    if (!query.exec()) {
        qDebug() << "Error fetching client IDs:" << query.lastError().text();
        return;
    }

    clientIds.clear();
    while (query.next()) {
        clientIds.append(query.value(0).toString());
    }
}



bool plateforme::deleteClientFromPlatform(const QString& platName, const QString& clientId)
{
    QSqlQuery query(getConnection());
    query.prepare("DELETE FROM INSCRIPTION WHERE ID_PLAT = (SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName) AND ID_CL = :clientId");
    query.bindValue(":platName", platName);
    query.bindValue(":clientId", clientId);

    return query.exec();
}


bool plateforme::findPlatform(const QString& searchText, int& platId, QString& platName)
{
    QSqlQuery q(getConnection());
    bool okId = false;
    int idVal = searchText.toInt(&okId);

    if (okId) {
        q.prepare("SELECT ID_PLAT, NOM_PLAT FROM PLATEFORME WHERE ID_PLAT = :id");
        q.bindValue(":id", idVal);
    } else {
        int parenIndex = searchText.indexOf('(');
        QString nameOnly = (parenIndex > 0) ? searchText.left(parenIndex).trimmed() : searchText;
        q.prepare("SELECT ID_PLAT, NOM_PLAT FROM PLATEFORME WHERE LOWER(NOM_PLAT) = LOWER(:name)");
        q.bindValue(":name", nameOnly);
    }

    if (!q.exec() || !q.next()) {
        return false;
    }

    platId = q.value(0).toInt();
    platName = q.value(1).toString();
    return true;
}



QStringList plateforme::getPlatformSuggestions()
{
    QStringList items;

    QSqlQuery q(getConnection());
    q.prepare("SELECT ID_PLAT, NOM_PLAT FROM PLATEFORME");

    if (!q.exec()) {
        qDebug() << "Error fetching platform suggestions:" << q.lastError().text();
        return items;
    }

    while (q.next()) {
        QString id = q.value(0).toString();
        QString name = q.value(1).toString();
        items << QString("%1 (%2)").arg(name, id);
    }

    return items;
}


QVector<QPair<QString, int>> plateforme::getPlatformTypeStatistics()
{
    QVector<QPair<QString, int>> platformTypesCounts;

    QSqlQuery query(getConnection());
    query.prepare("SELECT TYPE_PLAT, COUNT(*) AS count FROM PLATEFORME GROUP BY TYPE_PLAT ORDER BY count DESC");

    if (!query.exec()) {
        qDebug() << "Error fetching platform type statistics:" << query.lastError().text();
        return platformTypesCounts;
    }

    while (query.next()) {
        QString type = query.value("TYPE_PLAT").toString();
        int count = query.value("count").toInt();
        platformTypesCounts.append(qMakePair(type, count));
    }

    return platformTypesCounts;
}


QSqlQueryModel* plateforme::getAllPlatforms()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_PLAT, NOM_PLAT, TYPE_PLAT FROM PLATEFORME");
    query.exec();

    model->setQuery(query);
    return model;
}


QSqlQueryModel* plateforme::getClientsForPlatform(int platId)
{
    QSqlQueryModel *model = new QSqlQueryModel();

    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_CL, USERNAME, NB_AB FROM INSCRIPTION WHERE ID_PLAT = :platId");
    query.bindValue(":platId", platId);

    if (!query.exec()) {
        qDebug() << "Error fetching client data:" << query.lastError().text();
        return model;
    }

    model->setQuery(query);
    return model;
}


QSqlQueryModel* plateforme::getTopClientsForPlatform(int platId)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT * FROM ("
                  "SELECT c.PHOTO_CL, i.USERNAME FROM CLIENT c "
                  "JOIN INSCRIPTION i ON c.ID_CL = i.ID_CL "
                  "WHERE i.ID_PLAT = :platId "
                  "ORDER BY i.NB_AB DESC"
                  ") WHERE ROWNUM <= 3");
    query.bindValue(":platId", platId);

    QSqlQueryModel *model = new QSqlQueryModel();
    if (!query.exec()) {
        qDebug() << "Error fetching top clients for podium:" << query.lastError().text();
        return nullptr;
    }

    model->setQuery(query);
    return model;
}


QList<QString> plateforme::getOtherPlatforms(int clientId, int currentPlatformId)
{
    QList<QString> otherPlatforms;

    QSqlQuery query(getConnection());
    query.prepare("SELECT p.NOM_PLAT FROM PLATEFORME p JOIN INSCRIPTION i ON p.ID_PLAT = i.ID_PLAT WHERE i.ID_CL = :clientId AND p.ID_PLAT != :currentPlatformId");
    query.bindValue(":clientId", clientId);
    query.bindValue(":currentPlatformId", currentPlatformId);

    if (!query.exec()) {
        qDebug() << "Error fetching other platforms:" << query.lastError().text();
        return otherPlatforms;
    }

    while (query.next()) {
        otherPlatforms.append(query.value(0).toString());
    }

    return otherPlatforms;
}


bool plateforme::getUserDetails(int clientId, QString &photo, QString &firstName, QString &lastName, int &subCount, int &oldSubCount, double &growthRate, QString &email, QStringList &otherPlatforms)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT * FROM CLIENT WHERE ID_CL = :clientId");
    query.bindValue(":clientId", clientId);

    if (query.exec() && query.next()) {
        firstName = query.value("NOM").toString();
        lastName = query.value("PRENOM").toString();
        subCount = query.value("FOLLOWERS").toInt();
        oldSubCount = query.value("FOLLOWERS").toInt();
        photo = query.value("PHOTO_CL").toString();
        email = query.value("EMAIL").toString();

        otherPlatforms.clear();
        QSqlQuery platformQuery(getConnection());
        platformQuery.prepare("SELECT PLATFORM_NAME FROM OTHER_PLATFORMS WHERE CLIENT_ID = :clientId");
        platformQuery.bindValue(":clientId", clientId);

        if (platformQuery.exec()) {
            while (platformQuery.next()) {
                otherPlatforms.append(platformQuery.value(0).toString());
            }
        }

        return true;
    }

    return false;
}


QSqlQueryModel* plateforme::getClientDetails(int clientId, const QString& platName)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_CL, USERNAME, NB_AB FROM INSCRIPTION WHERE ID_PLAT = (SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName) AND ID_CL = :clientId");
    query.bindValue(":platName", platName);
    query.bindValue(":clientId", clientId);

    if (!query.exec()) {
        qDebug() << "Error fetching client data:" << query.lastError().text();
        return nullptr;
    }

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(query);
    return model;
}


bool plateforme::updateClientInfo(int clientId, const QString& platName, const QString& newUsername, int newSubCount)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName");
    query.bindValue(":platName", platName);

    if (!query.exec() || !query.next()) {
        qDebug() << "Error fetching platform ID for" << platName << ":" << query.lastError().text();
        return false;
    }

    int platId = query.value(0).toInt();

    QString updateQuery = "UPDATE INSCRIPTION SET NB_AB = :subCount WHERE ID_CL = :clientId AND ID_PLAT = :platId";
    query.prepare(updateQuery);
    query.bindValue(":subCount", newSubCount);
    query.bindValue(":clientId", clientId);
    query.bindValue(":platId", platId);

    if (!query.exec()) {
        qDebug() << "Error executing update query:" << query.lastError().text();
        return false;
    }

    return true;
}


int plateforme::getCurrentSubscriberCount(const QString& platName, int clientId)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT NB_AB FROM INSCRIPTION WHERE ID_CL = :clientId AND ID_PLAT = (SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName)");
    query.bindValue(":clientId", clientId);
    query.bindValue(":platName", platName);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}


QString plateforme::getCurrentUsername(const QString& platName, int clientId)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT USERNAME FROM INSCRIPTION WHERE ID_CL = :clientId AND ID_PLAT = (SELECT ID_PLAT FROM PLATEFORME WHERE NOM_PLAT = :platName)");
    query.bindValue(":clientId", clientId);
    query.bindValue(":platName", platName);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return "";
}


QVector<QPair<QString, int>> plateforme::getMostUsedPlatforms()
{
    QVector<QPair<QString, int>> platformUsage;

    QSqlQuery query(getConnection());
    query.prepare("SELECT p.NOM_PLAT, COUNT(i.ID_CL) as client_count FROM PLATEFORME p LEFT JOIN INSCRIPTION i ON p.ID_PLAT = i.ID_PLAT GROUP BY p.NOM_PLAT, p.ID_PLAT HAVING COUNT(i.ID_CL) > 0 ORDER BY client_count DESC");

    if (!query.exec()) {
        qDebug() << "Error fetching most used platforms:" << query.lastError().text();
        return platformUsage;
    }

    while (query.next()) {
        QString platName = query.value(0).toString();
        int clientCount = query.value(1).toInt();

        if (clientCount > 0) {
            platformUsage.append(qMakePair(platName, clientCount));
        }
    }

    return platformUsage;
}


int plateforme::getTotalUsersForPlatform(int platId)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT COUNT(*) FROM INSCRIPTION WHERE ID_PLAT = :platId");
    query.bindValue(":platId", platId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int plateforme::getTotalSubscribersForPlatform(int platId)
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT SUM(NB_AB) FROM INSCRIPTION WHERE ID_PLAT = :platId");
    query.bindValue(":platId", platId);

    if (query.exec() && query.next()) {
        QVariant val = query.value(0);
        return val.isNull() ? 0 : val.toInt();
    }

    return 0;
}


QSqlQueryModel* plateforme::getAllPlatformsSorted(const QString &sortBy)
{
    QSqlQueryModel *model = new QSqlQueryModel();

    QSqlQuery query(getConnection());
    QString sql = QString("SELECT * FROM PLATEFORME ORDER BY %1 ASC").arg(sortBy);
    query.exec(sql);

    model->setQuery(query);
    return model;
}



int plateforme::getPlatformCount()
{
    QSqlQuery query(getConnection());
    query.prepare("SELECT COUNT(*) FROM PLATEFORME");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}


QList<plateforme::PlatformButtonData> plateforme::getPlatformButtonsData()
{
    QList<PlatformButtonData> result;

    QSqlQuery query(getConnection());
    query.prepare("SELECT p.ID_PLAT, p.NOM_PLAT, p.PHOTO, COUNT(i.ID_CL) as USER_COUNT FROM PLATEFORME p LEFT JOIN INSCRIPTION i ON p.ID_PLAT = i.ID_PLAT GROUP BY p.ID_PLAT, p.NOM_PLAT, p.PHOTO ORDER BY p.NOM_PLAT ASC");

    if (query.exec()) {
        while (query.next()) {
            PlatformButtonData data;
            data.id = query.value(0).toInt();
            data.name = query.value(1).toString();
            data.photo = query.value(2).toString();
            data.userCount = query.value(3).toInt();
            result.append(data);
        }
    }

    return result;
}

QList<plateforme::PlatformButtonData> plateforme::getPlatformsSortedBy(const QString &sortType)
{
    QList<PlatformButtonData> result;

    QString orderBy;
    if (sortType == "Most Used") {
        orderBy = "USER_COUNT DESC";
    } else if (sortType == "Least Used") {
        orderBy = "USER_COUNT ASC";
    } else {
        orderBy = "p.ID_PLAT ASC";
    }

    QSqlQuery query(getConnection());
    QString sql = QString("SELECT p.ID_PLAT, p.NOM_PLAT, p.PHOTO, COUNT(i.ID_CL) as USER_COUNT FROM PLATEFORME p LEFT JOIN INSCRIPTION i ON p.ID_PLAT = i.ID_PLAT GROUP BY p.ID_PLAT, p.NOM_PLAT, p.PHOTO ORDER BY %1").arg(orderBy);
    query.prepare(sql);

    if (query.exec()) {
        while (query.next()) {
            PlatformButtonData data;
            data.id = query.value(0).toInt();
            data.name = query.value(1).toString();
            data.photo = query.value(2).toString();
            data.userCount = query.value(3).toInt();
            result.append(data);
        }
    }

    return result;
}


plateforme::ClientDetails plateforme::getClientDetails(const QString& clientId)
{
    ClientDetails details;

    QSqlQuery query(getConnection());
    query.prepare("SELECT EMAIL, PHOTO_CL FROM CLIENT WHERE ID_CL = :id");
    query.bindValue(":id", clientId);

    if (query.exec() && query.next()) {
        details.email = query.value(0).toString();
        details.photoPath = query.value(1).toString();
    }

    return details;
}


plateforme::ClientDetails plateforme::getClientDetailsExcludingPlatform(const QString& clientId, int excludePlatId)
{
    ClientDetails details;

    QSqlQuery query(getConnection());
    query.prepare("SELECT EMAIL, PHOTO_CL FROM CLIENT WHERE ID_CL = :id");
    query.bindValue(":id", clientId);

    if (query.exec() && query.next()) {
        details.email = query.value(0).toString();
        details.photoPath = query.value(1).toString();
    }

    QSqlQuery platformQuery(getConnection());
    platformQuery.prepare("SELECT p.NOM_PLAT FROM PLATEFORME p JOIN INSCRIPTION i ON p.ID_PLAT = i.ID_PLAT WHERE i.ID_CL = :id AND p.ID_PLAT != :excludeId");
    platformQuery.bindValue(":id", clientId);
    platformQuery.bindValue(":excludeId", excludePlatId);

    if (platformQuery.exec()) {
        while (platformQuery.next()) {
            details.otherPlatforms.append(platformQuery.value(0).toString());
        }
    }

    return details;
}
