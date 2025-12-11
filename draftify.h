#ifndef DRAFTIFY_H
#define DRAFTIFY_H
#include "qlabel.h"
#include <QMainWindow>
#include <employe.h>
#include "client.h"
#include <projet.h>
#include <plateforme.h>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QComboBox>  // ← AJOUTER CETTE INCLUDE
#include <QBuffer>

// Animation
#include <QVariantAnimation>
#include "qlistwidget.h"

//arduino nash
#include <QSerialPort>
#include <QJsonDocument>
#include <QJsonObject>
#include "clientidialog.h"

//arduino nahna
#include "arduino.h"










QT_BEGIN_NAMESPACE
namespace Ui {
class Draftify;
}
QT_END_NAMESPACE

//arduino nash
class ArduinoRating;


class Draftify : public QMainWindow
{
    Q_OBJECT

public:
    Draftify(QWidget *parent = nullptr);
    ~Draftify();
protected:
    bool eventFilter(QObject *obj, QEvent *event) ;
    void keyPressEvent(QKeyEvent *event);

private slots:
    void readRfid();
    void on_linkBtn_clicked();

    void on_nexttostatEmpl_clicked();

    void on_backtoemployeeEmpl_clicked();


    void on_LoginEmpl_clicked();

    void on_NextSecurityQstsEmpl_clicked();

    void on_NextRecoverAccountEmpl_clicked();

    void on_LogoutSideBarEmpl_clicked();

    void on_LogoutSideBarEmpl_2_clicked();


    void on_HomeSideBarEmpl_clicked();


    void on_LogoutSideBarEmpl_3_clicked();




    void on_employeeSideBarEmpl_clicked();

    void on_HomeSideBarEmpl_2_clicked();

    void on_HomeSideBarEmpl_3_clicked();

    void on_EmployeeSideBarEmpl_clicked();

    void on_employeeSideBarEmpl_2_clicked();

    void on_Button_Client_clicked();

    void on_cl_btn_emp_clicked();

    void on_cl_btn_home_clicked();

    void on_cl_btn_emp2_clicked();

    void on_Button_Emp_clicked();

    void on_Button_Home_clicked();

    void on_platyas_clicked();

    void on_cl_btnyas_clicked();



    void on_EmployeeSideBarEmplyas_clicked();

    void on_toolButton_20_clicked();

    void on_toolButton_27_clicked();

    void on_toolButton_29_clicked();

    void on_LogoutSideBarEmplyas_clicked();

    void on_HomeSideBarEmplyas_clicked();

    void on_Button_plat_clicked();

    void on_nexttostatProjet_clicked();

    void on_toolButton_19_clicked();

    void on_backtoprojetProjet_clicked();








    void on_Button_Pro_clicked();

    void on_toolButtonyas_clicked();


    void on_HomeSideBarProjet_clicked();

    void on_employeeSideBarProjet_clicked();

    void on_client_sidebar_projet_clicked();

    void on_ProjetSideBarProjet_clicked();

    void on_PlatformesSideBarProjet_clicked();

    void on_LogoutSideBarProjet_clicked();

    void on_toolButton_28_clicked();

    void on_toolButton_26_clicked();



//CRUD EMPL__________________________________________
    void on_ConfirmAddEmpl_clicked();

    void on_DeleteEmpl_clicked();

    void on_SearchEmpl_clicked();

    void on_tableView_EMPL_doubleClicked(const QModelIndex &index);

    void on_EditEmpl_clicked();

    void on_SaveEditEmpl_clicked();

    void on_CancelEditEmpl_clicked();

    void on_ListOfEmployeesEmpl_clicked();
    // crud projet
    void on_ConfirmAddProjet_clicked();
    void on_ConfirmUpdateProjet_clicked();
    void on_confirmshowprojet_clicked();
    void on_DeleteProjet_clicked();
    void on_tableWidgetProjet_cellClicked(int row, int col);
    //new for crud projet mta el search
    void on_lineEditSearchProjet_textChanged(const QString &text);
    // new for crud projet mta sort
    void on_EditProjet_clicked();
    void on_SearchProjet_clicked();
    void on_ConfirmTriProjet_clicked();  // Add this line
    void on_PDF_ExportProjet_clicked();  // Add this line
    //calendrier
    void onCalendarDateSelected();  // ← AJOUTE CE SLOT
    // CHECKLIST - SLOTS
    void onAutoChecklistClicked();
    //void onChecklistItemClicked(QListWidgetItem *item);
    void onChecklistItemChanged(QListWidgetItem *item);  // Slot for item state change

    //void onProjectComboChanged();  // ← AJOUTER CETTE LIGNE
    // stats slots
    void on_btnStats_clicked();
    // ⬇️⬇️⬇️ AJOUTER CES DEUX LIGNES ⬇️⬇️⬇️
    void onProjectComboChanged();

    //void on_comboBoxProjetParDate_currentIndexChanged(int index);
    // Quand l'utilisateur change de projet dans le ComboBox
    void on_comboBoxproj_currentIndexChanged(int index);

    void loadChecklistForSelectedProject(int projetId);
    // Charger la checklist du projet sélectionné







    //CRUD CL
    void on_Button_sub_add_cli_clicked();
    void on_Button_cancel_add_cli_clicked();
    void on_edit_tab_clicked();
    void on_supp_tab_clicked();
    void on_tableWidgetcl_clicked(const QModelIndex &index);
    void on_tableWidgetcl_cellClicked(int row, int column);
    void on_Button_supprimer_cli_clicked();
    //crud nash
    void on_importaddplat_clicked();
    void on_addplat_clicked();
    void on_deleteplat_clicked();
    void on_importeditplat_clicked();
    void on_editplat_clicked();
    void refreshPlatformButtons();
    void onPlatformButtonClicked();
    void on_aedapp_clicked();
    void on_addapp_clicked();
    void populatePlatformComboBox();    // Declare the function here
    void populateClientComboBox();      // Declare the function here
    void on_cplatuseredit_currentIndexChanged(int index);
    void on_editapp_clicked();
    void populateClientComboBoxForEdit();
    void populatePlatformComboBoxForEdit();
    void populateClientComboBoxForDelete();
    void on_deleteapp_clicked();
    void populatePlatformComboBoxForDelete();
    void populateClientComboBoxForEditAndDelete();
    void clearForm1();
    void on_cplatuserdelete_currentIndexChanged(int index);
    void populateClientsTableView(int platId);



    void on_butfindplat_clicked();
    void setupPlatformSearch();      // fills completer for findapp
    void openPlatformById(int platId); // shows platform page + table for given ID
    void onPlatformButtonClickedFromId(int id);
    void on_sortPlat_clicked();

    void on_statplatbut_clicked();
    //void on_statplatPageActivated();
    void createPlatformTypeStatistics(QGraphicsView* typeplatstat);
    void generateAllPlatformsReport();

    void on_exportplat_clicked();
    void updatePodiumWithClientPics(int platId);
    void setPodiumImage(QLabel* label, const QString& imagePath);
    void onPlatformSelectionChanged();
    void on_cplatuseradd_currentIndexChanged(int index);
    void on_tableapp_doubleClicked(const QModelIndex &index);
    void showUserDetails(int clientId);
    void showUserInfoDialog(const QString &photo, const QString &firstName, const QString &lastName, int subCount, const QStringList &otherPlatforms, const QString &email);
    void createMostUsedPlatformStatistics(QGraphicsView* usedplatstat);
    void styleTableApp();
    QWidget* createStatCard(const QString &icon, const QString &label, const QString &value, const QColor &color);

//METIERS EMPL________________________________________________________

    void on_comboBoxSortEmpl_currentIndexChanged(int index);

    void on_PDF_ExportEmpl_clicked();

    void on_comboBoxPostEmpl_currentIndexChanged(int index);

    void on_StatEmpl_clicked();

    void on_NextForgotPasswordEmpl_clicked();

    void on_SignupEmpl_clicked();

    void on_ConfirmSecurityQstEmpl_clicked();

    void on_ShowPasswordSignUpEmpl_stateChanged(int arg1);

    void on_ShowPasswordLoginEmpl_stateChanged(int arg1);

    void on_ConfirmPickQuestionEmpl_clicked();

    void on_ConfirmAnswerQstRecoverEmpl_clicked();

    void on_ShowPasswordChangePasswordEmpl_stateChanged(int arg1);
    void on_retournerstat_clicked();
    void on_btn_stat_clicked();
    void on_btn_rating_clicked();
    void on_Button_sub_tri_clicked();
    void trierClientFollowers(int index);
    void on_pushButton_19_clicked();
      void on_enreg_tab_clicked();
    void on_returnplat_clicked();

      void on_returnapp_clicked();

    void on_adeplat_clicked();

      void on_importcl_clicked();

  private:
    Ui::Draftify *ui;
    QString serialBuffer;
    Employe Etmp;
    Arduino A;
    Client Ctmp;
    plateforme p;

    bool    m_isEditing = false;   // are we in edit mode?
    QString m_editingIdOld;        // original ID (for WHERE)
    int m_currentPlatformId = -1;

    QLabel* hoverTooltip;  // Custom tooltip widget
    bool m_isEditingClient;
    QString m_editingIdClientOld;
    int m_editingClientId = -1; // identifiant du client qu'on modifie
    int m_selectedClientId = -1;
    QString selectedPhotoPath;

    //projet
    Projet projet;

    void chargerClients();
    void actualiserTable();
    void viderChamps();
    void chargerProjetDansFormulaire(int id);
    void setProjetModelToTable(QSqlQueryModel *model);
    //calendrier
    // CALENDRIER - DÉCLARATIONS CORRIGÉES
    void loadProjectsFromDatabase();
    void colorCodeProject(const QDate &deadline, const QString &status); // ← GARDE CETTE VERSION
    void displayProjectsInList(const QDate &selectedDate);

    // AJOUTE CES DEUX NOUVELLES FONCTIONS
    void colorCodeProjectImproved(const QDate &deadline, const QString &status, const QString &projectType);
    QString getStatusEmoji(const QString &status, const QDate &deadline);
    void updateCalendarTooltips(const QMap<QDate, QList<QString>> &projectsMap); // ← AJOUTE CETTE LIGNE
    ////****************************///////////////************************//////////////
    // CHECKLIST - VARIABLES TEMPORAIRES
    int currentChecklistProjectId = -1;
    QString currentChecklistProjectType = "";
    //CHECKLIST - FONCTIONS
    void generateChecklistForProject(int projectId, const QString &projectType);
    void displayChecklist(int projectId);
    void updateChecklistStats(int projectId);
    void on_listWidgetChecklist_itemChanged(QListWidgetItem *item);
    void updateChecklistStats();
    void checkAndUpdateProjectStatus(int projectId);


    QMap<QString, QStringList> loadChecklistTemplates();  // Déclaration de la fonction
    //QComboBox *comboProjects;      // ← AJOUTER CETTE LIGNE
    // AJOUTE cette ligne dans la section private :
    //void updateChecklistStats();



    // stats
    void mettreAJourStats();
    /** 🔹 Génère le QChart animé dans widgetStatsChart */
    void afficherStatsProjets();

    /** 🔹 Animation des barres (0 → valeur) */
    void animateBarSetproj(QBarSet *set);


    QComboBox *comboBoxproj;
    // ekher hope
    // void repositionnerElementsChecklist(); // 🔥 AJOUTER CETTE LIGNE
    //new ajouter
    // --- Fonctions utilitaires internes ---
    void chargerClientsDansTable();
    void refreshPlatformCombos();

    void fillFormFromRow(int row);
    void clearClientForm();
    void clearForm();         // si utilisée (sinon supprime l'appel dans .cpp)

    void randomGenerator();   // si tu l'appelles dans .cpp
    void fillClientFormFromRow(int row);

    QPixmap drawPieChartPixmap(const QMap<QString,int>& counts, const QSize &size);
    void statParTypeClients();
    void chargerTableClients();
    void loadAllClients();
    void on_lineEdit_19_textChanged(const QString &text);




    //metiers EMPL____
    void afficherStatsSalaireParPoste();
    void animateBarSet(QBarSet *set);  // if you keep QtCharts:: prefix


    //nash arduino
    ArduinoRating *arduinoRating = nullptr;
    ClientIDialog *idDialog = nullptr;
    void reloadClientTable();
    Client clientManager;

};
#endif // DRAFTIFY_H
