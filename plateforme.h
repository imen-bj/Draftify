#ifndef PLATEFORME_H
#define PLATEFORME_H
#include<Qstring>
#include<QSqlQuery>
#include<QSqlQueryModel>
#include<QGraphicsView>
#include<QComboBox>



class plateforme
{

    int ID_PLAT;
    QString NOM_PLAT;
    QString TYPE_PLAT;
    QString PHOTO;

    int ID_CL;
    int NB_AB;
    QString USERNAME;
    // In plateforme.h - public section




public:
    plateforme();
    plateforme(int idplat, const QString& nomPlat, const QString& typePlat, const QString& photo,int idCl, int nbAb, const QString& username);

    int getidplat(){return ID_PLAT;}
    void setidplat(int idp){ID_PLAT=idp;}

    QString getnomplat(){return NOM_PLAT;}
    void setnomplat(QString np){NOM_PLAT=np;}

    QString gettypeplat(){return TYPE_PLAT;}
    void settypeplat(QString tp){TYPE_PLAT=tp;}

    QString getphoto(){return PHOTO;}
    void setphoto(QString p){PHOTO=p;}

    int getidcl(){return ID_CL;}
    void setidcl(int idc){ID_CL=idc;}

    int getnbab(){return NB_AB;}
    void setnbab(int na){NB_AB=na;}

    QString getusername(){return USERNAME;}
    void setUsername(QString us){USERNAME=us;}

    bool ajouterplat();
    QSqlQueryModel * afficherplat();
    int supprimerplat(int id, QString &deletedName);
    bool modifierplat(int id,const QString &newName, const QString &newType, const QString &newPhoto);
    bool chargerParId(int id, QString &name, QString &type, QString &photo);
    bool isUserOnPlatform(int clientId, int platId);
    bool addUserToPlatform(int clientId, int platId, const QString &username, int subCount);
    void createPlatformTypeStatistics(QGraphicsView *graphicsView);
    int generateUniqueID();
    int getPlatformIdByName(const QString &platName);  // Get platform ID by name
    void populatePlatformComboBox(QComboBox *comboBox);  // Populate platform combo boxes
    void populateClientComboBox(QComboBox *comboBox);    // Populate client combo box
    void populateClientsTableView(int platId, QSqlQueryModel *model);  // Declare the method here
    void populateClientComboBoxForPlatform(QComboBox *comboBox, const QString &platName);  // Fetch client IDs for a platform
    bool updateClientOnPlatform(int clientId, const QString &platName, const QString &username, int subCount);
    QList<int> getClientIdsByPlatform(const QString& platName);
    void getAllPlatforms(QList<QString>& platforms);
    void getClientsForPlatform(const QString& platName, QList<QString>& clientIds);
    bool deleteClientFromPlatform(const QString& platName, const QString& clientId);
    bool findPlatform(const QString& searchText, int& platId, QString& platName);
    QStringList getPlatformSuggestions();
    QVector<QPair<QString, int>> getPlatformTypeStatistics();
    QVector<QPair<QString, int>> getMostUsedPlatforms();  // Returns platform name + client count (excluding 0)

    QSqlQueryModel* getAllPlatforms();
    QSqlQueryModel* getClientsForPlatform(int platId);
    QSqlQueryModel* getTopClientsForPlatform(int platId);

    int getPreviousSubscriberCount(int clientId, int currentPlatformId);
    QStringList getOtherPlatforms(int clientId, int currentPlatformId);
    bool getUserDetails(int clientId, QString &photo, QString &firstName, QString &lastName, int &subCount, int &oldSubCount, double &growthRate, QString &email, QStringList &otherPlatforms);
    QSqlQueryModel* getClientDetails(int clientId, const QString& platName);
    bool updateClientInfo(int clientId, const QString& platName, const QString& newUsername, int newSubCount);
    int getCurrentSubscriberCount(const QString& platName, int clientId);
    QString getCurrentUsername(const QString& platName, int clientId);
    double calculateGrowthRate(int clientId, int currentPlatformId, int currentSubCount);
    struct ClientDetails {
        QString email;
        QString photoPath;
        QStringList otherPlatforms;
    };
    ClientDetails getClientDetails(const QString& clientId);
    ClientDetails getClientDetailsExcludingPlatform(const QString& clientId, int excludePlatId);
    // In plateforme.h - add these to the public section
    int getTotalUsersForPlatform(int platId);
    int getTotalSubscribersForPlatform(int platId);
    QSqlQueryModel* getAllPlatformsSorted(const QString &sortBy = "ID_PLAT");
    int getPlatformCount();


    struct PlatformButtonData {
        int id;
        QString name;
        QString photo;
        int userCount;
    };
    QList<PlatformButtonData> getPlatformButtonsData();
    QList<PlatformButtonData> getPlatformsSortedBy(const QString &sortType);


};

#endif // PLATEFORME_H
