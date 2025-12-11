#include "client.h"
#include "connection.h"
#include "draftify.h"
#include "employe.h"
#include "plateforme.h"
#include "qgraphicseffect.h"
#include "ui_draftify.h"
#include <QBuffer>
#include <QDir>
#include <QEnterEvent>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QIcon>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMessageBox>       // pour QMessageBox::information / critical
#include <QObject>
#include <QPageSize>
#include <QPainter>
#include <QPainterPath>
#include <QPdfWriter>
#include <QPieSeries>
#include <QPixmap>
#include <QPrinter>

#include <QPropertyAnimation>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSize>
#include <QSqlError>
#include <QSqlQuery>         // utile si tu utilises des requêtes SQL dans Employe
#include <QSqlQueryModel>    // pour QSqlQueryModel (afficher les données)
#include <QString>           // pour QString et ses méthodes (.trimmed(), .isEmpty(), etc.)
#include <QStringConverter>  // For Qt6 UTF-8 encoding
#include <QStringListModel>
#include <QTableView>        // pour ui->tableView_employes
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QTextStream>
#include <QVariantAnimation>
#include <QVector>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>
#include <plateforme.h>
#include <projet.h>
#include <qtooltip.h>
#include<QCompleter>
#include<QDebug>
#include<QEasingCurve>
#include<QFileDialog>
#include<QLabel>
//nash arduino
#include "arduinorating.h"






Draftify::Draftify(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Draftify)
{
    ui->setupUi(this);

    ui->stackedWidgetEmpl->setCurrentIndex(0);

    //arduino setup
    // =============== ARDUINO + RFID – FINAL WORKING VERSION ===============
    int ret = A.connect_arduino();
    switch(ret) {
    case 0:
        qDebug() << "Arduino connected successfully on" << A.getarduino_port_name();

        // Clear any old garbage
        A.getserial()->clear();
        serialBuffer.clear();        // ← HERE EXACTLY

        // Connect the signal (must be after port is open)
        connect(A.getserial(), &QSerialPort::readyRead, this, &Draftify::readRfid);

        QMessageBox::information(this, "RFID Ready", "Arduino connected!\nYou can now scan your card.");
        break;

    case 1:
        QMessageBox::critical(this, "Error", "Port failed to open!");
        break;

    case -1:
        QMessageBox::critical(this, "Error", "Arduino not detected!");
        break;
    }
    // =====================================================================



    //arduino stuuufff
    // dialog for ID input
    idDialog = new ClientIDialog(this);
    arduinoRating = new ArduinoRating(this);
    arduinoRating->setPortName("COM8");

    arduinoRating->init(idDialog);   // this runs the serial setup
    connect(arduinoRating, &ArduinoRating::ratingSaved,
            this, [this](int clientId, int rating) {
                Q_UNUSED(clientId);
                Q_UNUSED(rating);
                reloadClientTable();
            });




    // Populate sorting options in the combo box
    ui->csort->addItem("Most Used");
    ui->csort->addItem("Least Used");
    ui->csort->addItem("Default");

    connect(ui->csort, SIGNAL(currentIndexChanged(int)), this, SLOT(on_sortPlat_clicked()));

    // Populate platform combo boxes for various actions
    populatePlatformComboBoxForEdit();
    populatePlatformComboBoxForDelete();
    connect(ui->returnplat, &QPushButton::clicked, this, &Draftify::on_returnplat_clicked);
    connect(ui->cplatuseredit, SIGNAL(currentIndexChanged(int)), this, SLOT(populateClientComboBoxForEdit()));
    connect(ui->cplatuserdelete, SIGNAL(currentIndexChanged(int)), this, SLOT(populateClientComboBoxForDelete()));

    // Initialize platform and client data
    populatePlatformComboBox();
    populateClientComboBox();

    // Trigger initial podium update based on the selected platform
    int platId = p.getPlatformIdByName(ui->cplatuseradd->currentText());
    if (platId != -1) {
        updatePodiumWithClientPics(platId);  // Initial update based on selected platform
    }


    //add nash plat
    ui->ctypeplatadd->addItem("social networking");
    ui->ctypeplatadd->addItem("media sharing");
    ui->ctypeplatadd->addItem("microblogging");
    ui->ctypeplatadd->addItem("professional networking");
    ui->ctypeplatadd->addItem("streaming");
    ui->ctypeplatadd->setCurrentIndex(-1);

    //edit nash plat
    ui->ctypeeditplat->addItem("social networking");
    ui->ctypeeditplat->addItem("media sharing");
    ui->ctypeeditplat->addItem("microblogging");
    ui->ctypeeditplat->addItem("professional networking");
    ui->ctypeeditplat->addItem("streaming");
    ui->ctypeeditplat->setCurrentIndex(-1);

    //nash stuff
    QVector<QPushButton*> platButtons = {
        ui->app1,  ui->app2,  ui->app3,  ui->app4,  ui->app5,
        ui->app6,  ui->app7,  ui->app8,  ui->app9,  ui->app10,
        ui->app11, ui->app12, ui->app13, ui->app14, ui->app15
    };

    for (QPushButton *btn : platButtons) {
        connect(btn, &QPushButton::clicked,
                this, &Draftify::onPlatformButtonClicked);
    }



    // CRÉER le combobox
    comboBoxproj = new QComboBox(this);
    comboBoxproj->setVisible(false);
    comboBoxproj->setMinimumWidth(300);
    connect(ui->comboBoxproj, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onProjectComboChanged(int)));
    //ekher amal
    //connect(ui->listWidgetChecklist, &QListWidget::itemChanged, this, &Draftify::onChecklistItemChanged);
    // 🔥 AJOUTER CES INITIALISATIONS CRITIQUES :
    currentChecklistProjectId = -1;

    // Connexions des signaux
    connect(ui->comboBoxproj, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Draftify::on_comboBoxproj_currentIndexChanged);

    connect(ui->listWidgetChecklist, &QListWidget::itemChanged,
            this, &Draftify::onChecklistItemChanged);

    // Chargement initial
    loadProjectsFromDatabase();

    qDebug() << "=== DEBUG == Constructeur Draftify initialisé";

    //connect(ui->listWidgetChecklist, &QListWidget::itemChanged,
    //this, &::Draftify::onChecklistItemClicked);




    // 🔥 NE PAS POSITIONNER ICI - on le fera dans onCalendarDateSelected()
    // comboBoxproj->move(...); ← SUPPRIMER CETTE LIGNE

    // CONNECTER
    //connect(comboBoxproj, SIGNAL(currentIndexChanged(int)),
    //this, SLOT(onProjectComboChanged()));

    // 🔥 S'ASSURER QUE LE BOUTON EST VISIBLE ET CONNECTÉ
    if (ui->btnAutoChecklist) {
        ui->btnAutoChecklist->setVisible(true);
        connect(ui->btnAutoChecklist, &QPushButton::clicked,
                this, &Draftify::onAutoChecklistClicked);
        qDebug() << "=== DEBUG == Bouton initialisé dans constructeur";
    }
    // 🔹 Hide stats frame at startup
    ui->frameStats->hide();
    //ui->textEdit->hide();   // optional if textEdit is inside frameStats
    // Dans Draftify::Draftify() - AJOUTE :
    connect(ui->calendarWidgetprojet, &QCalendarWidget::clicked,
            this, &Draftify::onCalendarDateSelected);
    // 🔥 S'ASSURER QUE LE BOUTON EST VISIBLE ET CONNECTÉ AU DÉBUT
    if (ui->btnAutoChecklist) {
        ui->btnAutoChecklist->setVisible(true);
        connect(ui->btnAutoChecklist, &QPushButton::clicked,
                this, &Draftify::onAutoChecklistClicked);
        qDebug() << "=== DEBUG == Bouton connecté et visible dans constructeur";
    }


    if (ui->listWidgetChecklist) {
        connect(ui->listWidgetChecklist, &QListWidget::itemClicked,
                this, &::Draftify::onChecklistItemChanged);
    }
    // calendrier
    // CONNEXION CALENDRIER
    // calendrier - CONNEXION CORRECTE
    connect(ui->calendarWidgetprojet, &QCalendarWidget::selectionChanged,
            this, &Draftify::onCalendarDateSelected);

    // Charger et colorier les projets EXISTANTS au démarrage
    loadProjectsFromDatabase();








    // Clear the comboBox selection after applying the sort
    ui->comboBoxSortEmpl->setCurrentIndex(-1);  // Set the comboBox to no selection (empty)
    ui->comboBoxPostEmpl->setCurrentIndex(-1);
    ui->comboBoxQstsEmpl->setCurrentIndex(-1);
    ui->lineEditSalaryEmpl->clear();    // Set the comboBox to no selection (empty)

    // === TABLE PROJET ===
    ui->tableWidgetProjet->setColumnCount(5);
    ui->tableWidgetProjet->setHorizontalHeaderLabels(
        {"ID", "Type", "Deadline", "Statut", "Client ID"});
    ui->tableWidgetProjet->setSelectionBehavior(QAbstractItemView::SelectRows);

    // === COMBOS PROJET ===
    ui->comboBox_type->addItems({"Vidéo", "Image", "Audio", "Web"});
    ui->comboBox_Projet->addItems({"En attente", "En cours", "Terminé"});

    // === CHARGER DONNÉES PROJET ===
    chargerClients();
    actualiserTable();

    // === CONNEXION CLIC LIGNE PROJET ===
    connect(ui->tableWidgetProjet, &QTableWidget::cellClicked,
            this, &Draftify::on_tableWidgetProjet_cellClicked);

    connect(ui->tableWidgetcl, &QTableWidget::cellClicked,
            this, &Draftify::on_tableWidgetcl_cellClicked);
     chargerClientsDansTable();

    ui->tableView_EMPL->setModel(Etmp.afficher());
    // Sélectionner la ligne entière dans le QTableView
    ui->tableView_EMPL->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_EMPL->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_EMPL->setEditTriggers(QAbstractItemView::NoEditTriggers); // no other actions to edit
    //fixer le size du tableau
    ui->tableView_EMPL->setColumnWidth(0, 120);
    ui->tableView_EMPL->setColumnWidth(1, 120);
    ui->tableView_EMPL->setColumnWidth(2, 150);
    ui->tableView_EMPL->setColumnWidth(3, 140);
    ui->tableView_EMPL->setColumnWidth(4, 120);
    ui->tableView_EMPL->setColumnWidth(5, 140);
    ui->tableView_EMPL->setColumnWidth(6, 130);
    ui->tableView_EMPL->setColumnWidth(7, 140);
    ui->tableView_EMPL->setColumnWidth(8, 127);


    // Installer un event filter sur toute la fenêtre
    plateforme p;
    QSqlQueryModel *defaultModel = p.getAllPlatformsSorted("ID_PLAT");
    refreshPlatformButtons();
    setupPlatformSearch();


    // ===== CREATE CUSTOM HOVER TOOLTIP =====
    hoverTooltip = new QLabel(this);
    hoverTooltip->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    hoverTooltip->setAttribute(Qt::WA_TranslucentBackground);
    hoverTooltip->hide();

    // Enable mouse tracking for table
    ui->tableapp->setMouseTracking(true);
    ui->tableapp->viewport()->setMouseTracking(true);

    // Install event filter for hover detection
    ui->tableapp->viewport()->installEventFilter(this);

}

Draftify::~Draftify()
{
    delete ui;
}




void Draftify::on_linkBtn_clicked()
{
    // Step 1: Get the email entered by the user
    QString email = ui->EmailIDLoginEmpl->text().trimmed();

    // Step 2: Validate if the email is entered
    if (email.isEmpty()) {
        QMessageBox::warning(this, "Email Required", "Please enter an email address.");
        return;  // Stop if email is empty
    }

    // Step 3: Check if the email exists in the database
    Employe e;
    QString question = e.getSecurityQuestion(email);

    if (question.isEmpty()) {
        // If no security question is found for the email, show a warning
        QMessageBox::warning(this, "Email Not Found", "This email is not registered.");
        return;
    }

    // Step 4: Get a random position in the combo box (excluding the last position)
    int randomIndex = QRandomGenerator::global()->bounded(ui->comboBoxQstsEmpl->count() + 1);  // Random index between 0 and count

    // Step 5: Insert the security question at the random position
    ui->comboBoxQstsEmpl->insertItem(randomIndex, question);

    // Step 6: Clear the email input field
    ui->EmailIDLoginEmpl->clear();
    ui->PasswordLoginEmpl->clear();

    // Step 7: Change the stacked widget to the next page
    ui->stackedWidgetEmpl->setCurrentIndex(2);
}

void Draftify::readRfid()
{
    QByteArray data = A.read_from_arduino();
    if (data.isEmpty()) return;

    serialBuffer += QString::fromLatin1(data);

    // Look for complete line ending with \n
    int newlineIndex;
    while ((newlineIndex = serialBuffer.indexOf('\n')) != -1) {
        QString line = serialBuffer.left(newlineIndex);
        serialBuffer.remove(0, newlineIndex + 1);

        QString uid = line.trimmed().toUpper();
        qDebug() << "Full UID received:" << uid;

        if (uid.length() >= 11 && uid.contains(" ")) {
            QMessageBox::information(this, "Card Scanned", "UID: " + uid);
            A.process_uid(uid);
            return;  // one card per scan
        }
    }
}


void Draftify::on_nexttostatEmpl_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(6);
}




void Draftify::on_backtoemployeeEmpl_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(5);
}




void Draftify::on_LoginEmpl_clicked()
{
    QString email = ui->EmailIDLoginEmpl->text().trimmed();
    QString password = ui->PasswordLoginEmpl->text().trimmed();

    // Step 1: Get the password associated with the email
    Employe e;
    QString storedPassword = e.getPasswordByEmail(email);  // Get password from the database

    if (storedPassword.isEmpty()) {
        // Email not found, show a warning
        QMessageBox::warning(this, "Login Failed", "Invalid email or password. Please try again.");
        return;
    }

    // Step 2: Validate the entered password
    if (storedPassword != password) {
        // Password mismatch, show a warning
        QMessageBox::warning(this, "Login Failed", "Incorrect password. Please try again.");
        return;
    }

    // Step 3: Get the role associated with the email
    QString role = e.getRoleByEmail(email);

    // Step 4: Store the role and enable or disable access to CRUD operations based on the role
    QString currentRole = role;

    // Step 5: Based on the role, grant or deny access to CRUD operations
    if (role == "Admin") {
        // Admin has full CRUD access
        ui->DeleteEmpl->setEnabled(true);
        ui->EditEmpl->setEnabled(true);
        ui->ConfirmAddEmpl->setEnabled(true);  // Admin can add employees
        ui->SaveEditEmpl->setEnabled(true);
        ui->CancelEditEmpl->setEnabled(true);
        ui->tableView_EMPL->setEnabled(true); // Admin can see the employee table

    } else if (role == "Manager") {
        // Manager can Edit and View, but can't Add or Delete
        ui->DeleteEmpl->setEnabled(false);  // Disable delete empl
        ui->supp_tab->setEnabled(false);  // Disable delete client
        ui->DeleteProjet->setEnabled(false); //disable delete projet

        ui->deleteapp->setEnabled(false); //disable delete fb

        ui->ConfirmAddEmpl->setEnabled(false);  // Disable add empl
        ui->Button_sub_add_cli->setEnabled(false);  // Disable add client
        ui->Button_cancel_add_cli->setEnabled(false);  // Disable add client
        ui->ConfirmAddProjet->setEnabled(false);  // Disable add projet

        ui->addapp->setEnabled(false);  // Disable add fb


        // Disable buttons edit and add empl and client
        ui->EditEmpl->setEnabled(false);
        ui->SaveEditEmpl->setEnabled(false);
        ui->PDF_ExportEmpl->setEnabled(false);
        ui->edit_tab->setEnabled(false);
        ui->line_id_add_cli->setEnabled(false);
        ui->line_type_add_cli->setEnabled(false);
        ui->line_name_add_cli->setEnabled(false);
        ui->line_last_add_cli->setEnabled(false);
        ui->line_email_add_cli->setEnabled(false);
        ui->line_phone_add_cli->setEnabled(false);
        ui->line_social_add_cli->setEnabled(false);
        ui->line_fllwrs_add_cli->setEnabled(false);
        ui->lineEditIDEmpl->setEnabled(false);
        ui->lineEditCINEmpl->setEnabled(false);
        ui->lineEditNameEmpl->setEnabled(false);
        ui->lineEditEmailEmpl->setEnabled(false);
        ui->lineEditAdressEmpl->setEnabled(false);
        ui->lineEditPhoneEmpl->setEnabled(false);
        ui->comboBoxPostEmpl->setEnabled(false);
        ui->lineEditSalaryEmpl->setEnabled(false);
        ui->lineEditRoleEmpl->setEnabled(false);



    } else if (role == "HR") {
        // HR Manager  access to employee and cant delete
        ui->DeleteEmpl->setEnabled(false);  // Disable delete

        ui->toolButton_19->setEnabled(false);
        ui->toolButton_20->setEnabled(false);
        ui->toolButton_26->setEnabled(false);
        ui->toolButton_27->setEnabled(false);
        ui->toolButton_28->setEnabled(false);
        ui->toolButton_29->setEnabled(false);
        ui->cl_btn_home->setEnabled(false);
        ui->cl_btn_emp->setEnabled(false);
        ui->cl_btn_emp2->setEnabled(false);



    } else if (role == "Client Service") {
        // Client service  access to client only and cant delete
        ui->supp_tab->setEnabled(false);  // Disable delete client

        ui->Button_Emp->setEnabled(false);
         ui->Button_Pro->setEnabled(false);
        ui->Button_plat->setEnabled(false);
        ui->toolButton_28->setEnabled(false);
        ui->toolButton_29->setEnabled(false);
        ui->employeeSideBarEmpl_2->setEnabled(false);


    } else if (role == "Employee") {
        // Employee can only see projet and manipulate it

        //home
         ui->employeeSideBarEmpl_2->setEnabled(false);//Empl
         ui->cl_btn_home->setEnabled(false); //Client
         ui->toolButton_29->setEnabled(false); //platforme
        // Projet
         ui->employeeSideBarProjet->setEnabled(false);//Empl
         ui->client_sidebar_projet->setEnabled(false);// Client
         ui->PlatformesSideBarProjet->setEnabled(false);// platforme






    }else if (role == "Social Media") {
    // Employee can only interact with platforme
    //home
    ui->employeeSideBarEmpl_2->setEnabled(false);  // Disable empl
    ui->cl_btn_home->setEnabled(false);    // Disable client
    ui->toolButton_28->setEnabled(false);  //Disable projet



    //platforme
    ui->EmployeeSideBarEmplyas->setEnabled(true);
    ui->cl_btnyas->setEnabled(true);
    ui->toolButtonyas->setEnabled(true);
    ui->toolButtonyas->setEnabled(false);  //Disable projet
    ui->cl_btnyas->setEnabled(false);  //Disable projet
    ui->EmployeeSideBarEmplyas->setEnabled(false);  //Disable projet

}


    ui->EmailIDLoginEmpl->clear();
    ui->PasswordLoginEmpl->clear();
    ui->stackedWidgetEmpl->setCurrentIndex(4);

}



void Draftify::on_NextSecurityQstsEmpl_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(0);
}


void Draftify::on_NextRecoverAccountEmpl_clicked()//back from recovery to login
{
    ui->EmailForgotPssswdEmpl->clear();
    ui->stackedWidgetEmpl->setCurrentIndex(0);
}


void Draftify::on_LogoutSideBarEmpl_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(0);
}


void Draftify::on_LogoutSideBarEmpl_2_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(0);
}




void Draftify::on_HomeSideBarEmpl_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(4);
    ui->HomeSideBarEmpl->setChecked(true);
    ui->employeeSideBarEmpl->setChecked(false);
}




void Draftify::on_LogoutSideBarEmpl_3_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(0);
}


void Draftify::on_employeeSideBarEmpl_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(5);
    ui->HomeSideBarEmpl->setChecked(false);
    ui->employeeSideBarEmpl->setChecked(true);
}


void Draftify::on_HomeSideBarEmpl_2_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(4);
    ui->HomeSideBarEmpl_2->setChecked(true);
    ui->employeeSideBarEmpl_2->setChecked(false);
}




void Draftify::on_HomeSideBarEmpl_3_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(4);
    ui->HomeSideBarEmpl_3->setChecked(true);
    ui->employeeSideBarEmpl->setChecked(false);
}


void Draftify::on_EmployeeSideBarEmpl_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(5);
    ui->HomeSideBarEmpl->setChecked(false);
    ui->employeeSideBarEmpl->setChecked(true);
}




void Draftify::on_employeeSideBarEmpl_2_clicked()
{
    ui->stackedWidgetEmpl->setCurrentIndex(5);
    ui->HomeSideBarEmpl_2->setChecked(false);
    ui->employeeSideBarEmpl_2->setChecked(true);
}


void Draftify::on_Button_Client_clicked()
{

    ui->stackedWidget_client->setCurrentIndex(0);


}


void Draftify::on_cl_btn_emp_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_Client);
     ui->stackedWidget_client->setCurrentIndex(0);
}


void Draftify::on_cl_btn_home_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_Client);
    ui->stackedWidget_client->setCurrentIndex(0);
}


void Draftify::on_cl_btn_emp2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_Client);
    ui->stackedWidget_client->setCurrentIndex(0);
}


void Draftify::on_Button_Emp_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_Employee);
    ui->stackedWidget_client->setCurrentIndex(5);
}


void Draftify::on_Button_Home_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_Employee);
    ui->stackedWidget_client->setCurrentIndex(4);
}







void Draftify::on_nexttostatProjet_clicked()
{

    ui->stackedWidgetProjet->setCurrentIndex(1);

}


void Draftify::on_toolButton_19_clicked()
{
     ui->stackedWidget->setCurrentWidget (ui->page_projet);
     ui->stackedWidgetProjet->setCurrentIndex(0);
}


void Draftify::on_backtoprojetProjet_clicked()
{
    ui->stackedWidgetProjet->setCurrentIndex(0);
}



void Draftify::on_toolButtonyas_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_projet);
    ui->stackedWidgetProjet->setCurrentIndex(0);

}


void Draftify::on_cl_btnyas_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Client);
    ui->stackedWidget_client->setCurrentIndex(0);
}

void Draftify::on_EmployeeSideBarEmplyas_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Employee);
    ui->stackedWidgetEmpl->setCurrentIndex(5);
}












void Draftify::on_Button_Pro_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_projet);
    ui->stackedWidgetProjet->setCurrentIndex(0);
}








void Draftify::on_HomeSideBarProjet_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Employee);
    ui->stackedWidgetProjet->setCurrentIndex(4);
}


void Draftify::on_employeeSideBarProjet_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Employee);
    ui->stackedWidgetProjet->setCurrentIndex(5);
}


void Draftify::on_client_sidebar_projet_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Client);
    ui->stackedWidgetProjet->setCurrentIndex(0);
}


void Draftify::on_ProjetSideBarProjet_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_projet);
    ui->stackedWidgetProjet->setCurrentIndex(1);
}


void Draftify::on_PlatformesSideBarProjet_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidgetProjet->setCurrentIndex(1);
}


void Draftify::on_LogoutSideBarProjet_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Employee);
    ui->stackedWidgetProjet->setCurrentIndex(0);
}


void Draftify::on_toolButton_28_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_projet);
    ui->stackedWidgetProjet->setCurrentIndex(0);
}



void Draftify::on_toolButton_26_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_projet);
    ui->stackedWidgetProjet->setCurrentIndex(0);
}

void Draftify::on_platyas_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(1);

}

void Draftify::on_adeplat_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(2);
}
void Draftify::on_HomeSideBarEmplyas_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Employee);
    ui->stackedWidgetEmpl->setCurrentIndex(4);

}

void Draftify::on_returnplat_clicked()
{
    qDebug() << "Button clicked, navigating to index 1";
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(1);

}



void Draftify::on_returnapp_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(1);
}
void Draftify::on_Button_plat_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(1);
}
void Draftify::on_toolButton_20_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(1);

}


void Draftify::on_toolButton_27_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(1);

}
void Draftify::on_toolButton_29_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(1);

}
void Draftify::on_LogoutSideBarEmplyas_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_Employee);
    ui->stackedWidgetEmpl->setCurrentIndex(0);

}



void Draftify::on_aedapp_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(3);

}

void Draftify::on_statplatbut_clicked()
{
    ui->stackedWidget->setCurrentWidget (ui->page_plateforme);
    ui->stackedWidget_plateforme->setCurrentIndex(4);
    createPlatformTypeStatistics(ui->typeplatstat);
    createMostUsedPlatformStatistics(ui->usedplatstat);

}


//CRUD EMPLOYE____________________________________________________________________________

//fonction ajouter
void Draftify::on_ConfirmAddEmpl_clicked()
{
    // 1) Récupérer les champs UI
    int id       = ui->lineEditIDEmpl->text().trimmed().toInt();
    QString nom  = ui->lineEditNameEmpl->text().trimmed();
    QString prenom  = ui->lineEditFamNameEmpl->text().trimmed();
    QString mail = ui->lineEditEmailEmpl->text().trimmed();
    int cin       = ui->lineEditCINEmpl->text().trimmed().toInt();
    int tel       = ui->lineEditPhoneEmpl->text().trimmed().toInt();
    QString adresse    = ui->lineEditAdressEmpl->text().trimmed();
    int salary;  // We will set this based on the post
    QString role = "";  // We will set this based on the post

    // Get the selected post from comboBox
    QString poste = ui->comboBoxPostEmpl->currentText(); // Get selected post

    // Map the post to the corresponding salary and role
    if (poste == "Video Editor") {
        salary = 3500;
        role = "Employee";
    } else if (poste == "Graphic Designer") {
        salary = 3000;
        role = "Employee";
    } else if (poste == "Social Media Manager") {
        salary = 2500;
        role = "Social Media";
    } else if (poste == "Photographer") {
        salary = 2800;
        role = "Employee";
    } else if (poste == "Project Manager") {
        salary = 4500;
        role = "Manager";
    } else if (poste == "Client Manager") {
    salary = 4500;
    role = "Client Service";
    } else if (poste == "Developer") {
        salary = 4000;
        role = "Employee";
    } else if (poste == "Admin") {
        salary = 5000;
        role = "Admin";
    } else if (poste == "HR Manager") {
        salary = 4500;
        role = "HR";
    }

    // Update the salary and role fields (optional, to automatically show the salary and role in the UI)
    ui->lineEditSalaryEmpl->setText(QString::number(salary));
    ui->lineEditRoleEmpl->setText(role);

    // 2) Contrôles de saisie

    // Vérifier ID, Nom et Prénom
    if (id == 0 || nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, tr("Missing data"),
                             tr("Please fill at least ID, Name, and First Name."));
        return;
    }

    // Vérifier que l'ID est un nombre
    bool isValidId = true;
    if (!id) {
        isValidId = false;  // id is not a valid number
        QMessageBox::warning(this, tr("Invalid ID"), tr("ID must be a valid number."));
        return;  // If not a valid ID, stop the function or return
    }

    // Vérifier que le nom et prénom ne contiennent pas de chiffres ou de caractères spéciaux
    bool isValidName = true;
    for (int i = 0; i < nom.length(); ++i) {
        if (!nom[i].isLetter()) {
            isValidName = false;
            break;
        }
    }
    for (int i = 0; i < prenom.length(); ++i) {
        if (!prenom[i].isLetter()) {
            isValidName = false;
            break;
        }
    }
    if (!isValidName) {
        QMessageBox::warning(this, tr("Invalid Name or Surname"),
                             tr("Name and Family Name must contain only letters."));
        return;
    }

    // Vérifier l'email (contient @ et .)
    if (mail.isEmpty() || !mail.contains('@') || !mail.contains('.')) {
        QMessageBox::warning(this, tr("Invalid email"),
                             tr("Please enter a valid email address with '@' and '.'"));
        return;
    }

    // Vérifier le CIN (doit être un nombre à 8 chiffres)
    if (cin < 10000000 || cin > 99999999) {
        QMessageBox::warning(this, tr("Invalid CIN"),
                             tr("CIN must contain exactly 8 digits."));
        return;
    }

    // Vérifier le numéro de téléphone (doit être uniquement numérique)
    QString phoneStr = QString::number(tel);  // Assurer que le téléphone est sous forme de chaîne
    bool isValidPhone = true;
    for (int i = 0; i < phoneStr.length(); ++i) {
        if (!phoneStr[i].isDigit()) {
            isValidPhone = false;
            break;
        }
    }
    if (!isValidPhone) {
        QMessageBox::warning(this, tr("Invalid Phone Number"),
                             tr("Phone number must contain only digits."));
        return;
    }

    // 3) Instancier le modèle et appeler la logique d’ajout (AUCUNE SQL ici)
    Employe e{id, nom, prenom, mail, poste, adresse, cin, tel, salary, role};
    bool test = e.ajouter();

    // 4) Feedback utilisateur + rafraîchissement du tableau
    if (test) {
        // Message d'information (style de l’image)
        QMessageBox::information(
            this,
            QObject::tr("Success"),
            QObject::tr("Employee added.\nClick Cancel to exit."),
            QMessageBox::Ok | QMessageBox::Cancel
            );

        // Rafraîchir la vue avec les données à jour
        ui->tableView_EMPL->setModel(e.afficher());
        // Effacer les champs de saisie après l'ajout
        ui->lineEditIDEmpl->clear();
        ui->lineEditNameEmpl->clear();
        ui->lineEditFamNameEmpl->clear();
        ui->lineEditEmailEmpl->clear();
        ui->lineEditCINEmpl->clear();
        ui->lineEditPhoneEmpl->clear();
        ui->lineEditRoleEmpl->clear();
        ui->lineEditSalaryEmpl->clear();
        ui->comboBoxPostEmpl->setCurrentIndex(-1);  // Set the comboBox to no selection (empty)
    } else {
        // Message d'erreur critique (style de l’image)
        QMessageBox::critical(
            this,
            QObject::tr("Error"),
            QObject::tr("Insert failed.\n Check database/logs."),
            QMessageBox::Cancel
            );
    }
}



//fonction recherche
void Draftify::on_SearchEmpl_clicked()
{
    Employe e;
    // Récupérer le texte du champ de recherche
    QString critere = ui->lineEditSearchEmpl->text().trimmed();

    // Appeler la méthode rechercher() de la classe Employe
    QSqlQueryModel* model = e.rechercher(critere);

    // Mettre à jour le QTableView avec les résultats
    ui->tableView_EMPL->setModel(model);
}


//fonction supprimer
void Draftify::on_DeleteEmpl_clicked()
{
    // Vérifier si une ligne est sélectionnée
    QModelIndexList selectedIndexes = ui->tableView_EMPL->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, tr("No selection"), tr("Please select an employee to delete."));
        return;
    }

    // Récupérer l'ID de l'employé à partir de la première cellule de la ligne sélectionnée
    int id = selectedIndexes.first().data().toInt();

    // Créer l'objet Employe avec l'ID récupéré
    Employe e;
    bool test = e.supprimer(id);  // Supprimer l'employé

    // Afficher un message en fonction du résultat
    if (test) {
        QMessageBox::information(this, tr("Success"), tr("Employee deleted."));
        // Mettre à jour le QTableView après la suppression
        ui->tableView_EMPL->setModel(e.afficher());
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to delete employee."));
    }
}

// Handle double-click event for a cell


void Draftify::on_tableView_EMPL_doubleClicked(const QModelIndex &index)
{
    int row = index.row(); // Get selected row
    int column = index.column(); // Get selected column
}


//copying the info from the table to the edit box
// Copying the info from the table to the edit box
void Draftify::fillFormFromRow(int row)
{
    QAbstractItemModel *m = ui->tableView_EMPL->model();
    if (!m || row < 0) return;

    // Retrieve the data for the employee
    QString id = m->index(row, 0).data().toString();
    QString nom = m->index(row, 1).data().toString();
    QString prenom = m->index(row, 2).data().toString();
    QString mail = m->index(row, 3).data().toString();
    QString poste = m->index(row, 4).data().toString();  // Get the employee's post
    QString cin = m->index(row, 5).data().toString();
    QString phone = m->index(row, 6).data().toString();
    QString adresse = m->index(row, 7).data().toString();

    // Set the values in the line edits
    ui->lineEditIDEmpl->setText(id);
    ui->lineEditNameEmpl->setText(nom);
    ui->lineEditFamNameEmpl->setText(prenom);
    ui->lineEditEmailEmpl->setText(mail);
    ui->lineEditCINEmpl->setText(cin);
    ui->lineEditPhoneEmpl->setText(phone);
    ui->lineEditRoleEmpl->setText(adresse);

    // Set the comboBox for Post (Position)
    int postIndex = ui->comboBoxPostEmpl->findText(poste);  // Find the index of the post in the comboBox
    if (postIndex != -1) {
        ui->comboBoxPostEmpl->setCurrentIndex(postIndex);  // Set the selected post in the comboBox
    }
}

 //clear infos after editing
void Draftify::clearForm()
{
    ui->lineEditIDEmpl->clear();
    ui->lineEditNameEmpl->clear();
    ui->lineEditFamNameEmpl->clear();
    ui->lineEditEmailEmpl->clear();
    ui->comboBoxPostEmpl->setCurrentIndex(-1);
    ui->lineEditCINEmpl->clear();
    ui->lineEditPhoneEmpl->clear();
    ui->lineEditRoleEmpl->clear();
    ui->lineEditSalaryEmpl->clear();

    // ensure ID is editable again for "Add" mode
    ui->lineEditIDEmpl->setReadOnly(false);  // allow ID to be editable again for Add mode
    m_isEditing = false;  // not editing anymore
    m_editingIdOld.clear(); // reset stored old ID
}
void Draftify::on_EditEmpl_clicked()
{
    QItemSelectionModel *sel = ui->tableView_EMPL->selectionModel();
    if (!sel || !sel->hasSelection()) {
        QMessageBox::warning(this, tr("Select a row"), tr("Please select an employee row first."));
        return;
    }

    const int row = sel->currentIndex().row();
    fillFormFromRow(row);

    m_isEditing = true;

    // Remember original ID (used in WHERE even if user could have changed it)
    m_editingIdOld = ui->lineEditIDEmpl->text().trimmed();

    // Lock ID field during edit
    ui->lineEditIDEmpl->setEnabled(true); // greys out and not editable)
}

void Draftify::on_SaveEditEmpl_clicked()
{
    if (!m_isEditing) {
        QMessageBox::information(this, tr("Not editing"),
                                 tr("Click Edit first, then modify fields."));
        return;
    }

    // Read fields
    const QString idNew     = ui->lineEditIDEmpl->text().trimmed();       // locked, but we still read it
    const QString nom       = ui->lineEditNameEmpl->text().trimmed();
    const QString prenom    = ui->lineEditFamNameEmpl->text().trimmed();
    const QString mail      = ui->lineEditEmailEmpl->text().trimmed();
    const QString poste     = ui->comboBoxPostEmpl->currentText().trimmed(); // Get selected post from comboBox
    const QString cinStr    = ui->lineEditCINEmpl->text().trimmed();      // keep as string
    const QString telStr    = ui->lineEditPhoneEmpl->text().trimmed();      // keep as string
    const QString adresse   = ui->lineEditRoleEmpl->text().trimmed();

    // Minimal validation
    if (idNew.isEmpty() || nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, tr("Missing data"),
                             tr("ID, Name and Family name are required."));
        return;
    }
    // Check if the comboBox post value is valid (if necessary)
    if (poste.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Post"), tr("Please select a valid post (position)."));
        return;
    }

    // Build UPDATE (update all fields)
    QSqlQuery q;
    q.prepare(R"(
        UPDATE employe
           SET ID_EMPL = :idNew,
               NOM     = :nom,
               PRENOM  = :prenom,
               MAIL    = :mail,
               POSTE   = :poste,
               CIN     = :cin,
               NUM_TEL = :tel,
               ADRESSE = :adresse
         WHERE ID_EMPL = :idOld
    )"); //R there is araw string literal allows writing a string exactly as-is without escaping special characters like \n, \", used for long versions of code

    q.bindValue(":idNew",   idNew);
    q.bindValue(":nom",     nom);
    q.bindValue(":prenom",  prenom);
    q.bindValue(":mail",    mail);
    q.bindValue(":poste",   poste);
    q.bindValue(":cin",     cinStr);
    q.bindValue(":tel",     telStr);
    q.bindValue(":adresse", adresse);


    q.bindValue(":idOld", m_editingIdOld);

    if (!q.exec()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Update failed:\n%1").arg(q.lastError().text()));
        return;
    }

    QMessageBox::information(this, tr("Saved"), tr("Employee updated successfully."));

    // Refresh table
    QSqlQueryModel *m = Etmp.afficher();  // or Employe::afficher()
    m->setParent(ui->tableView_EMPL);
    ui->tableView_EMPL->setModel(m);

    // leave edit mode, clear, and re-enable ID for future “Add”
    clearForm(); // resets m_isEditing, m_editingIdOld, and makes ID editable for add
    }

//deselectionner la ligne en cliquant dehors du tableau
    bool Draftify::eventFilter(QObject *obj, QEvent *event)
    {
        // Si clic souris n'importe où dans la fenêtre
        if (event->type() == QEvent::MouseButtonPress)
        {
            QWidget *focused = QApplication::widgetAt(QCursor::pos()); //ça sert à détecter où l’utilisateur a cliqué.

            // Si on clique en dehors du tableau → clear selection
            if (focused != ui->tableView_EMPL && !ui->tableView_EMPL->isAncestorOf(focused)) //tester si le clic ne vient ni du tableau, ni d'un élément contenu dans le tableau
            {
                ui->tableView_EMPL->clearSelection();
            }
        }
        // Table hover tooltip with photo
        if (obj == ui->tableapp->viewport()) {
            if (event->type() == QEvent::MouseMove) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                QModelIndex index = ui->tableapp->indexAt(mouseEvent->pos());

                if (index.isValid()) {
                    int row = index.row();
                    QAbstractItemModel* model = ui->tableapp->model();

                    QString clientId = model->index(row, 0).data().toString();
                    QString username = model->index(row, 1).data().toString();
                    QString subCount = model->index(row, 2).data().toString();

                    // Get client details from model (excluding current platform)
                    plateforme p;
                    plateforme::ClientDetails details = p.getClientDetailsExcludingPlatform(clientId, m_currentPlatformId);

                    QString email = details.email.isEmpty() ? "N/A" : details.email;
                    QString platforms = details.otherPlatforms.isEmpty()
                                            ? "No other platforms"
                                            : details.otherPlatforms.join(", ");

                    // ✅ Create rectangular photo (centered)
                    QString photoHtml = "";
                    if (!details.photoPath.isEmpty() && QFile::exists(details.photoPath)) {
                        QPixmap pixmap(details.photoPath);
                        if (!pixmap.isNull()) {
                            // Scale to rectangular size (keeping aspect ratio)
                            QPixmap scaled = pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                            QByteArray byteArray;
                            QBuffer buffer(&byteArray);
                            scaled.save(&buffer, "PNG");
                            QString base64 = QString::fromLatin1(byteArray.toBase64().data());

                            photoHtml = QString(
                                            "<img src='data:image/png;base64,%1' "
                                            "style='border: 3px solid white; border-radius: 8px; "
                                            "display: block; margin: 0 auto 12px auto;' />"
                                            ).arg(base64);
                        }
                    }

                    QString tooltipText = QString(
                                              "<div style='background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                                              "stop:0 rgba(138, 103, 228, 250), stop:1 rgba(102, 126, 234, 250)); "
                                              "padding: 18px; border-radius: 12px; border: 3px solid white; "
                                              "min-width: 220px; text-align: center;'>"
                                              "%1"
                                              "<p style='color: white; font-size: 16px; font-weight: bold; "
                                              "margin: 5px;'>%2</p>"
                                              "<p style='color: white; font-size: 13px; margin: 8px 5px;'>📧 %3</p>"
                                              "<p style='color: white; font-size: 13px; margin: 8px 5px;'>👥 %4 subscribers</p>"
                                              "<p style='color: rgba(255,255,255,0.8); font-size: 11px; "
                                              "margin: 12px 5px 5px 5px;'>Other platforms:</p>"
                                              "<p style='color: white; font-size: 12px; margin: 2px 5px;'>%5</p>"
                                              "</div>"
                                              ).arg(photoHtml, username, email, subCount, platforms);

                    QToolTip::showText(
                        mouseEvent->globalPosition().toPoint(),
                        tooltipText,
                        ui->tableapp,
                        QRect(),
                        10000  // Stay visible for 10 seconds
                        );
                } else {
                    QToolTip::hideText();
                }
            }
            else if (event->type() == QEvent::Leave) {
                QToolTip::hideText();
            }
        }

        return QMainWindow::eventFilter(obj, event);
    }




    void Draftify::on_CancelEditEmpl_clicked()
    {
        clearForm(); // exits edit mode + re-enables ID field for Add
    }



    void Draftify::on_ListOfEmployeesEmpl_clicked()
    {
        ui->lineEditSearchEmpl->clear();
        QSqlQueryModel *m =Etmp.afficher();
        // Safety check: show DB error if any
        if (m->lastError().isValid()) {
            QMessageBox::critical(this, tr("Database error"),
                                  tr("Failed to load employees:\n%1").arg(m->lastError().text()));
            delete m;
            return;
        }
        ui->tableView_EMPL->setModel(m);
        ui->tableView_EMPL->clearSelection();       // no row selected after refresh
        // Clear the comboBox selection after applying the sort
        ui->comboBoxSortEmpl->setCurrentIndex(-1);  // Set the comboBox to no selection (empty)
    }
//some keys quickes
    void Draftify::keyPressEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key_Escape){
            ui->tableView_EMPL->clearSelection();
            on_ListOfEmployeesEmpl_clicked();
        }
        // ENTER → Trigger search
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) { //enter and return r the same
            on_SearchEmpl_clicked();  // Calls the function of search button
        }

    }
//CRUD CLIENT_______________________________________________________________________________________________________
    void Draftify::on_Button_sub_add_cli_clicked()
    {
        // Récupération des champs sous forme de QString
        QString idStr      = ui->line_id_add_cli->text().trimmed();
        QString nom        = ui->line_name_add_cli->text().trimmed();
        QString prenom     = ui->line_last_add_cli->text().trimmed();
        QString email      = ui->line_email_add_cli->text().trimmed();
        QString phone      = ui->line_phone_add_cli->text().trimmed();
        QString social     = ui->line_social_add_cli->text().trimmed();
        QString type       = ui->line_type_add_cli->text().trimmed();
        QString followers  = ui->line_fllwrs_add_cli->text().trimmed();

        QStringList errors;

        // ID : seulement chiffres et minimum 8 chiffres
        QRegularExpression reId("^\\d{8,}$");
        if (!reId.match(idStr).hasMatch()) {
            errors << "ID : doit contenir uniquement des chiffres et au minimum 8 chiffres.";
        }

        // NOM : seulement lettres, longueur max 20
        QRegularExpression reNom("^[A-Za-zÀ-ÖØ-öø-ÿ]{1,20}$");
        if (!reNom.match(nom).hasMatch()) {
            errors << "NOM : uniquement des lettres, maximum 20 caractères.";
        }

        // PRENOM : seulement lettres, longueur max 20
        if (!reNom.match(prenom).hasMatch()) {
            errors << "PRENOM : uniquement des lettres, maximum 20 caractères.";
        }

        // TYPE : seulement lettres (on autorise les espaces)
        QRegularExpression reType("^[A-Za-zÀ-ÖØ-öø-ÿ\\s]+$");
        if (!type.isEmpty() && !reType.match(type).hasMatch()) {
            errors << "TYPE : uniquement des caractères alphabétiques.";
        }

        // PHONE : entier de 8 chiffres
        QRegularExpression rePhone("^\\d{8}$");
        if (!rePhone.match(phone).hasMatch()) {
            errors << "PHONE : doit être un entier contenant exactement 8 chiffres.";
        }

        // EMAIL : contient @ et . (validation simple)
        QRegularExpression reEmail("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");
        if (!reEmail.match(email).hasMatch()) {
            errors << "EMAIL : format invalide (doit contenir '@' et un '.').";
        }

        // SOCIAL MEDIA : on accepte lettres / chiffres / _ . - et espaces
        QRegularExpression reSocial("^[A-Za-z0-9_\\.\\-\\s]*$");
        if (!social.isEmpty() && !reSocial.match(social).hasMatch()) {
            errors << "SOCIAL MEDIA : ne doit contenir que des lettres, chiffres ou '_', '-', '.'.";
        }

        // NUMBER OF FOLLOWERS : entier
        QRegularExpression reFollowers("^\\d+$");
        if (!followers.isEmpty() && !reFollowers.match(followers).hasMatch()) {
            errors << "NUMBER OF FOLLOWERS : doit être un entier.";
        }

        // S'il y a des erreurs, on affiche un message et on arrête
        if (!errors.isEmpty()) {
            QMessageBox::warning(
                this,
                "Erreur de saisie",
                "Impossible d'ajouter le client car :\n- " + errors.join("\n- ")
                );
            return;
        }

        // Tout est OK -> on peut convertir ID en int et ajouter
        int id = idStr.toInt();

        Client c(id, nom, prenom, email, phone, social, type, followers, selectedPhotoPath);
        if (c.ajouter()) {
            QMessageBox::information(this, "Succès", "Client ajouté avec succès !");

            // recharger le tableau
            QSqlQueryModel *model = c.afficher();
            ui->tableWidgetcl->setRowCount(model->rowCount());
            ui->tableWidgetcl->setColumnCount(model->columnCount());

            for (int row = 0; row < model->rowCount(); ++row) {
                for (int col = 0; col < model->columnCount(); ++col) {
                    QString data = model->data(model->index(row, col)).toString();
                    ui->tableWidgetcl->setItem(row, col, new QTableWidgetItem(data));
                }
            }

            clearClientForm();
            selectedPhotoPath.clear(); // Clear the selected photo path
        } else {
            QMessageBox::critical(
                this,
                "Erreur",
                "Échec d’ajout du client (erreur base de données)."
                );
        }
    }
    void Draftify::on_Button_cancel_add_cli_clicked()
    {
        clearClientForm();
    }
    void Draftify::fillClientFormFromRow(int row)
    {
        QAbstractItemModel *m = ui->tableWidgetcl->model();
        if (!m || row < 0) return;

        ui->line_id_add_cli->setText(m->index(row, 0).data().toString());
        ui->line_name_add_cli->setText(m->index(row, 1).data().toString());
        ui->line_last_add_cli->setText(m->index(row, 2).data().toString());
        ui->line_email_add_cli->setText(m->index(row, 3).data().toString());
        ui->line_phone_add_cli->setText(m->index(row, 4).data().toString());
        ui->line_social_add_cli->setText(m->index(row, 5).data().toString());
        ui->line_type_add_cli->setText(m->index(row, 6).data().toString());
        ui->line_fllwrs_add_cli->setText(m->index(row, 7).data().toString());
    }
    void Draftify::clearClientForm()
    {
        ui->line_id_add_cli->clear();
        ui->line_name_add_cli->clear();
        ui->line_last_add_cli->clear();
        ui->line_email_add_cli->clear();
        ui->line_phone_add_cli->clear();
        ui->line_social_add_cli->clear();
        ui->line_type_add_cli->clear();
        ui->line_fllwrs_add_cli->clear();

        m_isEditingClient = false;
        m_editingIdClientOld.clear();
    }
    void Draftify::on_supp_tab_clicked()
    {
        // 1) Vérifier qu’un client est bien sélectionné
        if (m_selectedClientId == -1) {
            QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un client à supprimer !");
            return;
        }

        // 2) Demander confirmation
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmation",
                                      "Voulez-vous vraiment supprimer ce client ?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            QSqlQuery query;
            query.prepare("DELETE FROM CLIENT WHERE ID_CL = :id");
            query.bindValue(":id", m_selectedClientId);

            if (!query.exec()) {
                QMessageBox::critical(this, "Erreur SQL",
                                      "Échec de la suppression :\n" + query.lastError().text());
                return;
            }

            QMessageBox::information(this, "Succès", "Client supprimé avec succès !");
            m_selectedClientId = -1;

            // 3) Rafraîchir le tableau
            Client c;
            QSqlQueryModel *model = c.afficher();

            ui->tableWidgetcl->setRowCount(model->rowCount());
            ui->tableWidgetcl->setColumnCount(model->columnCount());

            for (int r = 0; r < model->rowCount(); ++r) {
                for (int col = 0; col < model->columnCount(); ++col) {
                    QString data = model->data(model->index(r, col)).toString();
                    ui->tableWidgetcl->setItem(r, col, new QTableWidgetItem(data));
                }
            }

            clearClientForm();
        }
    }

    void Draftify::on_tableWidgetcl_clicked(const QModelIndex &index)
    {
        Q_UNUSED(index);
        // juste pour permettre double clic ou future édition
    }
    void Draftify::on_tableWidgetcl_cellClicked(int row, int column)
    {
        Q_UNUSED(column); // évite l’avertissement

        QTableWidgetItem *idItem = ui->tableWidgetcl->item(row, 0); // colonne 0 = ID_CL
        if (!idItem)
            return;

        m_selectedClientId = idItem->text().toInt();
         m_editingClientId  = m_selectedClientId;
        // Tu peux aussi remplir les champs si tu veux :
        ui->line_id_add_cli->setText(idItem->text());
        ui->line_name_add_cli->setText(ui->tableWidgetcl->item(row, 1)->text());
        ui->line_last_add_cli->setText(ui->tableWidgetcl->item(row, 2)->text());
        ui->line_phone_add_cli->setText(ui->tableWidgetcl->item(row, 3)->text());
        ui->line_email_add_cli->setText(ui->tableWidgetcl->item(row, 4)->text());
        ui->line_type_add_cli->setText(ui->tableWidgetcl->item(row, 5)->text());
        ui->line_fllwrs_add_cli->setText(ui->tableWidgetcl->item(row, 6)->text());
        ui->line_social_add_cli->setText(ui->tableWidgetcl->item(row, 7)->text());
    }

    void Draftify::on_edit_tab_clicked()
    {
        // 1) Vérification basique
        if (m_editingClientId == -1) {
            QMessageBox::warning(this, "Erreur", "Aucun client sélectionné pour la modification !");
            return;
        }

        // 2) Récupérer les valeurs depuis le formulaire (adapte les noms si nécessaire)
        int id = m_editingClientId;
        QString nom       = ui->line_name_add_cli->text().trimmed();
        QString prenom    = ui->line_last_add_cli->text().trimmed();
        QString phone     = ui->line_phone_add_cli->text().trimmed();
        QString email     = ui->line_email_add_cli->text().trimmed();
        QString type      = ui->line_type_add_cli->text().trimmed();
        QString followers = ui->line_fllwrs_add_cli->text().trimmed();
        QString social    = ui->line_social_add_cli->text().trimmed();

        // 3) Validation simple
        if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty()) {
            QMessageBox::warning(this, "Champs manquants", "Veuillez remplir Nom, Prénom et Email.");
            return;
        }

        // 4) Préparer et exécuter la requête UPDATE (vérifie bien le nom de la table "CLIENT" et des colonnes)
        QSqlQuery query;
        query.prepare(R"(
        UPDATE CLIENT
           SET NOM = :nom,
               PRENOM = :prenom,
               PHONE = :phone,
               EMAIL = :email,
               TYPE = :type,
               FOLLOWERS = :followers,
               SOCIAL = :social
         WHERE ID_CL = :id_cl
    )");

        query.bindValue(":nom", nom);
        query.bindValue(":prenom", prenom);
        query.bindValue(":phone", phone);
        query.bindValue(":email", email);
        query.bindValue(":type", type);
        query.bindValue(":followers", followers.toInt()); // convertit en entier proprement
        query.bindValue(":social", social);
        query.bindValue(":id_cl", id);

        if (!query.exec()) {
            // affiche l'erreur SQL exactement pour que tu puisses la copier-coller ici
            qDebug() << "Erreur SQL UPDATE CLIENT :" << query.lastError().text();
            QMessageBox::critical(this, "Erreur mise à jour",
                                  "Échec de la mise à jour :\n" + query.lastError().text());
            return;
        }

        // 5) Succès : recharger le tableau depuis la base
        QMessageBox::information(this, "Succès", "Client modifié avec succès !");

        // Rafraîchir le QTableWidget à partir du modèle
        Client c;
        QSqlQueryModel *model = c.afficher();
        ui->tableWidgetcl->setRowCount(model->rowCount());
        ui->tableWidgetcl->setColumnCount(model->columnCount());

        for (int r = 0; r < model->rowCount(); ++r) {
            for (int col = 0; col < model->columnCount(); ++col) {
                QString data = model->data(model->index(r, col)).toString();
                ui->tableWidgetcl->setItem(r, col, new QTableWidgetItem(data));
            }
        }

        // 6) Reset du formulaire + état d'édition
        clearClientForm();
        ui->line_id_add_cli->setEnabled(true);
        m_editingClientId = -1;
    }
    void Draftify::chargerClientsDansTable()
    {
        Client c;
        QSqlQueryModel *model = c.afficher();

        ui->tableWidgetcl->setRowCount(model->rowCount());
        ui->tableWidgetcl->setColumnCount(model->columnCount());

        for (int r = 0; r < model->rowCount(); ++r) {
            for (int col = 0; col < model->columnCount(); ++col) {
                QString data = model->data(model->index(r, col)).toString();
                ui->tableWidgetcl->setItem(r, col, new QTableWidgetItem(data));
            }
        }
    }
    //crud nash
    // In Draftify.cpp, just remove the redundant definitions and use the function calls

    void Draftify::populatePlatformComboBox()
    {
        plateforme p; // Instance of the model
        p.populatePlatformComboBox(ui->cplatuseradd);  // Call the function from plateforme
    }

    void Draftify::populateClientComboBox()
    {
        plateforme p; // Instance of the model
        p.populateClientComboBox(ui->cuseradd);  // Call the function from plateforme
    }

    void Draftify::on_importaddplat_clicked()
    {
        QString filePath = QFileDialog::getOpenFileName(
            this, tr("Choose icon"), "", "Images (*.png *.jpg *.jpeg)");

        if (!filePath.isEmpty())
            ui->imageaddplat->setText(filePath);
    }


    void Draftify::on_importeditplat_clicked()
    {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("Choose icon"),
            "",
            tr("Images (*.png *.jpg *.jpeg)")
            );

        if (!filePath.isEmpty())
            ui->imageeditplat->setText(filePath);
    }


    void Draftify::on_addplat_clicked()
    {
        // 0) Vérifier qu'il reste une place (max 15 plateformes)
        {
            plateforme p;
            int currentCount = p.getPlatformCount();
            const int maxSlots = 15;
            if (currentCount >= maxSlots) {
                QMessageBox::warning(this, "Slots full", "All slots filled, delete a platform first.");
                return;

            }
        }

        plateforme p;

        // ✔ Generate unique 9-digit ID here
        int id = p.generateUniqueID();
        p.setidplat(id);

        // Read UI values
        QString name = ui->platnameadd->text();
        p.setnomplat(name);

        QString type = ui->ctypeplatadd->currentText();
        p.settypeplat(type);

        QString icon = ui->imageaddplat->text();
        p.setphoto(icon);

        // Try to insert (toute la logique SQL + validation est dans plateforme::ajouterplat)
        if (p.ajouterplat())
        {
            ui->idaddplat->setText(QString::number(id));

            QMessageBox::information(this, "Success",
                                     "Added successfully, click on exit to leave");

            // Reset fields except ID (keep ID displayed)
            ui->platnameadd->clear();
            ui->ctypeplatadd->setCurrentIndex(-1);
            ui->imageaddplat->clear();

            // 🔹 Mettre à jour la grille des 15 boutons (app1..app15)
            plateforme p;
            QSqlQueryModel *updatedModel = p.getAllPlatformsSorted("NOM_PLAT");
            refreshPlatformButtons();
            setupPlatformSearch();
            // Refresh the Combo Boxes to show the new platform in all relevant combo boxes
            populatePlatformComboBox();  // Refresh all platform ComboBoxes (Add, Edit, Delete)
        }
        else
        {
            // Error messages (contrôle côté UI pour des messages plus précis)
            if (name.trimmed().isEmpty() ||
                !QRegularExpression("^[A-Za-z]+$").match(name).hasMatch())
            {
                QMessageBox::critical(this, "Error",
                                      "Name unavailable or already used, try again");
            }
            else if (icon.trimmed().isEmpty())
            {
                QMessageBox::critical(this, "Error",
                                      "Icon unavailable! Try again");
            }
            else if (type.trimmed().isEmpty())
            {
                QMessageBox::critical(this, "Error",
                                      "Type unavailable! Try again");
            }
            else
            {
                QMessageBox::critical(this, "Error",
                                      "Add unsuccessful, try again!");
            }
        }
    }



    void Draftify::on_deleteplat_clicked()
    {
        // 1) Lire l'ID saisi
        QString idStr = ui->iddeleteplat->text().trimmed();

        if (idStr.isEmpty()) {
            QMessageBox::warning(this,
                                 "Error",
                                 "id unavailable please retry");
            return;
        }

        bool ok = false;
        int id = idStr.toInt(&ok);
        if (!ok) {
            // pas un entier valide
            QMessageBox::warning(this,
                                 "Error",
                                 "id unavailable please retry");
            return;
        }

        // 2) Appeler le modèle (AUCUNE QSqlQuery ici)
        plateforme p;
        QString platName;
        int res = p.supprimerplat(id, platName);

        if (res == 1) {
            // id inexistant
            QMessageBox::warning(this,
                                 "Error",
                                 "id unavailable please retry");
        }
        else if (res == -1) {
            // erreur SQL
            QMessageBox::critical(this,
                                  "Error",
                                  "deletion unsuccesful");
        }
        else { // res == 0
            // succès
            QMessageBox::information(this,
                                     "Success",
                                     QString("plateform %1 deleted succesfully !")
                                         .arg(platName));

            ui->iddeleteplat->clear();

            // 🔹 Rafraîchir l'affichage des 15 boutons
            plateforme p;
            QSqlQueryModel *updatedModel = p.getAllPlatformsSorted("NOM_PLAT");
            refreshPlatformButtons();
            setupPlatformSearch();
        }
    }


    void Draftify::on_editplat_clicked()
    {
        // 1) ID obligatoire
        QString idStr = ui->ideditplat->text().trimmed();
        if (idStr.isEmpty()) {
            QMessageBox::warning(this,
                                 "Error",
                                 "please provide id to edit");
            return;
        }

        bool ok = false;
        int id = idStr.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(this,
                                 "Error",
                                 "please provide id to edit");
            return;
        }

        // 2) Lire les nouveaux champs (vides = ne pas modifier)
        QString newName = ui->nameeditplat->text().trimmed();

        QString newType;
        if (ui->ctypeeditplat->currentIndex() >= 0)
            newType = ui->ctypeeditplat->currentText();  // l’utilisateur a choisi un type
        else
            newType = "";                                // pas de changement

        QString newPhoto = ui->imageeditplat->text().trimmed();

        // 3) Appeler le modèle (toute la SQL dans plateforme)
        plateforme p;
        bool okEdit = p.modifierplat(id, newName, newType, newPhoto);

        if (okEdit) {
            QMessageBox::information(this,
                                     "Success",
                                     "changes made succesfully");

            // Nettoyer les champs d’édition
            ui->nameeditplat->clear();
            ui->ctypeeditplat->setCurrentIndex(-1);
            ui->imageeditplat->clear();
            // ui->ideditplat->clear(); // à toi de voir si tu veux aussi effacer l’ID

            // Et rafraîchir les boutons d’affichage
            plateforme p;
            QSqlQueryModel *updatedModel = p.getAllPlatformsSorted("NOM_PLAT");
            refreshPlatformButtons();
            setupPlatformSearch();
        }
        else {
            QMessageBox::critical(this,
                                  "Error",
                                  "changes failed , try again");
        }
    }

    void Draftify::refreshPlatformButtons() {
        plateforme p;
        QList<plateforme::PlatformButtonData> platformsData = p.getPlatformButtonsData();

        QVector<QPushButton*> buttons = {
            ui->app1, ui->app2, ui->app3, ui->app4, ui->app5,
            ui->app6, ui->app7, ui->app8, ui->app9, ui->app10,
            ui->app11, ui->app12, ui->app13, ui->app14, ui->app15
        };

        // Clear all buttons
        for (QPushButton *btn : buttons) {
            btn->setText("");
            btn->setIcon(QIcon());
            btn->setEnabled(false);
            btn->setProperty("platId", -1);
        }

        // Fill buttons with platform data
        for (int i = 0; i < platformsData.size() && i < buttons.size(); i++) {
            QPushButton *btn = buttons[i];
            auto data = platformsData[i];

            btn->setProperty("platId", data.id);
            btn->setToolTip(QString("%1 - Users: %2").arg(data.name).arg(data.userCount));
            btn->setEnabled(true);

            if (!data.photo.isEmpty() && QFile::exists(data.photo)) {
                QPixmap pix(data.photo);
                if (!pix.isNull()) {
                    btn->setIcon(QIcon(pix));
                    QSize s = btn->size();
                    btn->setIconSize(QSize(int(s.width() * 0.8), int(s.height() * 0.8)));
                    btn->setText("");
                } else {
                    btn->setText(data.name);
                }
            } else {
                btn->setText(data.name);
            }
        }
    }






    void Draftify::onPlatformButtonClicked()
    {
        // 1) Identify which app button was clicked
        auto *btn = qobject_cast<QPushButton*>(sender());
        if (!btn) {
            qDebug() << "onPlatformButtonClicked: sender is not a QPushButton";
            return;
        }

        // 2) Get the platform ID associated with the clicked button
        bool ok = false;
        int id = btn->property("platId").toInt(&ok);
        if (!ok || id <= 0) {
            qDebug() << "onPlatformButtonClicked: invalid platId property";
            return;
        }

        // ✅ Remember the currently opened platform
        m_currentPlatformId = id;

        // 3) Clear previous platform data to prevent showing incorrect info
        ui->nameapp->clear();
        ui->picapp->clear();
        ui->tableapp->setModel(nullptr); // Clear the table data

        // 4) Load platform data based on the platform ID
        plateforme p;
        QString name, type, photo;
        if (!p.chargerParId(id, name, type, photo)) {
            QMessageBox::warning(this, "Error", "Unable to load platform data.");
            return;
        }

        // ================== Update NAME (QLabel) ==================
        ui->nameapp->setText(name);
        // ===== BEAUTIFUL NAME STYLING =====
        QFont f;
        f.setFamily("Segoe UI");
        f.setPointSize(32);  // Much bigger
        f.setBold(true);
        f.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);  // Letter spacing for elegance
        ui->nameapp->setFont(f);

        ui->nameapp->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        // Beautiful gradient text effect with shadow
        ui->nameapp->setStyleSheet(R"(
        QLabel {
            color: qlineargradient(
                x1:0, y1:0, x2:1, y2:0,
                stop:0 #FFFFFF,
                stop:0.5 #F0E6FF,
                stop:1 #FFFFFF
            );
            background: transparent;
            padding: 15px;
        }
    )");

        // Add elegant shadow for depth
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
        shadowEffect->setBlurRadius(25);
        shadowEffect->setColor(QColor(0, 0, 0, 100));
        shadowEffect->setOffset(0, 3);
        ui->nameapp->setGraphicsEffect(shadowEffect);

        // ✨ NEW: Update PLATFORM TYPE (apptype label) ✨
        ui->apptype->setText(type.toUpper());  // Uppercase for style

        QFont typeFont;
        typeFont.setFamily("Segoe UI");
        typeFont.setPointSize(14);
        typeFont.setWeight(QFont::DemiBold);
        typeFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);  // More spacing for elegance
        ui->apptype->setFont(typeFont);
        ui->apptype->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        // Beautiful badge-style with gradient
        ui->apptype->setStyleSheet(R"(
        QLabel {
            color: rgba(255, 255, 255, 240);
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(102, 126, 234, 180),
                stop:1 rgba(138, 103, 228, 200)
            );
            border: 2px solid rgba(255, 255, 255, 120);
            border-radius: 20px;
            padding: 8px 25px;
        }
    )");

        // Add subtle shadow
        QGraphicsDropShadowEffect *typeShadow = new QGraphicsDropShadowEffect();
        typeShadow->setBlurRadius(15);
        typeShadow->setColor(QColor(0, 0, 0, 80));
        typeShadow->setOffset(0, 2);
        ui->apptype->setGraphicsEffect(typeShadow);

        // ✅ Reflect platform name in the search bar (optional but useful)
        ui->findplat->setText(name);

        // ================== Update PICTURE (QLabel) ==================
        ui->picapp->clear();
        ui->picapp->setScaledContents(false);
        ui->picapp->setAlignment(Qt::AlignCenter);

        if (photo.isEmpty()) {
            ui->picapp->setText("No image (PHOTO empty)");
        } else if (!QFile::exists(photo)) {
            ui->picapp->setText("No image: file not found");
            ui->picapp->setToolTip(photo); // Show the path on hover
        } else {
            QPixmap pix(photo);
            if (pix.isNull()) {
                ui->picapp->setText("Invalid image");
                ui->picapp->setToolTip(photo);
            } else {
                QPixmap scaled = pix.scaled(ui->picapp->size(),
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
                ui->picapp->setPixmap(scaled);
                ui->picapp->setToolTip(photo);
            }
        }

        // ================== Populate Clients TableView with the Platform's Clients ==================
        populateClientsTableView(id);  // Pass the platform ID to filter clients for this platform

        // ================== Navigate to the platform page ==================
        ui->stackedWidget->setCurrentWidget(ui->page_plateforme);
        ui->stackedWidget_plateforme->setCurrentIndex(0);
        updatePodiumWithClientPics(id);
        // After loading platform data, add stats cards

        // ===== POPULATE STATS CARDS (After loading platform data) =====

        // Get statistics
        int totalUsers = p.getTotalUsersForPlatform(id);
        int totalSubs = p.getTotalSubscribersForPlatform(id);
        double avgSubs = totalUsers > 0 ? (double)totalSubs / totalUsers : 0;

        // === Use QGraphicsView with QGraphicsScene (FIXED VERSION) ===

        // Card 1: Total Users
        QGraphicsScene *scene1 = new QGraphicsScene();
        QWidget *card1 = createStatCard("👥", "Total Users", QString::number(totalUsers), QColor(102, 126, 234));
        QGraphicsProxyWidget *proxy1 = scene1->addWidget(card1);
        ui->totalsusers->setScene(scene1);
        ui->totalsusers->setRenderHint(QPainter::Antialiasing);
        ui->totalsusers->setStyleSheet("background: transparent; border: none;");

        // Card 2: Total Subscribers
        QGraphicsScene *scene2 = new QGraphicsScene();
        QWidget *card2 = createStatCard("📊", "Total Subscribers", QString::number(totalSubs), QColor(138, 103, 228));
        QGraphicsProxyWidget *proxy2 = scene2->addWidget(card2);
        ui->topsubs->setScene(scene2);
        ui->topsubs->setRenderHint(QPainter::Antialiasing);
        ui->topsubs->setStyleSheet("background: transparent; border: none;");

        // Card 3: Average
        QGraphicsScene *scene3 = new QGraphicsScene();
        QWidget *card3 = createStatCard("📈", "Average per User", QString::number(avgSubs, 'f', 1), QColor(155, 89, 182));
        QGraphicsProxyWidget *proxy3 = scene3->addWidget(card3);
        ui->averagesubs->setScene(scene3);
        ui->averagesubs->setRenderHint(QPainter::Antialiasing);
        ui->averagesubs->setStyleSheet("background: transparent; border: none;");

        // Add shadow to the container (not the card)
        QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect();
        shadow1->setBlurRadius(20);
        shadow1->setColor(QColor(0, 0, 0, 60));
        shadow1->setOffset(0, 4);
        ui->totalsusers->setGraphicsEffect(shadow1);

        QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect();
        shadow2->setBlurRadius(20);
        shadow2->setColor(QColor(0, 0, 0, 60));
        shadow2->setOffset(0, 4);
        ui->topsubs->setGraphicsEffect(shadow2);

        QGraphicsDropShadowEffect *shadow3 = new QGraphicsDropShadowEffect();
        shadow3->setBlurRadius(20);
        shadow3->setColor(QColor(0, 0, 0, 60));
        shadow3->setOffset(0, 4);
        ui->averagesubs->setGraphicsEffect(shadow3);




    }





    //**********************************adduser***********************************


    // In Draftify.cpp

    // In Draftify.cpp

    void Draftify::on_addapp_clicked()
    {
        // 1) Retrieve UI values
        QString platName = ui->cplatuseradd->currentText().trimmed();
        QString clientIdStr = ui->cuseradd->currentText().trimmed();
        QString username = ui->useraddapp->text().trimmed();
        QString subCountStr = ui->subaddapp->text().trimmed();
        int idp = ui->cplatuseradd->currentIndex();

        // 2) Validate fields
        if (platName.isEmpty() || clientIdStr.isEmpty() || username.isEmpty() || subCountStr.isEmpty()) {
            QMessageBox::warning(this, "Incomplete Information", "One field is missing, try again.");
            return;
        }

        bool isInt;
        int subCount = subCountStr.toInt(&isInt);
        if (!isInt) {
            QMessageBox::warning(this, "Invalid Subscriber Count", "Subscriber count must be an integer. Try again.");
            return;
        }

        int clientId = clientIdStr.toInt();

        // 3) Get Platform ID from platform name (call model method)
        plateforme p;  // Create an instance of plateforme (model)
        int platId = p.getPlatformIdByName(platName);  // Use model method

        if (platId == -1) {
            QMessageBox::warning(this, "Error", "Platform not found.");
            return;
        }

        // 4) Add user to the platform (call model method)
        bool success = p.addUserToPlatform(clientId, platId, username, subCount);  // Call model method

        if (!success) {
            QMessageBox::warning(this, "Client Already Added", "Client is already on this platform.");
            return;
        }

        // 5) Show success message and clear the form
        QMessageBox::information(this, "Success", "Client successfully added to platform.");
        ui->useraddapp->clear();
        ui->subaddapp->clear();

        // Refresh platform-related views
        populateClientsTableView(platId);  // Call model method to populate the table view
        updatePodiumWithClientPics(platId);  // Update podium view for the platform

        // Refresh the ComboBoxes
        populatePlatformComboBox();  // Refresh all platform ComboBoxes
        populatePlatformComboBoxForEdit();  // Refresh for Edit
        populatePlatformComboBoxForDelete();  // Refresh for Delete
    }

    void Draftify::populateClientsTableView(int platId)
    {
        QSqlQueryModel *model = new QSqlQueryModel(this);  // Create a new model to display the data
        plateforme p;  // Create a model instance

        // Call the model's method to populate the clients table
        p.populateClientsTableView(platId, model);  // Use model method to populate the table

        // Bind the model to the tableview
        ui->tableapp->setModel(model);
        ui->tableapp->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableapp->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableapp->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable editing directly in the table

        styleTableApp();
    }




    //*******edituser********************************************

    // In Draftify.cpp

    void Draftify::on_cplatuseredit_currentIndexChanged(int index)
    {
        QString platName = ui->cplatuseredit->currentText().trimmed();

        if (!platName.isEmpty()) {
            plateforme p;  // Create an instance of the model

            // Call the model's method to populate the client combo box
            p.populateClientComboBoxForPlatform(ui->cuseredit, platName);  // Pass the ComboBox and platform name
        }
    }


    // Editing the SubCount for a selected client and platform
    void Draftify::on_editapp_clicked()
    {
        // Get platform and client from UI
        QString platName = ui->cplatuseredit->currentText().trimmed();
        QString clientIdStr = ui->cuseredit->currentText().trimmed();
        QString username = ui->usereditapp->text().trimmed();
        QString subCountStr = ui->subeditapp->text().trimmed();

        // Debug: Log the input data
        qDebug() << "Editing platform:" << platName << ", client ID:" << clientIdStr
                 << ", username:" << username << ", subCount:" << subCountStr;

        // Validate that platform and client are selected
        if (platName.isEmpty() || clientIdStr.isEmpty()) {
            QMessageBox::warning(this, "Incomplete Information", "Please select a platform and client.");
            return;
        }

        // Convert clientId to int
        int clientId = clientIdStr.toInt();

        // Debug: Log the clientId conversion
        qDebug() << "Client ID converted:" << clientId;

        // Fetch the current subscriber count and username from the database
        plateforme p;
        int oldSubCount = p.getCurrentSubscriberCount(platName, clientId);  // Get old subscriber count
        QString oldUsername = p.getCurrentUsername(platName, clientId);  // Fetch current username

        bool isInt = true;
        int subCount = 0;
        if (!subCountStr.isEmpty()) {
            subCount = subCountStr.toInt(&isInt);
            if (!isInt) {
                QMessageBox::warning(this, "Invalid Subscriber Count", "Subscriber count must be an integer.");
                return;
            }
        }

        // Check if at least one field is being updated
        if (username.isEmpty() && subCountStr.isEmpty()) {
            QMessageBox::warning(this, "No Changes", "Please specify what you want to change.");
            return;
        }

        // Debug: Log the update operation
        qDebug() << "Updating client info for clientId:" << clientId << "on platform:" << platName
                 << "with new username:" << username << "and new subCount:" << subCount;

        // Update the client info with new username and subscriber count
        bool success = p.updateClientOnPlatform(clientId, platName, username, subCount);
        if (!success) {
            QMessageBox::critical(this, "Error", "Changes unsuccessful. Try again!");
            return;
        }

        // Display success message without growth rate
        QMessageBox::information(this, "Success", "Changes successfully made.");

        // Refresh the UI with updated client info
        populateClientComboBoxForEditAndDelete();
        int platId = p.getPlatformIdByName(platName);
        if (platId > 0) {
            populateClientsTableView(platId);
            updatePodiumWithClientPics(platId);
        }

        // Clear form after successful update
        clearForm1();
    }



    void Draftify::populateClientComboBoxForEdit()
    {
        QString platName = ui->cplatuseredit->currentText().trimmed();  // Get the platform name

        // Clear the client ID combobox
        ui->cuseredit->clear();

        // Check if a platform is selected
        if (platName.isEmpty()) {
            return;  // Exit if no platform is selected
        }

        // Call model method to get client IDs for the selected platform
        plateforme p;
        QList<int> clientIds = p.getClientIdsByPlatform(platName);

        // Add the client IDs to the combobox
        for (int clientId : clientIds) {
            ui->cuseredit->addItem(QString::number(clientId));  // Convert client ID to string
        }
    }





    void Draftify::populatePlatformComboBoxForEdit()
    {
        plateforme p;  // Create an instance of the model

        // Call the model's method to populate the platform combo box
        p.populatePlatformComboBox(ui->cplatuseredit);  // Pass the ComboBox to the model
    }



    //*************************************deleteuser**************************************
    void Draftify::populatePlatformComboBoxForDelete()
    {
        QList<QString> platforms;
        plateforme p;  // Assuming plateforme is your model class
        p.getAllPlatforms(platforms);  // Retrieve platforms from the model

        ui->cplatuserdelete->clear();  // Clear ComboBox before populating

        for (const QString& platform : platforms) {
            ui->cplatuserdelete->addItem(platform);  // Add each platform to the ComboBox
        }
    }
    void Draftify::on_cplatuserdelete_currentIndexChanged(int index)
    {
        QString platName = ui->cplatuserdelete->currentText().trimmed();
        ui->cuserdelete->clear();  // Clear previous data

        if (!platName.isEmpty()) {
            QList<QString> clientIds;  // Use QList<QString> for client IDs
            plateforme p;
            p.getClientsForPlatform(platName, clientIds);  // Retrieve client IDs from the model

            for (const QString& clientId : clientIds) {
                ui->cuserdelete->addItem(clientId);  // Add client IDs as strings to the ComboBox
            }
        }
    }

    void Draftify::populateClientComboBoxForDelete()
    {
        QString platName = ui->cplatuserdelete->currentText().trimmed();  // Get the platform name
        ui->cuserdelete->clear();  // Clear previous data

        if (!platName.isEmpty()) {
            QList<QString> clientIds;  // Use QList<QString> for client IDs
            plateforme p;
            p.getClientsForPlatform(platName, clientIds);  // Retrieve client IDs from the model

            for (const QString& clientId : clientIds) {
                ui->cuserdelete->addItem(clientId);  // Add client IDs as strings to the ComboBox
            }
        }
    }


    void Draftify::on_deleteapp_clicked()
    {
        QString platName = ui->cplatuserdelete->currentText().trimmed();
        QString clientIdStr = ui->cuserdelete->currentText().trimmed();

        if (platName.isEmpty() || clientIdStr.isEmpty()) {
            QMessageBox::warning(this, "Incomplete Information", "Please select a platform and client.");
            return;
        }

        plateforme p;
        bool success = p.deleteClientFromPlatform(platName, clientIdStr);  // Call the model method

        if (!success) {
            QMessageBox::critical(this, "Error", "Failed to delete the client from the platform.");
            return;
        }

        // Refresh ComboBox and TableView (populate clients and update the podium)
        populateClientComboBoxForDelete();
        int platId = p.getPlatformIdByName(platName);
        if (platId > 0) {
            populateClientsTableView(platId);
            updatePodiumWithClientPics(platId);
        }

        QMessageBox::information(this, "Success", "Client successfully deleted from the selected platform.");
        clearForm1();
    }




    void Draftify::populateClientComboBoxForEditAndDelete()
    {
        QString platName = ui->cplatuseredit->currentText().trimmed();
        ui->cuseredit->clear();
        ui->cuserdelete->clear();

        if (platName.isEmpty()) {
            return;
        }

        QList<QString> clientIds;
        plateforme p;
        p.getClientsForPlatform(platName, clientIds);// Call the model method

        for (const QString& clientId : clientIds) {
            ui->cuseredit->addItem(clientId);   // Add client ID to edit combo box
            ui->cuserdelete->addItem(clientId); // Add client ID to delete combo box
        }
    }


    void Draftify::clearForm1()
    {
        // Clear all text fields
        ui->usereditapp->clear();
        ui->subeditapp->clear();

        // Reset combo boxes
        ui->cplatuseredit->setCurrentIndex(-1);  // Reset to no selection
        ui->cplatuserdelete->setCurrentIndex(-1);
        ui->cuseredit->clear();
        ui->cuserdelete->clear();

        // Optionally, you can reset flags or variables for editing modes
        m_isEditing = false;
    }




    void Draftify::on_butfindplat_clicked()
    {
        QString text = ui->findplat->text().trimmed();

        // Empty search
        if (text.isEmpty()) {
            QMessageBox::warning(this, "Missing data",
                                 "Please provide a platform name or ID.");
            return;
        }

        int platId;
        QString platName;

        plateforme p; // Assuming plateforme is your model class
        if (!p.findPlatform(text, platId, platName)) {
            QMessageBox::warning(this, "Not found",
                                 "This platform doesn't exist.");
            return;
        }

        // Put the clean name back in the line edit
        ui->findplat->setText(platName);

        // Reuse your existing display logic: open platform by ID
        onPlatformButtonClickedFromId(platId); // helper you already have
    }


    void Draftify::setupPlatformSearch()
    {
        plateforme p;  // Create an instance of the model

        // Get platform suggestions from the model
        QStringList items = p.getPlatformSuggestions();

        if (items.isEmpty()) {
            qDebug() << "No platform suggestions found.";
            return;
        }

        // Create the QStringListModel for displaying in the completer
        auto *model = new QStringListModel(items, this);

        // Create the QCompleter and set its properties
        auto *completer = new QCompleter(model, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);  // So typing either ID or name works
        completer->setCompletionMode(QCompleter::PopupCompletion);

        // Set the completer to the line edit (search field)
        ui->findplat->setCompleter(completer);
    }


    void Draftify::openPlatformById(int platId)
    {
        if (platId <= 0) {
            return;
        }

        // Load platform data from the model
        plateforme p;
        QString name, type, photo;
        if (!p.chargerParId(platId, name, type, photo)) {
            QMessageBox::warning(this, "Error", "Unable to load platform data.");
            return;
        }

        // ===== NAME LABEL =====
        ui->nameapp->setText(name);
        QFont f = ui->nameapp->font();
        f.setPointSize(20);
        f.setBold(true);
        ui->nameapp->setFont(f);
        ui->nameapp->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->nameapp->setStyleSheet("QLabel { color: white; background: transparent; }");

        // ===== PICTURE LABEL =====
        ui->picapp->clear();
        ui->picapp->setScaledContents(false);
        ui->picapp->setAlignment(Qt::AlignCenter);

        if (photo.isEmpty()) {
            ui->picapp->setText("No image (PHOTO empty)");
        } else if (!QFile::exists(photo)) {
            ui->picapp->setText("No image: file not found");
            ui->picapp->setToolTip(photo);
        } else {
            QPixmap pix(photo);
            if (pix.isNull()) {
                ui->picapp->setText("Invalid image");
                ui->picapp->setToolTip(photo);
            } else {
                QPixmap scaled = pix.scaled(ui->picapp->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                ui->picapp->setPixmap(scaled);
                ui->picapp->setToolTip(photo);
            }
        }

        // ===== CLIENTS TABLE FOR THIS PLATFORM =====
        populateClientsTableView(platId);

        // ===== SHOW PAGE =====
        ui->stackedWidget->setCurrentWidget(ui->page_plateforme);
        ui->stackedWidget_plateforme->setCurrentIndex(0);
        updatePodiumWithClientPics(platId);
    }

    void Draftify::onPlatformButtonClickedFromId(int platId)
    {
        // 1) Remember the currently opened platform
        m_currentPlatformId = platId;

        // 2) Clear previous platform data to prevent showing incorrect info
        ui->nameapp->clear();
        ui->picapp->clear();
        ui->tableapp->setModel(nullptr); // Clear the table data

        // 3) Load platform data based on the platform ID
        plateforme p;
        QString name, type, photo;
        if (!p.chargerParId(platId, name, type, photo)) {
            QMessageBox::warning(this, "Error", "Unable to load platform data.");
            return;
        }

        // ===== NAME =====
        ui->nameapp->setText(name);

        // BEAUTIFUL NAME STYLING
        QFont f;
        f.setFamily("Segoe UI");
        f.setPointSize(32);
        f.setBold(true);
        f.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
        ui->nameapp->setFont(f);
        ui->nameapp->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        ui->nameapp->setStyleSheet(R"(
        QLabel {
            color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #FFFFFF, stop:0.5 #F0E6FF, stop:1 #FFFFFF);
            background: transparent;
            padding: 15px;
        }
    )");

        QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect;
        shadowEffect->setBlurRadius(25);
        shadowEffect->setColor(QColor(0, 0, 0, 100));
        shadowEffect->setOffset(0, 3);
        ui->nameapp->setGraphicsEffect(shadowEffect);

        // ===== PLATFORM TYPE =====
        ui->apptype->setText(type.toUpper());

        QFont typeFont;
        typeFont.setFamily("Segoe UI");
        typeFont.setPointSize(14);
        typeFont.setWeight(QFont::DemiBold);
        typeFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
        ui->apptype->setFont(typeFont);
        ui->apptype->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        ui->apptype->setStyleSheet(R"(
        QLabel {
            color: rgba(255, 255, 255, 240);
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(102, 126, 234, 180), stop:1 rgba(138, 103, 228, 200));
            border: 2px solid rgba(255, 255, 255, 120);
            border-radius: 20px;
            padding: 8px 25px;
        }
    )");

        QGraphicsDropShadowEffect *typeShadow = new QGraphicsDropShadowEffect;
        typeShadow->setBlurRadius(15);
        typeShadow->setColor(QColor(0, 0, 0, 80));
        typeShadow->setOffset(0, 2);
        ui->apptype->setGraphicsEffect(typeShadow);

        // ===== PICTURE =====
        ui->picapp->clear();
        ui->picapp->setScaledContents(false);
        ui->picapp->setAlignment(Qt::AlignCenter);

        if (photo.isEmpty()) {
            ui->picapp->setText("No image");
        } else if (!QFile::exists(photo)) {
            ui->picapp->setText("No image (file not found)");
            ui->picapp->setToolTip(photo);
        } else {
            QPixmap pix(photo);
            if (pix.isNull()) {
                ui->picapp->setText("Invalid image");
                ui->picapp->setToolTip(photo);
            } else {
                QPixmap scaled = pix.scaled(ui->picapp->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                ui->picapp->setPixmap(scaled);
                ui->picapp->setToolTip(photo);
            }
        }

        // ✅ ===== POPULATE STATS CARDS ===== (THIS WAS MISSING!)
        int totalUsers = p.getTotalUsersForPlatform(platId);
        int totalSubs = p.getTotalSubscribersForPlatform(platId);
        double avgSubs = (totalUsers > 0) ? (double(totalSubs) / totalUsers) : 0;

        // Card 1: Total Users
        QGraphicsScene *scene1 = new QGraphicsScene;
        QWidget *card1 = createStatCard("👥", "Total Users", QString::number(totalUsers), QColor(102, 126, 234));
        QGraphicsProxyWidget *proxy1 = scene1->addWidget(card1);
        ui->totalsusers->setScene(scene1);
        ui->totalsusers->setRenderHint(QPainter::Antialiasing);
        ui->totalsusers->setStyleSheet("background: transparent; border: none;");

        // Card 2: Total Subscribers
        QGraphicsScene *scene2 = new QGraphicsScene;
        QWidget *card2 = createStatCard("📊", "Total Subscribers", QString::number(totalSubs), QColor(138, 103, 228));
        QGraphicsProxyWidget *proxy2 = scene2->addWidget(card2);
        ui->topsubs->setScene(scene2);
        ui->topsubs->setRenderHint(QPainter::Antialiasing);
        ui->topsubs->setStyleSheet("background: transparent; border: none;");

        // Card 3: Average per User
        QGraphicsScene *scene3 = new QGraphicsScene;
        QWidget *card3 = createStatCard("📈", "Average per User", QString::number(avgSubs, 'f', 1), QColor(155, 89, 182));
        QGraphicsProxyWidget *proxy3 = scene3->addWidget(card3);
        ui->averagesubs->setScene(scene3);
        ui->averagesubs->setRenderHint(QPainter::Antialiasing);
        ui->averagesubs->setStyleSheet("background: transparent; border: none;");

        // Add shadows
        QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect;
        shadow1->setBlurRadius(20);
        shadow1->setColor(QColor(0, 0, 0, 60));
        shadow1->setOffset(0, 4);
        ui->totalsusers->setGraphicsEffect(shadow1);

        QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect;
        shadow2->setBlurRadius(20);
        shadow2->setColor(QColor(0, 0, 0, 60));
        shadow2->setOffset(0, 4);
        ui->topsubs->setGraphicsEffect(shadow2);

        QGraphicsDropShadowEffect *shadow3 = new QGraphicsDropShadowEffect;
        shadow3->setBlurRadius(20);
        shadow3->setColor(QColor(0, 0, 0, 60));
        shadow3->setOffset(0, 4);
        ui->averagesubs->setGraphicsEffect(shadow3);

        // ===== TABLE =====
        populateClientsTableView(platId);

        // ===== NAVIGATION =====
        ui->stackedWidget->setCurrentWidget(ui->page_plateforme);
        ui->stackedWidget_plateforme->setCurrentIndex(0);

        // ===== PODIUM =====
        updatePodiumWithClientPics(platId);
    }


    void Draftify::on_sortPlat_clicked()
    {
        QString sortOption = ui->csort->currentText();

        plateforme p;
        QList<plateforme::PlatformButtonData> platformsData = p.getPlatformsSortedBy(sortOption);

        // Update buttons (same code as refreshPlatformButtons but with platformsData)
        QVector<QPushButton*> buttons = {
            ui->app1, ui->app2, ui->app3, ui->app4, ui->app5,
            ui->app6, ui->app7, ui->app8, ui->app9, ui->app10,
            ui->app11, ui->app12, ui->app13, ui->app14, ui->app15
        };

        // Clear all buttons
        for (QPushButton *btn : buttons) {
            btn->setText("");
            btn->setIcon(QIcon());
            btn->setEnabled(false);
            btn->setProperty("platId", -1);
        }

        // Fill with sorted data
        for (int i = 0; i < platformsData.size() && i < buttons.size(); i++) {
            QPushButton *btn = buttons[i];
            auto data = platformsData[i];

            btn->setProperty("platId", data.id);
            btn->setToolTip(QString("%1 - Users: %2").arg(data.name).arg(data.userCount));
            btn->setEnabled(true);

            // Load icon (same as before)
            if (!data.photo.isEmpty() && QFile::exists(data.photo)) {
                QPixmap pix(data.photo);
                if (!pix.isNull()) {
                    btn->setIcon(QIcon(pix));
                    QSize s = btn->size();
                    btn->setIconSize(QSize(int(s.width() * 0.8), int(s.height() * 0.8)));
                    btn->setText("");
                } else {
                    btn->setText(data.name);
                }
            } else {
                btn->setText(data.name);
            }
        }
    }

    void Draftify::createPlatformTypeStatistics(QGraphicsView *typeplatstat)
    {
        plateforme p;
        QVector<QPair<QString, int>> platformTypesCounts = p.getPlatformTypeStatistics();

        if (platformTypesCounts.isEmpty()) {
            qDebug() << "No platform type data found!";
            return;
        }

        QVector<QString> platformTypes;
        QVector<int> counts;
        for (const auto &pair : platformTypesCounts) {
            platformTypes.append(pair.first);
            counts.append(pair.second);
        }

        // Create bar set with gradient
        QBarSet *set = new QBarSet("Platform Types");
        QLinearGradient gradient(0, 0, 0, 200);
        gradient.setColorAt(0.0, QColor(138, 103, 228));
        gradient.setColorAt(1.0, QColor(102, 204, 255));
        set->setBrush(gradient);

        for (int count : counts) {
            *set << count;
        }

        QBarSeries *barSeries = new QBarSeries();
        barSeries->append(set);

        // ✅ DISABLE TOOLTIPS ON BAR SERIES
        barSeries->setLabelsVisible(false);

        QChart *chart = new QChart();
        chart->addSeries(barSeries);
        chart->setTitle("Most Trending Platform Types");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setAnimationDuration(1500);

        // Title styling
        QFont titleFont;
        titleFont.setPixelSize(18);
        titleFont.setBold(true);
        chart->setTitleFont(titleFont);
        chart->setTitleBrush(QBrush(QColor(80, 60, 150)));

        // X-axis
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(platformTypes);
        axisX->setLabelsColor(QColor(100, 80, 160));
        chart->addAxis(axisX, Qt::AlignBottom);
        barSeries->attachAxis(axisX);

        // Y-axis
        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0, *std::max_element(counts.begin(), counts.end()) + 1);
        axisY->setLabelsColor(QColor(100, 80, 160));
        chart->addAxis(axisY, Qt::AlignLeft);
        barSeries->attachAxis(axisY);

        // Background
        chart->setBackgroundBrush(QBrush(QColor(250, 250, 255)));
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);

        // Create chart view
        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        // ✅ DISABLE MOUSE TRACKING ON CHART VIEW
        chartView->setMouseTracking(false);
        chart->setAcceptHoverEvents(false);

        int width = typeplatstat->width();
        int height = typeplatstat->height();
        chartView->setMinimumSize(width, height);
        chartView->setMaximumSize(width, height);
        chartView->resize(width, height);

        if (typeplatstat->scene()) {
            typeplatstat->scene()->clear();
        } else {
            typeplatstat->setScene(new QGraphicsScene);
        }

        typeplatstat->scene()->clear();
        typeplatstat->scene()->addWidget(chartView);
        typeplatstat->setSceneRect(0, 0, width, height);

        // ✅ DISABLE MOUSE TRACKING ON GRAPHICS VIEW
        typeplatstat->setMouseTracking(false);
    }


    void Draftify::createMostUsedPlatformStatistics(QGraphicsView *usedplatstat)
    {
        plateforme p;
        QVector<QPair<QString, int>> platformUsage = p.getMostUsedPlatforms();

        if (platformUsage.isEmpty()) {
            qDebug() << "No platform usage data found!";
            return;
        }

        QPieSeries *pieSeries = new QPieSeries();

        QVector<QColor> colors = {
            QColor(102, 204, 255),  // Light cyan
            QColor(93, 140, 255),   // Blue
            QColor(138, 103, 228),  // Purple
            QColor(180, 130, 255),  // Light purple
            QColor(70, 180, 255),   // Sky blue
            QColor(150, 100, 200)   // Medium purple
        };

        int total = 0;
        for (const auto &pair : platformUsage) {
            total += pair.second;
        }

        for (int i = 0; i < platformUsage.size(); i++) {
            const auto &pair = platformUsage[i];
            QString label = pair.first;
            int count = pair.second;
            double percentage = (count * 100.0) / total;

            QPieSlice *slice = pieSeries->append(
                QString("%1: %2 (%3%)").arg(label).arg(count).arg(percentage, 0, 'f', 1),
                count
                );

            slice->setColor(colors[i % colors.size()]);
            slice->setBorderColor(Qt::white);
            slice->setBorderWidth(2);
            slice->setLabelVisible(true);

            if (i == 0) {
                slice->setExploded(true);
                slice->setExplodeDistanceFactor(0.1);
            }

            // ✅ DISABLE HOVER ON SLICES
            slice->setExplodeDistanceFactor(0.05);
        }

        // ✅ DISABLE HOVER EVENTS ON PIE SERIES
        pieSeries->setHoleSize(0.0);

        QChart *chart = new QChart();
        chart->addSeries(pieSeries);
        chart->setTitle("Most Used Platforms");

        QFont titleFont;
        titleFont.setPixelSize(18);
        titleFont.setBold(true);
        chart->setTitleFont(titleFont);
        chart->setTitleBrush(QBrush(QColor(80, 60, 150)));

        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setAnimationDuration(1500);

        chart->setBackgroundBrush(QBrush(QColor(250, 250, 255)));
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        // ✅ DISABLE MOUSE TRACKING ON CHART VIEW
        chartView->setMouseTracking(false);
        chart->setAcceptHoverEvents(false);

        int width = usedplatstat->width();
        int height = usedplatstat->height();
        chartView->setMinimumSize(width, height);
        chartView->setMaximumSize(width, height);
        chartView->resize(width, height);

        if (usedplatstat->scene()) {
            usedplatstat->scene()->clear();
        } else {
            usedplatstat->setScene(new QGraphicsScene);
        }

        usedplatstat->scene()->clear();
        usedplatstat->scene()->addWidget(chartView);
        usedplatstat->setSceneRect(0, 0, width, height);

        // ✅ DISABLE MOUSE TRACKING ON GRAPHICS VIEW
        usedplatstat->setMouseTracking(false);
    }









    void Draftify::generateAllPlatformsReport()
    {
        // Ask user where to save the PDF
        QString filePath = QFileDialog::getSaveFileName(
            this,
            tr("Save Platforms Report"),
            "platform_report.pdf",
            tr("PDF Files (*.pdf)")
            );

        if (filePath.isEmpty()) {
            return; // User cancelled
        }

        // Initialize PDF writer
        QPdfWriter writer(filePath);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setResolution(96);

        QPainter painter(&writer);
        if (!painter.isActive()) {
            qDebug() << "Failed to initialize painter for PDF";
            return;
        }

        // ===== LAYOUT CONSTANTS =====
        const int pageWidth = writer.width();
        const int marginLeft = 100;
        const int marginRight = 100;
        const int marginTop = 100;
        const int contentWidth = pageWidth - marginLeft - marginRight;
        const int lineHeight = 28;
        const int pageBottom = writer.height() - 150;

        int y = marginTop;

        // ===== FONTS (Smaller and Professional) =====
        QFont titleFont("Arial", 18, QFont::Bold);
        QFont platformHeaderFont("Arial", 10, QFont::Bold);  // Smaller
        QFont sectionFont("Arial", 10, QFont::DemiBold);
        QFont normalFont("Arial", 9);

        // ===== MAIN TITLE (Centered) =====
        painter.setFont(titleFont);
        painter.setPen(QColor(0, 0, 0));  // Black
        QRect titleRect(marginLeft, y, contentWidth, lineHeight * 2);
        painter.drawText(titleRect, Qt::AlignCenter, "Platforms Report");
        y += lineHeight * 2.5;

        // Horizontal separator line
        painter.setPen(QPen(QColor(100, 100, 100), 2));  // Dark grey
        painter.drawLine(marginLeft, y, pageWidth - marginRight, y);
        y += lineHeight * 1.5;

        painter.setFont(normalFont);

        // ✅ GET ALL PLATFORMS FROM MODEL (MVC fix)
        plateforme p;
        QSqlQueryModel* platformModel = p.getAllPlatforms();

        if (!platformModel || platformModel->rowCount() == 0) {
            painter.setFont(normalFont);
            painter.setPen(QColor(255, 100, 100));
            painter.drawText(marginLeft, y, "No platforms found.");
            painter.end();
            delete platformModel;
            return;
        }

        // ===== LOOP THROUGH PLATFORMS =====
        for (int row = 0; row < platformModel->rowCount(); row++) {
            // Check if we need a new page
            if (y > pageBottom) {
                writer.newPage();
                y = marginTop;
            }

            int platId = platformModel->index(row, 0).data().toInt();
            QString platName = platformModel->index(row, 1).data().toString();
            QString platType = platformModel->index(row, 2).data().toString();

            // ===== PLATFORM HEADER (Centered, Grey Box) =====
            painter.setFont(platformHeaderFont);

            // Draw grey background box
            QRect headerBox(marginLeft, y, contentWidth, lineHeight + 8);
            painter.fillRect(headerBox, QColor(240, 240, 240));  // Light grey
            painter.setPen(QPen(QColor(150, 150, 150), 1));  // Grey border
            painter.drawRect(headerBox);

            // Platform details (centered) - Smaller font to fit on one line
            QString platformDetails = QString("Platform: %1  |  Type: %2  |  ID: %3")
                                          .arg(platName)
                                          .arg(platType)
                                          .arg(platId);

            painter.setPen(QColor(40, 40, 40));  // Dark grey text
            painter.drawText(headerBox, Qt::AlignCenter, platformDetails);
            y += lineHeight + 8 + 12; // Box height + spacing

            // ===== CLIENTS SECTION =====
            painter.setFont(sectionFont);
            painter.setPen(QColor(80, 80, 80));  // Medium grey

            // Centered "Clients" title
            QRect clientsTitleRect(marginLeft, y, contentWidth, lineHeight);
            painter.drawText(clientsTitleRect, Qt::AlignCenter, "———————— Clients ————————");
            y += lineHeight + 8;

            // ✅ FETCH CLIENTS (MVC fix - use model method)
            QSqlQueryModel *clientModel = p.getClientsForPlatform(platId);

            if (!clientModel) {
                painter.setFont(normalFont);
                painter.setPen(QColor(120, 120, 120));  // Grey
                QRect errorRect(marginLeft, y, contentWidth, lineHeight);
                painter.drawText(errorRect, Qt::AlignCenter, "Error fetching client data.");
                y += lineHeight + 8;
            } else if (clientModel->rowCount() == 0) {
                painter.setFont(normalFont);
                painter.setPen(QColor(120, 120, 120));  // Grey
                QRect noClientsRect(marginLeft, y, contentWidth, lineHeight);
                painter.drawText(noClientsRect, Qt::AlignCenter, "No clients registered for this platform.");
                y += lineHeight + 8;
            } else {
                painter.setFont(normalFont);
                painter.setPen(QColor(50, 50, 50));  // Dark grey

                // Draw clients (indented)
                for (int i = 0; i < clientModel->rowCount(); i++) {
                    // Check page limit
                    if (y > pageBottom) {
                        writer.newPage();
                        y = marginTop;
                    }

                    int clientId = clientModel->index(i, 0).data().toInt();
                    QString username = clientModel->index(i, 1).data().toString();
                    int subCount = clientModel->index(i, 2).data().toInt();

                    QString clientLine = QString("  • ID: %1  |  Username: %2  |  Subscribers: %3")
                                             .arg(clientId)
                                             .arg(username)
                                             .arg(subCount);

                    painter.drawText(marginLeft + 30, y, clientLine);
                    y += lineHeight - 2;
                }

                delete clientModel;  // Clean up
            }

            // ===== SPACING BETWEEN PLATFORMS =====
            y += lineHeight;

            // Draw separator line between platforms
            painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));  // Light grey dashed
            painter.drawLine(marginLeft + 50, y, pageWidth - marginRight - 50, y);
            y += lineHeight * 1.5;
        }

        painter.end();
        delete platformModel;  // Clean up

        QMessageBox::information(this, "Success", "PDF report generated successfully!\n\nSaved to: " + filePath);
        qDebug() << "PDF generated successfully!";
    }




    void Draftify::on_exportplat_clicked()
    {
        // Create a model for platform data
        QSqlQueryModel *model = new QSqlQueryModel(this);

        // Prepare the query to fetch all platforms
        model->setQuery("SELECT * FROM PLATEFORME ORDER BY NOM_PLAT ASC");

        // If there is any error while querying, return early
        if (model->lastError().isValid()) {
            qDebug() << "Error fetching platforms:" << model->lastError().text();
            QMessageBox::critical(this, "Error", "Failed to fetch platform data.");
            return;
        }

        // Call the function to generate the report
        generateAllPlatformsReport();
    }



    void Draftify::updatePodiumWithClientPics(int platId)
    {
        // Clear photos
        ui->first->clear();
        ui->second->clear();
        ui->third->clear();

        // Clear usernames
        ui->username1->clear();
        ui->username2->clear();
        ui->username3->clear();

        if (platId <= 0) return;

        plateforme p;
        QSqlQueryModel* model = p.getTopClientsForPlatform(platId);

        if (model) {
            QVector<QLabel*> photoLabels = {ui->first, ui->second, ui->third};
            QVector<QLabel*> nameLabels = {ui->username1, ui->username2, ui->username3};

            for (int index = 0; index < model->rowCount() && index < 3; index++) {
                QString photoPath = model->index(index, 0).data().toString();
                QString username = model->index(index, 1).data().toString();

                // ===== SET CIRCULAR PHOTO =====
                if (QFile::exists(photoPath)) {
                    QPixmap pix(photoPath);
                    if (!pix.isNull()) {
                        QLabel* photoLabel = photoLabels[index];

                        // Get label size
                        int labelSize = qMin(photoLabel->width(), photoLabel->height());

                        // Scale image
                        QPixmap scaled = pix.scaled(labelSize, labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

                        // Create circular image with border
                        QPixmap final(labelSize, labelSize);
                        final.fill(Qt::transparent);

                        QPainter painter(&final);
                        painter.setRenderHint(QPainter::Antialiasing);
                        painter.setRenderHint(QPainter::SmoothPixmapTransform);

                        // ✅ Draw outer border circle (thicker, visible)
                        int borderWidth = 4;
                        QPen borderPen(QColor(255, 255, 255, 200), borderWidth);
                        painter.setPen(borderPen);
                        painter.setBrush(Qt::NoBrush);
                        painter.drawEllipse(borderWidth/2, borderWidth/2, labelSize - borderWidth, labelSize - borderWidth);

                        // Draw circular clipping path for image
                        QPainterPath path;
                        int margin = borderWidth + 2;
                        path.addEllipse(margin, margin, labelSize - margin*2, labelSize - margin*2);
                        painter.setClipPath(path);
                        painter.setPen(Qt::NoPen);

                        // Draw the image centered
                        int offset = (labelSize - scaled.width()) / 2;
                        painter.drawPixmap(offset, offset, scaled);
                        painter.end();

                        // Set the final image
                        photoLabel->setPixmap(final);
                        photoLabel->setScaledContents(false);
                        photoLabel->setAlignment(Qt::AlignCenter);

                        // ✅ Add background circle effect behind the image
                        photoLabel->setStyleSheet(QString(R"(
                        QLabel {
                            background: qradialgradient(
                                cx:0.5, cy:0.5, radius:0.5,
                                stop:0 rgba(255, 255, 255, 25),
                                stop:1 rgba(255, 255, 255, 0)
                            );
                            border-radius: %1px;
                        }
                    )").arg(labelSize/2));

                        // ✅ Add shadow for depth
                        QGraphicsDropShadowEffect* photoShadow = new QGraphicsDropShadowEffect();
                        photoShadow->setBlurRadius(20);
                        photoShadow->setColor(QColor(0, 0, 0, 100));
                        photoShadow->setOffset(0, 4);
                        photoLabel->setGraphicsEffect(photoShadow);
                    }
                }

                // ===== SET USERNAME =====
                nameLabels[index]->setText(username);
                nameLabels[index]->setAlignment(Qt::AlignCenter);

                QFont font;
                font.setFamily("Segoe UI");
                font.setPointSize(14);
                font.setWeight(QFont::DemiBold);
                nameLabels[index]->setFont(font);

                nameLabels[index]->setStyleSheet(R"(
                QLabel {
                    color: #FFFFFF;
                    background: transparent;
                }
            )");

                QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
                shadowEffect->setBlurRadius(12);
                shadowEffect->setColor(QColor(30, 20, 60, 120));
                shadowEffect->setOffset(0, 2);
                nameLabels[index]->setGraphicsEffect(shadowEffect);
            }
        }
    }




    void Draftify::setPodiumImage(QLabel* label, const QString& imagePath)
    {
        if (imagePath.isEmpty()) {
            label->setText("No Image");  // Display text if no image exists
            return;
        }

        if (!QFile::exists(imagePath)) {
            label->setText("Image not found");
            return;
        }

        QPixmap pix(imagePath);
        if (pix.isNull()) {
            label->setText("Invalid Image");
            return;
        }

        label->setPixmap(pix.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    void Draftify::onPlatformSelectionChanged()
    {
        // Get the selected platform name
        QString platName = ui->cplatuseradd->currentText().trimmed();

        // Retrieve the platform ID from the model
        plateforme p;
        int platId = p.getPlatformIdByName(platName);  // Use the model to fetch the platform ID

        // Update the podium with the selected platform's clients
        if (platId != -1) {
            updatePodiumWithClientPics(platId);
        }
    }

    void Draftify::on_cplatuseradd_currentIndexChanged(int index)
    {
        // Get the selected platform name
        QString platName = ui->cplatuseradd->currentText().trimmed();

        // Retrieve the platform ID using the model
        plateforme p;
        int platId = p.getPlatformIdByName(platName);  // Use the model to fetch the platform ID

        // If a valid platform ID is found, update the podium with the top 3 clients
        if (platId != -1) {
            updatePodiumWithClientPics(platId);  // Refresh the podium images
        }
    }

    //***********************METIER2**************************************
    void Draftify::on_tableapp_doubleClicked(const QModelIndex &index)
    {
        int row = index.row();  // Get the selected row
        int clientId = ui->tableapp->model()->index(row, 0).data().toInt();  // Assuming the first column is ID_CL

        // Call function to show the user's detailed info
        showUserDetails(clientId);
    }






    void Draftify::showUserDetails(int clientId)
    {
        // Initialize necessary variables
        QString photo, firstName, lastName, email;
        int subCount, currentPlatformId, oldSubCount;
        double growthRate;
        QStringList otherPlatforms;

        // Fetch user details from the model (plateforme class)
        plateforme p;
        bool success = p.getUserDetails(clientId, photo, firstName, lastName, subCount, oldSubCount, growthRate, email, otherPlatforms);

        if (!success) {
            QMessageBox::warning(this, "Error", "Unable to fetch user details.");
            return;
        }

        // Fetch other platforms using the model
        otherPlatforms = p.getOtherPlatforms(clientId, m_currentPlatformId);  // Get other platforms

        // Show user info dialog with other platforms
        showUserInfoDialog(photo, firstName, lastName, subCount, otherPlatforms, email);
    }


    void Draftify::showUserInfoDialog(const QString &photo, const QString &firstName, const QString &lastName, int subCount, const QStringList &otherPlatforms, const QString &email)
    {
        // Create the user details dialog
        QDialog *userDetailsDialog = new QDialog(this);
        userDetailsDialog->setWindowTitle("📊 User Details");
        userDetailsDialog->setModal(true);
        userDetailsDialog->setFixedWidth(320);

        // Main layout
        QVBoxLayout *layout = new QVBoxLayout(userDetailsDialog);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // ===== CONTENT WIDGET WITH GRADIENT BACKGROUND =====
        QWidget *contentWidget = new QWidget();
        contentWidget->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(138, 103, 228, 250),
                stop:1 rgba(102, 126, 234, 250));
            border-radius: 12px;
        }
    )");

        QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(20, 20, 20, 20);
        contentLayout->setSpacing(12);

        // ===== PHOTO =====
        QLabel *photoLabel = new QLabel();
        if (!photo.isEmpty() && QFile::exists(photo)) {
            QPixmap pixmap(photo);
            if (!pixmap.isNull()) {
                // Scale to rectangular size
                QPixmap scaled = pixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                photoLabel->setPixmap(scaled);
            } else {
                photoLabel->setText("📷");
                photoLabel->setStyleSheet("font-size: 48px;");
            }
        } else {
            photoLabel->setText("📷");
            photoLabel->setStyleSheet("font-size: 48px; color: white;");
        }

        photoLabel->setAlignment(Qt::AlignCenter);
        photoLabel->setStyleSheet(photoLabel->styleSheet() + R"(
        border: 3px solid white;
        border-radius: 8px;
        background: rgba(255, 255, 255, 30);
        padding: 5px;
    )");
        contentLayout->addWidget(photoLabel, 0, Qt::AlignCenter);

        // ===== NAME =====
        QLabel *nameLabel = new QLabel(QString("<b>%1 %2</b>").arg(firstName).arg(lastName));
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setStyleSheet(R"(
        color: white;
        font-size: 18px;
        font-weight: bold;
        margin: 5px;
        background: transparent;
    )");
        contentLayout->addWidget(nameLabel);

        // ===== EMAIL =====
        QLabel *emailLabel = new QLabel(email.isEmpty() ? "N/A" : email);
        emailLabel->setAlignment(Qt::AlignCenter);
        emailLabel->setStyleSheet(R"(
        color: white;
        font-size: 14px;
        margin: 5px;
        background: transparent;
    )");
        contentLayout->addWidget(emailLabel);

        // ===== SUBSCRIBER COUNT =====
        QLabel *subCountLabel = new QLabel(QString("<b>%1</b> subscribers").arg(subCount));
        subCountLabel->setAlignment(Qt::AlignCenter);
        subCountLabel->setStyleSheet(R"(
        color: white;
        font-size: 14px;
        margin: 8px 5px;
        background: transparent;
    )");
        contentLayout->addWidget(subCountLabel);

        // ===== SEPARATOR LINE =====
        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet(R"(
        background: rgba(255, 255, 255, 100);
        border: none;
        height: 1px;
        margin: 8px 0px;
    )");
        contentLayout->addWidget(line);

        // ===== OTHER PLATFORMS TITLE =====
        QLabel *platformsTitleLabel = new QLabel("<b>Other platforms</b>");
        platformsTitleLabel->setAlignment(Qt::AlignCenter);
        platformsTitleLabel->setStyleSheet(R"(
        color: rgba(255, 255, 255, 200);
        font-size: 12px;
        margin: 5px 5px 5px 5px;
        background: transparent;
    )");
        contentLayout->addWidget(platformsTitleLabel);

        // ===== OTHER PLATFORMS LIST =====
        if (otherPlatforms.isEmpty()) {
            QLabel *noPlatformsLabel = new QLabel("No other platforms");
            noPlatformsLabel->setAlignment(Qt::AlignCenter);
            noPlatformsLabel->setStyleSheet(R"(
            color: white;
            font-size: 13px;
            margin: 2px 5px;
            background: transparent;
        )");
            contentLayout->addWidget(noPlatformsLabel);
        } else {
            for (const QString &platform : otherPlatforms) {
                QLabel *platformLabel = new QLabel("• " + platform);
                platformLabel->setAlignment(Qt::AlignCenter);
                platformLabel->setStyleSheet(R"(
                color: white;
                font-size: 13px;
                margin: 2px 5px;
                background: transparent;
            )");
                contentLayout->addWidget(platformLabel);
            }
        }

        // Add content widget to main layout
        layout->addWidget(contentWidget);

        // ===== CLOSE BUTTON =====
        QPushButton *closeButton = new QPushButton("Close");
        closeButton->setStyleSheet(R"(
        QPushButton {
            background: white;
            color: rgba(138, 103, 228, 255);
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
            margin: 10px;
        }
        QPushButton:hover {
            background: rgba(255, 255, 255, 230);
        }
        QPushButton:pressed {
            background: rgba(255, 255, 255, 200);
        }
    )");

        connect(closeButton, &QPushButton::clicked, userDetailsDialog, &QDialog::accept);
        layout->addWidget(closeButton, 0, Qt::AlignCenter);

        // Show the dialog
        userDetailsDialog->exec();
        delete userDetailsDialog;
    }



    void Draftify::styleTableApp()
    {
        ui->tableapp->setStyleSheet(R"(
        /* Table styling - Soft background */
        QTableView {
            background-color: #F8F7FC;
            border: 3px solid rgba(138, 103, 228, 180);
            border-radius: 12px;
            selection-background-color: rgba(138, 103, 228, 255);
            selection-color: white;
            font-family: 'Segoe UI';
            font-size: 14px;
            color: #3D3D5C;
            padding: 5px;
        }

        /* Header styling - Softer purple gradient */
        QHeaderView::section {
            background: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(138, 103, 228, 200),
                stop:1 rgba(102, 126, 234, 200)
            );
            color: white;
            padding: 12px;
            border: none;
            border-right: 2px solid rgba(255, 255, 255, 120);
            font-weight: bold;
            font-size: 14px;
        }

        QHeaderView::section:first {
            border-top-left-radius: 8px;
        }

        QHeaderView::section:last {
            border-top-right-radius: 8px;
            border-right: none;
        }

        /* Row styling - Soft alternating colors */
        QTableView::item {
            padding: 12px 8px;
            border: none;
            border-bottom: 1px solid rgba(200, 200, 220, 120);
        }

        QTableView::item:alternate {
            background-color: rgba(138, 103, 228, 40);
        }

        QTableView::item:selected {
            background-color: rgba(138, 103, 228, 220);
            color: white;
            font-weight: bold;
        }

        QTableView::item:hover {
            background-color: rgba(138, 103, 228, 100);
            color: #3D3D5C;
        }

        /* Scrollbar - Softer colors */
        QScrollBar:vertical {
            background: rgba(230, 230, 240, 150);
            width: 14px;
            border-radius: 7px;
            margin: 2px;
        }

        QScrollBar::handle:vertical {
            background: rgba(138, 103, 228, 150);
            border-radius: 7px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background: rgba(138, 103, 228, 200);
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )");

        // ===== TABLE SETTINGS =====
        ui->tableapp->setAlternatingRowColors(true);
        ui->tableapp->setShowGrid(false);
        ui->tableapp->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableapp->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableapp->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        ui->tableapp->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

        // Header settings
        ui->tableapp->horizontalHeader()->setStretchLastSection(true);
        ui->tableapp->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableapp->horizontalHeader()->setHighlightSections(false);
        ui->tableapp->verticalHeader()->setVisible(false);

        // Row height
        ui->tableapp->verticalHeader()->setDefaultSectionSize(45);

        // Comfortable font
        QFont tableFont("Segoe UI", 13);
        tableFont.setWeight(QFont::Medium);
        ui->tableapp->setFont(tableFont);
    }
    QWidget* Draftify::createStatCard(const QString &icon, const QString &label, const QString &value, const QColor &color)
    {
        QWidget *card = new QWidget();
        card->setFixedSize(280, 150);

        // Clean gradient without shadow
        card->setStyleSheet(QString(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(%1, %2, %3, 240),
                stop:1 rgba(%4, %5, %6, 255));
            border-radius: 20px;
            border: none;
        }
    )").arg(color.red()).arg(color.green()).arg(color.blue())
                                .arg(qMax(color.red() - 15, 0))
                                .arg(qMax(color.green() - 15, 0))
                                .arg(qMax(color.blue() - 15, 0)));

        // DO NOT ADD SHADOW EFFECT - it breaks QGraphicsView rendering

        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setSpacing(6);
        layout->setContentsMargins(25, 25, 25, 25);

        // Icon
        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 44px; background: transparent; border: none;");
        iconLabel->setAlignment(Qt::AlignCenter);

        // Value
        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet("color: white; font-size: 38px; font-weight: bold; background: transparent; border: none;");
        valueLabel->setAlignment(Qt::AlignCenter);

        // Label
        QLabel *textLabel = new QLabel(label);
        textLabel->setStyleSheet("color: rgba(255, 255, 255, 230); font-size: 14px; font-weight: 600; background: transparent; border: none;");
        textLabel->setAlignment(Qt::AlignCenter);

        layout->addStretch();
        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(textLabel);
        layout->addStretch();

        return card;
    }

    //----------------PROJET--------------------
    // ====================================
    // CRUD PROJET
    // ====================================

    void Draftify::chargerClients()
    {
        QSqlQuery q;
        QString queryStr = "SELECT ID_CL, NOM FROM CLIENT ORDER BY NOM";
        qDebug() << "Executing query:" << queryStr;

        if (!q.exec(queryStr))
        {
            qDebug() << "Error loading clients:" << q.lastError().text();
            QMessageBox::warning(this, "Erreur", "Impossible de charger les clients: " + q.lastError().text());
            return;
        }

        ui->comboBox_client->clear();
        ui->comboBox_client->addItem("-- Choisir un client --", QVariant(0));

        int count = 0;
        while (q.next())
        {
            int id = q.value(0).toInt();
            QString nom = q.value(1).toString();
            QString display = QString("%1 - %2").arg(id).arg(nom);
            ui->comboBox_client->addItem(display, id);
            count++;
        }

        qDebug() << "Loaded" << count << "clients";

        if (count == 0)
        {
            QMessageBox::information(this, "Info", "Aucun client trouvé dans la base de données.");
        }
    }

    void Draftify::actualiserTable()
    {
        QSqlQuery query;
        query.prepare("SELECT p.ID_PROJ, p.TYPE_PROJET, TO_CHAR(p.DEADLINE, 'YYYY-MM-DD') as DEADLINE, "
                      "p.STATUT, p.ID_CL FROM PROJET p ORDER BY p.ID_PROJ");

        if (!query.exec())
        {
            qDebug() << "Error fetching projects:" << query.lastError().text();
            QMessageBox::warning(this, "Erreur", "Impossible de charger les projets: " + query.lastError().text());
            return;
        }

        ui->tableWidgetProjet->setRowCount(0);
        while (query.next())
        {
            int row = ui->tableWidgetProjet->rowCount();
            ui->tableWidgetProjet->insertRow(row);

            for (int col = 0; col < 5; ++col)
            {
                QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
                ui->tableWidgetProjet->setItem(row, col, item);
            }
        }
        ui->tableWidgetProjet->resizeColumnsToContents();
    }

    void Draftify::viderChamps()
    {
        ui->lineEditIDProjet->clear();
        ui->dateEdit_projet->setDate(QDate::currentDate());
        ui->comboBox_type->setCurrentIndex(0);
        ui->comboBox_Projet->setCurrentIndex(0);
        ui->comboBox_client->setCurrentIndex(0);
        ui->ConfirmAddProjet->setText("Confirmer");
    }

    void Draftify::chargerProjetDansFormulaire(int id)
    {
        QSqlQuery q;
        q.prepare("SELECT TYPE_PROJET, TO_CHAR(DEADLINE,'YYYY-MM-DD'), STATUT, ID_CL "
                  "FROM PROJET WHERE ID_PROJ = :id");
        q.bindValue(":id", id);
        if (q.exec() && q.next())
        {
            ui->comboBox_type->setCurrentText(q.value(0).toString());
            ui->dateEdit_projet->setDate(QDate::fromString(q.value(1).toString(), "yyyy-MM-dd"));
            ui->comboBox_Projet->setCurrentText(q.value(2).toString());
            int id_cl = q.value(3).toInt();
            int index = ui->comboBox_client->findData(id_cl);
            if (index != -1)
                ui->comboBox_client->setCurrentIndex(index);
            ui->lineEditIDProjet->setText(QString::number(id));
            ui->ConfirmAddProjet->setText("Mettre à jour");
        }
        else
        {
            qDebug() << "Error loading project:" << q.lastError().text();
        }
    }

    void Draftify::on_ConfirmAddProjet_clicked()
    {
        bool ok;
        int id = ui->lineEditIDProjet->text().trimmed().toInt(&ok);
        if (!ok || id <= 0) {
            QMessageBox::warning(this, "Erreur", "L'ID doit être un nombre positif !");
            return;
        }

        QString type = ui->comboBox_type->currentText();
        QString statut = ui->comboBox_Projet->currentText();
        QDate deadline = ui->dateEdit_projet->date();
        int id_cl = ui->comboBox_client->currentData().toInt();

        // Vérifier champs obligatoires
        if (type.isEmpty() || statut.isEmpty() || id_cl <= 0) {
            QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs !");
            return;
        }

        // 1. Détection des statuts "terminés" (multi-langues)
        QString statutLower = statut.toLower();
        bool isFinishedStatus = (statutLower == "terminé" || statutLower == "finished" ||
                                 statutLower == "terminée" || statutLower == "done" ||
                                 statutLower == "completé" || statutLower == "completed");

        // 2. Vérifier que projet terminé n'a pas une deadline future
        if (isFinishedStatus && deadline > QDate::currentDate()) {
            QMessageBox::warning(this, "Erreur",
                                 "🚫 Impossible de créer un projet 'Terminé' avec une deadline future !\n\n"
                                 "Un projet terminé doit avoir une deadline passée.");
            return;
        }

        // 3. Vérifier que la deadline est future pour projets non terminés
        if (!isFinishedStatus && deadline <= QDate::currentDate()) {
            QMessageBox::warning(this, "Erreur",
                                 "📅 La deadline doit être dans le futur pour un projet non terminé !");
            return;
        }

        // 4. Deadline maximum réaliste
        QDate maxDeadline = QDate::currentDate().addYears(3);
        if (deadline > maxDeadline) {
            QMessageBox::warning(this, "Erreur",
                                 "La deadline est trop éloignée ! Maximum 3 ans.");
            return;
        }

        // Vérifier si l'ID existe déjà
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM PROJET WHERE ID_PROJ = :id");
        checkQuery.bindValue(":id", id);
        if (!checkQuery.exec()) {
            QMessageBox::critical(this, "Erreur", "Erreur lors de la vérification de l'ID: " + checkQuery.lastError().text());
            return;
        }
        checkQuery.next();
        if (checkQuery.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Erreur", "Un projet avec cet ID existe déjà !");
            return;
        }

        // Vérifier que le client existe vraiment
        QSqlQuery checkClientQuery;
        checkClientQuery.prepare("SELECT COUNT(*) FROM CLIENT WHERE ID_CL = :id_cl");
        checkClientQuery.bindValue(":id_cl", id_cl);
        if (!checkClientQuery.exec() || !checkClientQuery.next() || checkClientQuery.value(0).toInt() == 0) {
            QMessageBox::warning(this, "Erreur", "Le client sélectionné n'existe pas !");
            return;
        }

        // Créer l'objet projet et l'ajouter à la base
        projet = Projet(id, type, deadline.toString("yyyy-MM-dd"), statut, id_cl);

        qDebug() << "Adding project:" << id << type << deadline << statut << id_cl;

        if (!projet.ajouter()) {
            QMessageBox::critical(this, "Erreur", "Échec de l'ajout ! Erreur dans la base de données.");
            return;
        }

        QMessageBox::information(this, "Succès", "✅ Projet ajouté avec succès !");
        actualiserTable();
        loadProjectsFromDatabase();
        viderChamps();
        chargerClients();
    }

    void Draftify::on_ConfirmUpdateProjet_clicked()
    {
        bool ok;
        int id = ui->lineEditIDProjet->text().trimmed().toInt(&ok);
        if (!ok || id <= 0) {
            QMessageBox::warning(this, "Erreur", "Sélectionnez d'abord un projet à modifier !");
            return;
        }

        QString type = ui->comboBox_type->currentText();
        QString statut = ui->comboBox_Projet->currentText();
        QDate deadline = ui->dateEdit_projet->date();
        int id_cl = ui->comboBox_client->currentData().toInt();

        if (type.isEmpty() || statut.isEmpty() || id_cl <= 0) {
            QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires !");
            return;
        }

        // Normalize status
        QString statutNorm = statut.trimmed().toLower();

        // Remove accents
        statutNorm = statutNorm.normalized(QString::NormalizationForm_D);
        statutNorm.remove(QRegularExpression("[\\u0300-\\u036f]"));

        QStringList listeFin = {
            "termine", "terminee",
            "finished", "finish",
            "done",
            "complete", "completed",
            "fini", "fin", "finit"
        };

        bool estStatutTermine = listeFin.contains(statutNorm);

        // HARD constraint
        if (estStatutTermine && deadline > QDate::currentDate()) {
            QMessageBox::warning(
                this,
                "Erreur",
                "🚫 Impossible de marquer le projet comme 'Terminé' avec une deadline future !\n\n"
                "Un projet terminé doit avoir une deadline passée.\n\n"
                "Solutions :\n"
                "• Modifiez la deadline pour une date passée\n"
                "• Gardez un statut non terminé (ex : En cours)"
                );
            return;
        }

        // Check client exists
        QSqlQuery checkClientQuery;
        checkClientQuery.prepare("SELECT COUNT(*) FROM CLIENT WHERE ID_CL = :id_cl");
        checkClientQuery.bindValue(":id_cl", id_cl);

        if (!checkClientQuery.exec() || !checkClientQuery.next() || checkClientQuery.value(0).toInt() == 0) {
            QMessageBox::warning(this, "Erreur", "Le client sélectionné n'existe pas !");
            return;
        }

        // Update DB (🔥 DATE FORMAT ALIGNÉ AVEC PROJET::ajouter / modifier)
        QSqlQuery updateQuery;
        updateQuery.prepare(R"(
        UPDATE PROJET SET
            TYPE_PROJET = :type,
            DEADLINE    = TO_DATE(:deadline, 'YYYY-MM-DD'),
            STATUT      = :statut,
            ID_CL       = :id_cl
        WHERE ID_PROJ   = :id
    )");

        updateQuery.bindValue(":type", type);
        updateQuery.bindValue(":deadline", deadline.toString("yyyy-MM-dd"));
        updateQuery.bindValue(":statut", statut);
        updateQuery.bindValue(":id_cl", id);
        updateQuery.bindValue(":id", id);

        if (updateQuery.exec()) {
            QMessageBox::information(this, "Succès", "✅ Projet modifié avec succès !");
            actualiserTable();
            loadProjectsFromDatabase();
            viderChamps();

            // Clear the checklist and reset the combo box
            ui->listWidgetChecklist->clear();  // Clear current checklist
            ui->comboBoxproj->clear();         // Clear the combo box
            ui->labelChecklistProject->setText("Chargement...");  // Update label to loading state

            // Reload projects and set the updated project as the current one
            loadProjectsFromDatabase(); // Ensure projects are reloaded
            ui->comboBoxproj->setCurrentIndex(ui->comboBoxproj->findData(id));  // Set the modified project back
            ui->labelChecklistProject->setText("📋 " + ui->comboBoxproj->currentText());  // Update label with new project type

            // Refresh the checklist display for the new project
            loadChecklistForSelectedProject(id);
            checkAndUpdateProjectStatus(id); // 🔥 important
        } else {
            QMessageBox::critical(this, "Erreur", "Échec modification ! " + updateQuery.lastError().text());
        }
    }

    void Draftify::on_DeleteProjet_clicked()
    {
        bool ok;
        int id = ui->lineEditIDProjet->text().trimmed().toInt(&ok);

        if (!ok || id <= 0) {
            QMessageBox::warning(this, "Erreur", "ID invalide !");
            return;
        }

        Projet p;
        if (p.supprimer(id)) {
            QMessageBox::information(this, "Succès", "Projet supprimé !");
            actualiserTable();
            loadProjectsFromDatabase();
            viderChamps();
            chargerClients();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec suppression !");
        }
    }


    void Draftify::on_tableWidgetProjet_cellClicked(int row, int col)
    {
        Q_UNUSED(col);
        if (ui->tableWidgetProjet->item(row, 0)) {
            int id = ui->tableWidgetProjet->item(row, 0)->text().toInt();
            chargerProjetDansFormulaire(id);
        }
    }

    // new for the search
    void Draftify::on_lineEditSearchProjet_textChanged(const QString &text)
    {
        if (text.trimmed().isEmpty()) {
            actualiserTable();  // recharge tout
        }
    }

    // Enhanced search function
    void Draftify::on_SearchProjet_clicked()
    {
        QString searchText = ui->lineEditSearchProjet->text().trimmed();
        if (searchText.isEmpty()) {
            QMessageBox::information(this, "Recherche", "Veuillez entrer un ID ou un type !");
            actualiserTable();
            return;
        }

        QSqlQuery query;

        // Check if search text is numeric (ID search)
        bool isNumeric;
        int id = searchText.toInt(&isNumeric);

        if (isNumeric && id > 0) {
            // Search by ID
            query.prepare("SELECT ID_PROJ, TYPE_PROJET, TO_CHAR(DEADLINE,'YYYY-MM-DD') AS DEADLINE, STATUT, ID_CL "
                          "FROM PROJET WHERE ID_PROJ = :id");
            query.bindValue(":id", id);
        } else {
            // Search by type
            query.prepare("SELECT ID_PROJ, TYPE_PROJET, TO_CHAR(DEADLINE,'YYYY-MM-DD') AS DEADLINE, STATUT, ID_CL "
                          "FROM PROJET WHERE UPPER(TYPE_PROJET) LIKE UPPER(:type)");
            query.bindValue(":type", "%" + searchText + "%");
        }

        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Erreur lors de la recherche: " + query.lastError().text());
            return;
        }

        // Clear and setup table
        ui->tableWidgetProjet->setRowCount(0);
        ui->tableWidgetProjet->setColumnCount(5);
        QStringList headers = {"ID", "Type", "Deadline", "Statut", "Client ID"};
        ui->tableWidgetProjet->setHorizontalHeaderLabels(headers);

        int row = 0;
        while (query.next()) {
            ui->tableWidgetProjet->insertRow(row);
            for (int col = 0; col < 5; ++col) {
                QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
                ui->tableWidgetProjet->setItem(row, col, item);
            }
            row++;
        }

        if (row == 0) {
            QMessageBox::information(this, "Recherche", "Aucun résultat trouvé.");
            actualiserTable();
        } else {
            ui->tableWidgetProjet->resizeColumnsToContents();
        }
    }

    //added recently
    void Draftify::setProjetModelToTable(QSqlQueryModel *model)
    {
        ui->tableWidgetProjet->setRowCount(model->rowCount());
        ui->tableWidgetProjet->setColumnCount(model->columnCount());

        // Insert data into the QTableWidget
        for (int row = 0; row < model->rowCount(); ++row) {
            for (int col = 0; col < model->columnCount(); ++col) {
                QTableWidgetItem *item = new QTableWidgetItem(model->data(model->index(row, col)).toString());
                ui->tableWidgetProjet->setItem(row, col, item);
            }
        }
    }

    void Draftify::on_EditProjet_clicked()
    {
        actualiserTable();
        ui->comboBoxSortProjet->setCurrentIndex(0);  // Remet "Trier par..."
        QMessageBox::information(this, "Tri", "Affichage normal restauré");
    }

    void Draftify::on_ConfirmTriProjet_clicked()
    {
        // Vérifier s'il y a des projets à trier
        if (ui->tableWidgetProjet->rowCount() == 0) {
            QMessageBox::information(this, "Tri", "Aucun projet à trier !");
            return;
        }

        // Récupère l'index sélectionné dans le combobox
        int index = ui->comboBoxSortProjet->currentIndex();

        // Si "Trier par..." est sélectionné, on ne fait rien
        if (index == 0) {
            QMessageBox::information(this, "Tri", "Veuillez sélectionner un type de tri !");
            return;
        }

        QString sql;
        QString message;

        switch(index) {
        case 1: // "Deadline (plus proche)"
            sql = "SELECT ID_PROJ, TYPE_PROJET, TO_CHAR(DEADLINE,'YYYY-MM-DD') AS DEADLINE, STATUT, ID_CL FROM PROJET ORDER BY DEADLINE ASC";
            message = "Projets triés par deadline (plus proche d'abord)";
            break;

        case 2: // "Deadline (plus éloignée)"
            sql = "SELECT ID_PROJ, TYPE_PROJET, TO_CHAR(DEADLINE,'YYYY-MM-DD') AS DEADLINE, STATUT, ID_CL FROM PROJET ORDER BY DEADLINE DESC";
            message = "Projets triés par deadline (plus éloignée d'abord)";
            break;

        default:
            actualiserTable();
            return;
        }

        // Exécute le tri
        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery(sql);

        if (model->lastError().isValid()) {
            QMessageBox::critical(this, "Erreur", "Erreur lors du tri: " + model->lastError().text());
            return;
        }

        // Vérifier si le tri a retourné des résultats
        if (model->rowCount() == 0) {
            QMessageBox::information(this, "Tri", "Aucun projet trouvé !");
            return;
        }

        setProjetModelToTable(model);

        // Redimensionner les colonnes pour une meilleure visibilité
        ui->tableWidgetProjet->resizeColumnsToContents();

        QMessageBox::information(this, "Tri", message);
    }

    void Draftify::on_PDF_ExportProjet_clicked()
    {
        // Check if there's any data to export
        if (ui->tableWidgetProjet->rowCount() == 0) {
            QMessageBox::warning(this, "Export", "Aucune donnée à exporter ! La table des projets est vide.");
            return;
        }

        // Check if table has valid data (not just empty rows)
        bool hasData = false;
        for (int r = 0; r < ui->tableWidgetProjet->rowCount(); ++r) {
            for (int c = 0; c < ui->tableWidgetProjet->columnCount(); ++c) {
                QTableWidgetItem *it = ui->tableWidgetProjet->item(r, c);
                if (it && !it->text().trimmed().isEmpty()) {
                    hasData = true;
                    break;
                }
            }
            if (hasData) break;
        }

        if (!hasData) {
            QMessageBox::warning(this, "Export", "Aucune donnée valide à exporter ! Toutes les cellules sont vides.");
            return;
        }

        QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter les projets en CSV"),
                                                        QDir::homePath(), tr("Fichiers CSV (*.csv)"));
        if (fileName.isEmpty()) {
            QMessageBox::information(this, "Export", "Export annulé par l'utilisateur.");
            return;
        }

        if (!fileName.endsWith(".csv", Qt::CaseInsensitive))
            fileName += ".csv";

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Erreur",
                                  QString("Impossible d'ouvrir le fichier !\nErreur: %1").arg(file.errorString()));
            return;
        }

        QTextStream out(&file);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out.setCodec("UTF-8");
#else
        out.setEncoding(QStringConverter::Utf8);
#endif

        // Add BOM for better Excel compatibility
        out << "\uFEFF";

        int rows = ui->tableWidgetProjet->rowCount();
        int cols = ui->tableWidgetProjet->columnCount();

        QStringList headers;
        for (int c = 0; c < cols; ++c) {
            QTableWidgetItem *h = ui->tableWidgetProjet->horizontalHeaderItem(c);
            headers << (h ? h->text() : QString("Col%1").arg(c));
        }
        out << headers.join(';') << '\n';

        int exportedRows = 0;
        for (int r = 0; r < rows; ++r) {
            QStringList values;
            bool rowHasData = false;

            for (int c = 0; c < cols; ++c) {
                QTableWidgetItem *it = ui->tableWidgetProjet->item(r, c);
                QString val = it ? it->text() : "";

                if (!val.trimmed().isEmpty()) {
                    rowHasData = true;
                }

                val.replace('"', "\"\"");
                values << QString("\"%1\"").arg(val);
            }

            if (rowHasData) {
                out << values.join(';') << '\n';
                exportedRows++;
            }
        }

        file.close();

        if (file.error() != QFile::NoError) {
            QMessageBox::critical(this, "Erreur",
                                  QString("Erreur lors de l'écriture du fichier !\nErreur: %1").arg(file.errorString()));
            return;
        }

        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists() || fileInfo.size() == 0) {
            QMessageBox::critical(this, "Erreur", "Le fichier n'a pas été créé ou est vide !");
            return;
        }

        if (exportedRows == 0) {
            QMessageBox::warning(this, "Export",
                                 "Aucune ligne de données valide n'a été exportée !");
            return;
        }

        QMessageBox::information(this, "Export Réussi",
                                 QString("Export CSV réussi !\n\n%1 projet(s) exporté(s) vers:\n%2")
                                     .arg(exportedRows)
                                     .arg(fileName));
    }

    // calendrier part
    void Draftify::loadProjectsFromDatabase()
    {
        ui->calendarWidgetprojet->setDateTextFormat(QDate(), QTextCharFormat());
        ui->listWidgetprojet->clear();

        // Légende
        ui->listWidgetprojet->addItem("=== LÉGENDE DU CALENDRIER ===");
        ui->listWidgetprojet->addItem("🔴 ROUGE: Projet urgent (≤3 jours) ou en retard");
        ui->listWidgetprojet->addItem("🟠 ORANGE: Projet à venir (≤7 jours)");
        ui->listWidgetprojet->addItem("🟠 ORANGE FONCÉ: Projet à venir (≤14 jours)");
        ui->listWidgetprojet->addItem("🟢 VERT: Projet terminé");
        ui->listWidgetprojet->addItem("⚪ BLANC: Projet normal (>14 jours)");
        ui->listWidgetprojet->addItem("");
        ui->listWidgetprojet->addItem("=== TOUS LES PROJETS ===");

        QSqlQuery query;
        query.prepare("SELECT p.TYPE_PROJET, p.DEADLINE, p.STATUT, p.ID_PROJ, c.NOM as CLIENT_NOM "
                      "FROM PROJET p "
                      "LEFT JOIN CLIENT c ON p.ID_CL = c.ID_CL "
                      "ORDER BY p.DEADLINE");

        if (!query.exec()) {
            qDebug() << "Erreur lors du chargement des projets:" << query.lastError().text();
            QMessageBox::warning(this, "Erreur", "Impossible de charger les projets pour le calendrier");
            return;
        }

        QMap<QDate, QList<QString>> projectsMap; // Date -> Liste de descriptions

        int projectsLoaded = 0;
        while (query.next()) {
            QString type = query.value("TYPE_PROJET").toString();
            QDate deadline = query.value("DEADLINE").toDate();
            QString statut = query.value("STATUT").toString();
            int id = query.value("ID_PROJ").toInt();
            QString client = query.value("CLIENT_NOM").toString();

            if (deadline.isValid()) {
                colorCodeProjectImproved(deadline, statut, type);

                QString projectDesc = QString("• %1 (ID%2) - %3 | Client: %4")
                                          .arg(type)
                                          .arg(id)
                                          .arg(statut)
                                          .arg(client.isEmpty() ? "Non assigné" : client);

                projectsMap[deadline].append(projectDesc);

                QString emoji = getStatusEmoji(statut, deadline);
                ui->listWidgetprojet->addItem(
                    QString("%1 %2 | Deadline: %3 | Statut: %4")
                        .arg(emoji, type, deadline.toString("dd/MM/yyyy"), statut)
                    );
                projectsLoaded++;
            }
        }

        updateCalendarTooltips(projectsMap);
        qDebug() << "Calendrier chargé avec" << projectsLoaded << "projets";
    }

    void Draftify::updateCalendarTooltips(const QMap<QDate, QList<QString>> &projectsMap)
    {
        for (auto it = projectsMap.begin(); it != projectsMap.end(); ++it) {
            QDate date = it.key();
            QStringList projects = it.value();

            if (projects.size() > 1) {
                QString tooltip = QString("<b>📅 %1</b><br>").arg(date.toString("dd/MM/yyyy"));
                tooltip += "<b>Projets:</b><br>";
                tooltip += projects.join("<br>");

                QTextCharFormat format = ui->calendarWidgetprojet->dateTextFormat(date);
                format.setToolTip(tooltip);
                ui->calendarWidgetprojet->setDateTextFormat(date, format);
            }
        }
    }

    QString Draftify::getStatusEmoji(const QString &status, const QDate &deadline)
    {
        QDate today = QDate::currentDate();
        QString statusLower = status.toLower();

        if (statusLower.contains("termine") || statusLower.contains("fini") ||
            statusLower.contains("done") || statusLower.contains("complete")) {
            return "✅";
        }

        if (deadline < today) {
            return "🔴";
        }

        int daysUntil = today.daysTo(deadline);
        if (daysUntil <= 3) return "🚨";
        if (daysUntil <= 7) return "⚠️";

        return "📅";
    }

    void Draftify::colorCodeProjectImproved(const QDate &deadline, const QString &status, const QString &projectType)
    {
        QTextCharFormat format;
        QDate today = QDate::currentDate();
        int daysUntilDeadline = today.daysTo(deadline);

        QString statusLower = status.toLower().trimmed();

        QString daysText;
        if (daysUntilDeadline < 0) {
            daysText = QString("🔴 <b>En retard de %1 jours</b>").arg(qAbs(daysUntilDeadline));
        } else if (daysUntilDeadline == 0) {
            daysText = "🚨 <b>Deadline aujourd'hui!</b>";
        } else if (daysUntilDeadline == 1) {
            daysText = "⚠️ <b>Deadline demain!</b>";
        } else {
            daysText = QString("📅 <b>Dans %1 jours</b>").arg(daysUntilDeadline);
        }

        QString tooltipText = QString(
                                  "<html><body style='font-family: Arial; font-size: 11px;'>"
                                  "<div style='background-color: #f8f9fa; padding: 8px; border-radius: 4px;'>"
                                  "<b style='color: #2c3e50;'>📋 %1</b><br>"
                                  "────────────────<br>"
                                  "📊 <b>Statut:</b> %2<br>"
                                  "📅 <b>Deadline:</b> %3<br>"
                                  "%4"
                                  "</div>"
                                  "</body></html>"
                                  ).arg(projectType.toHtmlEscaped())
                                  .arg(status.toHtmlEscaped())
                                  .arg(deadline.toString("dd/MM/yyyy"))
                                  .arg(daysText);

        if (statusLower.contains("termine") || statusLower.contains("fini") ||
            statusLower.contains("done") || statusLower.contains("complete") ||
            status.toUpper().contains("TERMINÉ")) {
            format.setBackground(QColor(40, 167, 69)); // Vert
            format.setForeground(Qt::white);
        }
        else if (deadline < today) {
            format.setBackground(QColor(220, 53, 69)); // Rouge
            format.setForeground(Qt::white);
        }
        else if (daysUntilDeadline <= 3) {
            format.setBackground(QColor(220, 53, 69)); // Rouge
            format.setForeground(Qt::white);
        }
        else if (daysUntilDeadline <= 7) {
            format.setBackground(QColor(255, 193, 7)); // Orange
            format.setForeground(Qt::black);
        }
        else if (daysUntilDeadline <= 14) {
            format.setBackground(QColor(253, 126, 20)); // Orange foncé
            format.setForeground(Qt::white);
        }
        else {
            format.setBackground(QColor(248, 249, 250)); // Gris clair
            format.setForeground(Qt::black);
        }

        format.setToolTip(tooltipText);
        format.setFontWeight(QFont::Bold);

        ui->calendarWidgetprojet->setDateTextFormat(deadline, format);
    }

    void Draftify::displayProjectsInList(const QDate &selectedDate)
    {
        QStringList globalItems;
        int itemsToSave = 8; // Légende + titre "TOUS LES PROJETS"
        for (int i = 0; i < itemsToSave && i < ui->listWidgetprojet->count(); i++) {
            globalItems << ui->listWidgetprojet->item(i)->text();
        }

        ui->listWidgetprojet->clear();

        for (const QString &item : globalItems) {
            ui->listWidgetprojet->addItem(item);
        }

        ui->listWidgetprojet->addItem("");
        ui->listWidgetprojet->addItem(QString("=== PROJETS DU %1 ===")
                                          .arg(selectedDate.toString("dd/MM/yyyy")));

        QSqlQuery query;
        query.prepare("SELECT TYPE_PROJET, STATUT, ID_PROJ "
                      "FROM PROJET "
                      "WHERE DEADLINE = TO_DATE(:date, 'YYYY-MM-DD') "
                      "ORDER BY STATUT, TYPE_PROJET");
        query.bindValue(":date", selectedDate.toString("yyyy-MM-dd"));

        if (!query.exec()) {
            qDebug() << "Erreur récupération projets date:" << query.lastError().text();
            ui->listWidgetprojet->addItem("❌ Erreur lors du chargement");
            return;
        }

        bool foundProjects = false;
        while (query.next()) {
            QString type = query.value("TYPE_PROJET").toString();
            QString statut = query.value("STATUT").toString();
            int id = query.value("ID_PROJ").toInt();

            QString emoji = getStatusEmoji(statut, selectedDate);
            ui->listWidgetprojet->addItem(
                QString("%1 ID%2: %3 | Statut: %4")
                    .arg(emoji)
                    .arg(id)
                    .arg(type)
                    .arg(statut)
                );
            foundProjects = true;
        }

        if (!foundProjects) {
            ui->listWidgetprojet->addItem("📭 Aucun projet pour cette date");
        }
    }

    ////verif part

    void Draftify::onCalendarDateSelected() {
        QDate selectedDate = ui->calendarWidgetprojet->selectedDate();
        qDebug() << "=== DEBUG == Date sélectionnée:" << selectedDate.toString("dd/MM/yyyy");

        ui->calendarWidgetprojet->update();
        ui->calendarWidgetprojet->repaint();

        ui->comboBoxproj->clear();

        ui->labelChecklistProject->setText("Chargement...");
        currentChecklistProjectId = -1;
        ui->listWidgetChecklist->clear();
        ui->listWidgetChecklist->setVisible(false);
        ui->labelChecklistStats->setText("");
        ui->btnAutoChecklist->setVisible(false);

        QSqlQuery query;
        query.prepare("SELECT ID_PROJ, TYPE_PROJET "
                      "FROM PROJET "
                      "WHERE DEADLINE = TO_DATE(:date, 'YYYY-MM-DD')");
        query.bindValue(":date", selectedDate.toString("yyyy-MM-dd"));

        if (query.exec()) {
            qDebug() << "=== DEBUG == Query executed successfully";
            int projectCount = 0;

            while (query.next()) {
                int projectId = query.value("ID_PROJ").toInt();
                QString projectType = query.value("TYPE_PROJET").toString();

                QString text = QString("Projet %1 - %2").arg(projectId).arg(projectType);
                ui->comboBoxproj->addItem(text, projectId);
                projectCount++;
            }

            qDebug() << "=== DEBUG == Total projects:" << projectCount;

            if (projectCount > 0) {
                ui->comboBoxproj->setCurrentIndex(0);
                currentChecklistProjectId = ui->comboBoxproj->itemData(0).toInt();
                currentChecklistProjectType = ui->comboBoxproj->currentText().split(" - ").last();

                displayChecklist(currentChecklistProjectId);
                ui->listWidgetChecklist->setVisible(true);
                ui->labelChecklistProject->setText("📋 " + ui->comboBoxproj->currentText());
            } else {
                ui->labelChecklistProject->setText("❌ Aucun projet cette date");
                ui->listWidgetChecklist->setVisible(false);
                ui->btnAutoChecklist->setVisible(false);
            }
        } else {
            qDebug() << "=== DEBUG == SQL error:" << query.lastError().text();
            ui->labelChecklistProject->setText("❌ Erreur de chargement");
            ui->listWidgetChecklist->setVisible(false);
        }
    }

    void Draftify::loadChecklistForSelectedProject(int projectId) {
        qDebug() << "=== DEBUG == Loading checklist for project ID:" << projectId;

        ui->listWidgetChecklist->blockSignals(true);

        ui->listWidgetChecklist->clear();

        QSqlQuery query;
        query.prepare("SELECT ID_CHECK, NOM_TACHE, EST_TERMINE, ORDRE_TACHE "
                      "FROM PROJET_CHECKLIST "
                      "WHERE ID_PROJ = :id_proj "
                      "ORDER BY ORDRE_TACHE");
        query.bindValue(":id_proj", projectId);

        int taskCount = 0;
        if (query.exec()) {
            while (query.next()) {
                QString taskName = query.value("NOM_TACHE").toString();
                bool isCompleted = query.value("EST_TERMINE").toBool();
                int order = query.value("ORDRE_TACHE").toInt();

                QListWidgetItem *item = new QListWidgetItem(taskName);
                item->setCheckState(isCompleted ? Qt::Checked : Qt::Unchecked);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setData(Qt::UserRole, order);

                ui->listWidgetChecklist->addItem(item);
                taskCount++;
            }
            updateChecklistStats();
        } else {
            qDebug() << "Error loading checklist:" << query.lastError().text();
        }

        ui->listWidgetChecklist->blockSignals(false);

        ui->btnAutoChecklist->setVisible(taskCount == 0);
        ui->listWidgetChecklist->setVisible(true);
    }

    void Draftify::on_comboBoxproj_currentIndexChanged(int index)
    {
        if (index < 0) return;

        currentChecklistProjectId = ui->comboBoxproj->itemData(index).toInt();
        qDebug() << "=== DEBUG == Project ID selected:" << currentChecklistProjectId;

        loadChecklistForSelectedProject(currentChecklistProjectId);

        QString selectedProjectText = ui->comboBoxproj->currentText();
        ui->labelChecklistProject->setText("📋 " + selectedProjectText);
    }

    // 1. Templates de checklist
    QMap<QString, QStringList> Draftify::loadChecklistTemplates()
    {
        QMap<QString, QStringList> templates;

        templates["VIDEO"] = {
            "📝 Script écrit et validé",
            "🎬 Tournage planifié et exécuté",
            "✂️ Montage brut terminé",
            "🎵 Sound design et mixage audio",
            "🎨 Color grading et effets visuels",
            "📏 Export aux formats requis",
            "✅ Révision qualité finale",
            "🚀 Livraison au client"
        };

        templates["IMAGE"] = {
            "🖼️ Séance photo/création graphique",
            "📐 Cadrage et composition validés",
            "🎨 Retouches et traitements appliqués",
            "🌈 Correction couleur et luminosité",
            "📱 Adaptation multi-formats",
            "🔍 Vérification résolution DPI",
            "✅ Validation artistique",
            "🚀 Livraison finale"
        };

        templates["TEXTE"] = {
            "✍️ Recherche et documentation",
            "📄 Premier jet rédigé",
            "📖 Structure et plan vérifiés",
            "🔍 Relecture et correction",
            "🎯 Adaptation ton et style",
            "📊 Chiffres et données validés",
            "✅ Approbation finale",
            "🚀 Publication/diffusion"
        };

        templates["DEFAULT"] = {
            "📋 Analyse des besoins",
            "🛠️ Production/création",
            "✏️ Révision et corrections",
            "✅ Validation qualité",
            "🚀 Livraison/finalisation"
        };

        return templates;
    }

    // 2. Génération de checklist
    void Draftify::generateChecklistForProject(int projectId, const QString &projectType) {
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM PROJET_CHECKLIST WHERE ID_PROJ = :id_proj");
        checkQuery.bindValue(":id_proj", projectId);

        if (checkQuery.exec() && checkQuery.next()) {
            int count = checkQuery.value(0).toInt();
            if (count > 0) {
                qDebug() << "Checklist already exists for project ID" << projectId;
                return;
            }
        }

        QMap<QString, QStringList> templates = loadChecklistTemplates();
        QString templateKey = "DEFAULT";
        QString typeUpper = projectType.toUpper();

        if (typeUpper.contains("VIDEO") || typeUpper.contains("VIDÉO")) templateKey = "VIDEO";
        else if (typeUpper.contains("IMAGE") || typeUpper.contains("PHOTO")) templateKey = "IMAGE";
        else if (typeUpper.contains("TEXTE") || typeUpper.contains("RÉDACTION")) templateKey = "TEXTE";

        QStringList tasks = templates[templateKey];
        qDebug() << "Using template:" << templateKey << "for project:" << projectType;

        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM PROJET_CHECKLIST WHERE ID_PROJ = :id_proj");
        deleteQuery.bindValue(":id_proj", projectId);
        deleteQuery.exec();

        for (int i = 0; i < tasks.size(); ++i) {
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO PROJET_CHECKLIST (ID_CHECK, ID_PROJ, NOM_TACHE, EST_TERMINE, STATUT_TACHE, ORDRE_TACHE) "
                                "VALUES (:id_check, :id_proj, :nom_tache, :est_termine, 'EN_COURS', :ordre)");

            insertQuery.bindValue(":id_check", 1);
            insertQuery.bindValue(":id_proj", projectId);
            insertQuery.bindValue(":nom_tache", tasks[i]);
            insertQuery.bindValue(":est_termine", 0);
            insertQuery.bindValue(":ordre", i);

            if (insertQuery.exec()) {
                qDebug() << "=== DEBUG == Task inserted - ID_CHECK: 1, ID_PROJ:" << projectId
                         << "Order:" << i << "Task:" << tasks[i] << "EST_TERMINE: 0";
            } else {
                qDebug() << "=== DEBUG == Insert error:" << insertQuery.lastError().text();
            }
        }

        qDebug() << "Checklist generated:" << tasks.size() << "tasks for project ID" << projectId;
    }

    void Draftify::onProjectComboChanged()
    {
        int index = comboBoxproj->currentIndex();
        if (index >= 0) {
            currentChecklistProjectId = comboBoxproj->itemData(index).toInt();

            QSqlQuery query;
            query.prepare("SELECT TYPE_PROJET FROM PROJET WHERE ID_PROJ = :id");
            query.bindValue(":id", currentChecklistProjectId);
            if (query.exec() && query.next()) {
                currentChecklistProjectType = query.value("TYPE_PROJET").toString();
            }

            ui->labelChecklistProject->setText("📋 " + comboBoxproj->currentText());

            QPoint comboPos = comboBoxproj->pos();
            QSize comboSize = comboBoxproj->size();
            int yOffset = comboPos.y() + comboSize.height() + 10;

            QSize statsSize;
            QSize listSize;

            if (ui->labelChecklistStats) {
                ui->labelChecklistStats->move(comboPos.x(), yOffset);
                statsSize = ui->labelChecklistStats->size();
            }

            if (ui->listWidgetChecklist) {
                ui->listWidgetChecklist->move(comboPos.x(), yOffset + statsSize.height() + 5);
                listSize = ui->listWidgetChecklist->size();
            }

            if (ui->btnAutoChecklist) {
                ui->btnAutoChecklist->move(comboPos.x(), yOffset + statsSize.height() + listSize.height() + 10);
            }

            displayChecklist(currentChecklistProjectId);
        }
    }

    void Draftify::onChecklistItemChanged(QListWidgetItem *item) {
        if (!item) return;

        ui->listWidgetChecklist->blockSignals(true);

        int order = item->data(Qt::UserRole).toInt();
        bool newState = (item->checkState() == Qt::Checked);

        qDebug() << "=== DEBUG == Updating task Order:" << order << "to state:" << newState;

        QSqlQuery query;
        query.prepare("UPDATE PROJET_CHECKLIST "
                      "SET EST_TERMINE = :est_termine "
                      "WHERE ID_PROJ = :id_proj AND ORDRE_TACHE = :ordre");
        query.bindValue(":est_termine", newState ? 1 : 0);
        query.bindValue(":id_proj", currentChecklistProjectId);
        query.bindValue(":ordre", order);

        if (query.exec()) {
            QString originalText = item->text().replace("✅ ", "").replace("⏳ ", "");
            item->setText(newState ? "✅ " + originalText : "⏳ " + originalText);
            item->setBackground(newState ? QColor(220, 255, 220) : Qt::white);
            updateChecklistStats();

            checkAndUpdateProjectStatus(currentChecklistProjectId);
        } else {
            qDebug() << "Error updating task:" << query.lastError().text();
            item->setCheckState(newState ? Qt::Unchecked : Qt::Checked);
        }

        ui->listWidgetChecklist->blockSignals(false);
    }

    // 3. Affichage de la checklist
    void Draftify::displayChecklist(int projectId) {
        qDebug() << "=== DEBUG == displayChecklist() called for project ID:" << projectId;

        if (!ui->listWidgetChecklist) return;

        disconnect(ui->listWidgetChecklist, &QListWidget::itemChanged, this, &Draftify::onChecklistItemChanged);

        ui->listWidgetChecklist->blockSignals(true);
        ui->listWidgetChecklist->clear();
        ui->listWidgetChecklist->setVisible(true);

        connect(ui->listWidgetChecklist, &QListWidget::itemChanged, this, &Draftify::onChecklistItemChanged);

        QSqlQuery query;
        query.prepare("SELECT ID_CHECK, NOM_TACHE, EST_TERMINE, ORDRE_TACHE "
                      "FROM PROJET_CHECKLIST "
                      "WHERE ID_PROJ = :id_proj "
                      "ORDER BY ORDRE_TACHE");
        query.bindValue(":id_proj", projectId);

        int taskCount = 0;
        if (query.exec()) {
            while (query.next()) {
                int itemId = query.value("ID_CHECK").toInt();
                QString taskName = query.value("NOM_TACHE").toString();
                bool isCompleted = query.value("EST_TERMINE").toBool();
                int order = query.value("ORDRE_TACHE").toInt();

                QListWidgetItem *item = new QListWidgetItem(taskName);
                item->setCheckState(isCompleted ? Qt::Checked : Qt::Unchecked);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setData(Qt::UserRole, order);

                qDebug() << "=== DEBUG == Task loaded - Order:" << order << "Completed:" << isCompleted;

                ui->listWidgetChecklist->addItem(item);
                taskCount++;
            }
            updateChecklistStats();
            checkAndUpdateProjectStatus(projectId);
        } else {
            qDebug() << "Error loading tasks:" << query.lastError().text();
            taskCount = 0;
        }

        ui->listWidgetChecklist->blockSignals(false);
        ui->btnAutoChecklist->setVisible(taskCount == 0);
    }

    // stats marbouta bel verif
    void Draftify::updateChecklistStats() {
        if (!ui->listWidgetChecklist || !ui->labelChecklistStats) return;

        int completedCount = 0;
        int totalCount = ui->listWidgetChecklist->count();

        for (int i = 0; i < totalCount; ++i) {
            QListWidgetItem *item = ui->listWidgetChecklist->item(i);
            if (item && item->checkState() == Qt::Checked) {
                completedCount++;
            }
        }

        if (totalCount > 0) {
            int percentage = (completedCount * 100) / totalCount;

            if (percentage == 100) {
                ui->labelChecklistStats->setText("🎉 100% - PROJET TERMINÉ !");
                ui->labelChecklistStats->setStyleSheet("color: green; font-weight: bold;");
            } else {
                ui->labelChecklistStats->setText(QString("📊 %1/%2 • %3% terminé").arg(completedCount).arg(totalCount).arg(percentage));
                ui->labelChecklistStats->setStyleSheet("");
            }
        } else {
            ui->labelChecklistStats->setText("📊 Aucune tâche - Générer une checklist");
        }
    }

    // 4. Gestion du bouton
    void Draftify::onAutoChecklistClicked()
    {
        if (currentChecklistProjectId == -1) {
            QMessageBox::information(this, "Checklist", "📅 Sélectionnez d'abord une date avec projet!");
            return;
        }

        generateChecklistForProject(currentChecklistProjectId, currentChecklistProjectType);

        displayChecklist(currentChecklistProjectId);

        QMessageBox::information(this, "✅ Checklist Auto",
                                 QString("Checklist générée pour:\n%1 (ID%2)").arg(currentChecklistProjectType).arg(currentChecklistProjectId));
    }

    ///*** projet oufe k tkamel les metiers mteik ***////
    void Draftify::checkAndUpdateProjectStatus(int projectId) {
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) as total, SUM(EST_TERMINE) as completed "
                      "FROM PROJET_CHECKLIST WHERE ID_PROJ = :id_proj");
        query.bindValue(":id_proj", projectId);

        if (query.exec() && query.next()) {
            int totalTasks = query.value("total").toInt();
            int completedTasks = query.value("completed").toInt();

            qDebug() << "=== DEBUG == Project" << projectId << "- Completed:" << completedTasks << "/" << totalTasks;

            if (totalTasks > 0 && completedTasks == totalTasks) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE PROJET SET STATUT = 'TERMINÉ' WHERE ID_PROJ = :id_proj");
                updateQuery.bindValue(":id_proj", projectId);

                if (updateQuery.exec()) {
                    qDebug() << "=== DEBUG == Project" << projectId << "marked as TERMINÉ automatically!";

                    ui->labelChecklistProject->setText("🎉 " + ui->labelChecklistProject->text());
                    ui->labelChecklistStats->setText("✅ PROJET TERMINÉ - Toutes les tâches accomplies!");

                    loadProjectsFromDatabase();
                    actualiserTable();

                } else {
                    qDebug() << "=== DEBUG == Error updating project status:" << updateQuery.lastError().text();
                }
            }
        }
    }

    // ***stats***
    static void clearWidgetproj(QWidget* w) {
        if (!w) return;
        if (auto* layout = w->layout()) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (auto* child = item->widget())
                    child->deleteLater();
                delete item;
            }
            delete layout;
        }
    }

    void Draftify::mettreAJourStats()
    {
        QMap<QString, int> statsStatut = Projet::getStatistiquesStatut();
        int totalProjets   = Projet::getTotalProjets();
        int projetsEnRetard = Projet::getProjetsEnRetard();

        if (totalProjets == 0 && !statsStatut.isEmpty()) {
            for (auto it = statsStatut.cbegin(); it != statsStatut.cend(); ++it) {
                totalProjets += it.value();
            }
        }

        int termines = 0;
        for (auto it = statsStatut.cbegin(); it != statsStatut.cend(); ++it) {
            QString statutLower = it.key().trimmed().toLower();
            if (statutLower.contains("terminé")  ||
                statutLower.contains("termine")   ||
                statutLower.contains("finished")  ||
                statutLower.contains("completé")  ||
                statutLower.contains("complete")  ||
                statutLower.contains("done")) {
                termines += it.value();
            }
        }

        QString html = "<html><body style='font-family: Arial; margin: 10px;'>";

        html += "<div style='display: flex; justify-content: space-between; margin-bottom: 20px;'>";

        html += "<div style='background: #3498db; color: white; padding: 15px; border-radius: 8px; text-align: center; width: 30%;'>";
        html += "<div style='font-size: 14px;'>📋 Total</div>";
        html += "<div style='font-size: 24px; font-weight: bold;'>" + QString::number(totalProjets) + "</div>";
        html += "</div>";

        html += "<div style='background: #e74c3c; color: white; padding: 15px; border-radius: 8px; text-align: center; width: 30%;'>";
        html += "<div style='font-size: 14px;'>⚠️ En Retard</div>";
        html += "<div style='font-size: 24px; font-weight: bold;'>" + QString::number(projetsEnRetard) + "</div>";
        html += "</div>";

        html += "<div style='background: #27ae60; color: white; padding: 15px; border-radius: 8px; text-align: center; width: 30%;'>";
        html += "<div style='font-size: 14px;'>🟢 Terminés</div>";
        html += "<div style='font-size: 24px; font-weight: bold;'>" + QString::number(termines) + "</div>";
        html += "</div>";

        html += "</div>";

        html += "<h4 style='color: #2c3e50; margin-bottom: 10px;'>📊 Répartition par Statut :</h4>";

        if (statsStatut.isEmpty()) {
            html += "<p style='color: gray;'>Aucun projet dans la base de données</p>";
        } else {
            for (auto it = statsStatut.cbegin(); it != statsStatut.cend(); ++it) {
                QString statut = it.key().trimmed();
                int count      = it.value();

                double pourcentage = (totalProjets > 0)
                                         ? (count * 100.0 / totalProjets)
                                         : 0.0;

                QString couleurFond = "#e8f4fd";
                QString statutLower = statut.toLower();

                if (statutLower.contains("terminé") || statutLower.contains("finished")) {
                    couleurFond = "#d4edda";
                } else if (statutLower.contains("retard") || statutLower.contains("urgent")) {
                    couleurFond = "#f8d7da";
                } else if (statutLower.contains("en cours")) {
                    couleurFond = "#fff3cd";
                }

                html += QString(
                            "<div style='background: %1; margin: 4px 0; padding: 6px; border-radius: 4px;'>"
                            "<span style='font-weight: bold;'>%2</span>"
                            "<span style='float: right; font-weight: bold;'>%3 ( %4%% )</span>"
                            "</div>")
                            .arg(couleurFond)
                            .arg(statut)
                            .arg(count)
                            .arg(QString::number(pourcentage, 'f', 1));
            }
        }

        if (projetsEnRetard > 0) {
            html += QString(
                        "<div style='background: #fff3cd; color: #856404; padding: 8px; border-radius: 4px; "
                        "margin-top: 10px; border-left: 4px solid #ffc107;'>"
                        "⚠️ <b>Attention :</b> %1 projet(s) en retard !</div>")
                        .arg(projetsEnRetard);
        }

        html += "</body></html>";

        ui->textEdit->setReadOnly(true);
        ui->textEdit->setHtml(html);
    }

    void Draftify::animateBarSetproj(QBarSet *set)
    {
        if (!set || set->count() == 0)
            return;

        QVector<qreal> finalValues;
        finalValues.reserve(set->count());
        for (int i = 0; i < set->count(); ++i) {
            finalValues.append(set->at(i));
            set->replace(i, 0.0);
        }

        auto *animation = new QVariantAnimation(this);
        animation->setDuration(1500);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);

        connect(animation, &QVariantAnimation::valueChanged, this,
                [set, finalValues](const QVariant &value) {
                    qreal progress = value.toReal();
                    for (int i = 0; i < finalValues.size(); ++i) {
                        set->replace(i, finalValues[i] * progress);
                    }
                });

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void Draftify::afficherStatsProjets()
    {
        clearWidgetproj(ui->widgetStatsChart);

        QMap<QString, int> statsStatut = Projet::getStatistiquesStatut();
        if (statsStatut.isEmpty())
            return;

        QStringList categories;
        auto* set = new QBarSet("Projets");
        int maxCount = 0;

        for (auto it = statsStatut.cbegin(); it != statsStatut.cend(); ++it) {
            categories << it.key();
            *set << it.value();
            maxCount = qMax(maxCount, it.value());
        }

        auto* series = new QBarSeries();
        series->append(set);
        series->setLabelsVisible(true);
        series->setLabelsFormat("@value");

        animateBarSet(set);

        auto* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Répartition des projets par statut");
        chart->legend()->setVisible(true);

        auto* axX = new QBarCategoryAxis();
        axX->append(categories);
        axX->setTitleText("Statut");

        auto* axY = new QValueAxis();
        axY->setTitleText("Nombre");
        axY->setMin(0);
        axY->setMax(maxCount == 0 ? 5 : maxCount + 1);

        chart->addAxis(axX, Qt::AlignBottom);
        chart->addAxis(axY, Qt::AlignLeft);
        series->attachAxis(axX);
        series->attachAxis(axY);

        auto* view = new QChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);

        auto* layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(view);
        ui->widgetStatsChart->setLayout(layout);
    }

    void Draftify::on_btnStats_clicked()
    {
        bool willShow = !ui->frameStats->isVisible();

        if (willShow) {
            mettreAJourStats();
            afficherStatsProjets();

            ui->frameStats->show();

            int x = (this->width() - ui->frameStats->width()) / 2;
            int y = (this->height() - ui->frameStats->height()) / 2;
            ui->frameStats->move(x, y);
            ui->frameStats->raise();
        } else {
            ui->frameStats->hide();
        }
    }













//_________________EMPL METIERS____________________________________________________________________________


    void Draftify::on_comboBoxSortEmpl_currentIndexChanged(int index)
    {
        QString critere;

        // Define the sorting criteria based on ComboBox index
        if (index == 0) {  // Sort by Name
            critere = "nom";
        }
        else if (index == 1) {  // Sort by Post
            critere = "post";
        }
        else {
            // Clear the comboBox selection after applying the sort
            ui->comboBoxSortEmpl->setCurrentIndex(-1);  // Set the comboBox to no selection (empty)
        }

        // Update the table with sorted data based on selected criteria
        ui->tableView_EMPL->setModel(Etmp.trier(critere));

    }



    void Draftify::on_PDF_ExportEmpl_clicked()
    {
        QAbstractItemModel* m = ui->tableView_EMPL->model();
        if (!m) {
            QMessageBox::warning(this, tr("Export"), tr("No data to export."));
            return;
        }

        const QString suggested = QDir::homePath()
                                  + "/employees_" + QDateTime::currentDateTime().toString("yyyy_MM_dd__HH-mm") + ".csv";

        const QString path = QFileDialog::getSaveFileName(
            this, tr("Export employees to CSV"), suggested,
            tr("CSV files (*.csv);;All files (*.*)")
            );
        if (path.isEmpty()) return;

        if (Employe::exportCsvFromModel(m, path)) {
            QMessageBox::information(this, tr("Export"), tr("Export completed:\n%1").arg(path));
        } else {
            QMessageBox::critical(this, tr("Export"), tr("Failed to write the file."));
        }
    }


    void Draftify::on_comboBoxPostEmpl_currentIndexChanged(int index)
    {


        int salary = 0;  // We will set this based on the post
        QString role = "";  // We will set this based on the post

        // Get the selected post from comboBox
        QString poste = ui->comboBoxPostEmpl->currentText(); // Get selected post

        // Map the post to the corresponding salary and role
        if (poste == "Video Editor") {
            salary = 3500;
            role = "Employee";
        } else if (poste == "Graphic Designer") {
            salary = 3000;
            role = "Employee";
        } else if (poste == "Social Media Manager") {
            salary = 2500;
            role = "Social Media";
        } else if (poste == "Photographer") {
            salary = 2800;
            role = "Employee";
        } else if (poste == "Project Manager") {
            salary = 4500;
            role = "Manager";
        } else if (poste == "Client Manager") {
            salary = 4500;
            role = "Manager";
        } else if (poste == "Developer") {
            salary = 4000;
            role = "Employee";
        } else if (poste == "Admin") {
            salary = 5000;
            role = "Admin";
        } else if (poste == "HR Manager") {
            salary = 4500;
            role = "HR";
        }

        // Update the salary and role fields (optional, to automatically show the salary and role in the UI)
        ui->lineEditSalaryEmpl->setText(QString::number(salary));
        ui->lineEditRoleEmpl->setText(role);

    }

//stat________
static void clearWidget(QWidget* w) {
        if (!w) return;

        QLayout* layout = w->layout();
        if (!layout) return;

        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            QWidget* childWidget = item->widget();
            if (childWidget)
                childWidget->deleteLater();
            delete item;
        }

        delete layout;
    }
void Draftify::afficherStatsSalaireParPoste()
    {
        // 0) Nettoyer le widget cible
        clearWidget(ui->ChartViewEmpl);   // ton QWidget dans l'UI

        // 1) Récupérer les données depuis Employe
        QStringList categories;   // posts
        QVector<int> totals;      // total salary per post
        Employe::getSalaireParPoste(categories, totals);

        // Si aucune donnée → afficher un chart vide avec message
        if (categories.isEmpty()) {
            QChart* emptyChart = new QChart();
            emptyChart->setTitle(tr("Total salary per post (no data)"));


            QChartView* emptyView = new QChartView(emptyChart);
            emptyView->setRenderHint(QPainter::Antialiasing);

            QVBoxLayout* emptyLayout = new QVBoxLayout();
            emptyLayout->setContentsMargins(0, 0, 0, 0);
            emptyLayout->addWidget(emptyView);
            ui->ChartViewEmpl->setLayout(emptyLayout);
            return;
        }

        // 2) Construire le QBarSet à partir des totals
        QBarSet* set = new QBarSet(tr("Total salary"));
        int maxTotal = 0;

        for (int i = 0; i < totals.size(); ++i) {
            int value = totals[i];
            *set << value;
            if (value > maxTotal)
                maxTotal = value;
        }
        set->setColor(QColor("#6A0DAD"));        // dark purple
        set->setBorderColor(QColor("#4B0082"));  // slightly darker outline
        set->setLabelColor(Qt::white);           // label text on bars

        // 3) Série / chart (barres VERTICALES)
        QBarSeries* series = new QBarSeries();
        series->append(set);
        series->setLabelsVisible(true);
        series->setLabelsFormat("@value");

        // Animation sur les barres
        animateBarSet(set);

        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(tr("Total salary per post"));
        chart->legend()->setVisible(false);
        chart->setBackgroundBrush(QBrush(QColor("#F0E5FF"))); // soft purple
        chart->setBackgroundPen(Qt::NoPen);                   // remove border
        // 4) Axes
        QBarCategoryAxis* axX = new QBarCategoryAxis();
        axX->append(categories);
        axX->setTitleText(tr("Post"));

        QValueAxis* axY = new QValueAxis();
        axY->setTitleText(tr("Total salary"));
        axY->setLabelFormat("%d");
        axY->setMin(0);
        axY->setMax(maxTotal == 0 ? 10 : maxTotal + 500);

        chart->addAxis(axX, Qt::AlignBottom);
        chart->addAxis(axY, Qt::AlignLeft);
        series->attachAxis(axX);
        series->attachAxis(axY);

        // COLOR THE AXIS TEXT (ADD HERE)
        axX->setLabelsColor(QColor("#4B0082"));
        axY->setLabelsColor(QColor("#4B0082"));

        axX->setTitleBrush(QBrush(QColor("#4B0082")));
        axY->setTitleBrush(QBrush(QColor("#4B0082")));

        // soften grid lines
        axY->setGridLineColor(QColor("#DCCFFF"));

        // 5) Mettre le chart dans le widget de l'UI
        QChartView* view = new QChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);

        // background of viewer (not chart)
        view->setStyleSheet("background-color: transparent;");

        QVBoxLayout* layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(view);

        ui->ChartViewEmpl->setLayout(layout);
    }
void Draftify::animateBarSet(QBarSet *set)
    {
        if (set == nullptr || set->count() == 0)
            return;

        // 1) Sauvegarder les valeurs finales
        QVector<qreal> finalValues;
        finalValues.reserve(set->count());

        for (int i = 0; i < set->count(); ++i) {
            finalValues.append(set->at(i));
            set->replace(i, 0.0);  // démarrer à 0
        }

        // 2) Animation 0 → 1
        QVariantAnimation* animation = new QVariantAnimation(this);
        animation->setDuration(1500);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);

        QObject::connect(animation, &QVariantAnimation::valueChanged,
                         this,
                         [set, finalValues](const QVariant &value) {
                             qreal progress = value.toReal(); // entre 0 et 1
                             for (int i = 0; i < finalValues.size(); ++i) {
                                 set->replace(i, finalValues[i] * progress);
                             }
                         });

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

void Draftify::on_StatEmpl_clicked()
    {
         afficherStatsSalaireParPoste();
    }


    void Draftify::on_NextForgotPasswordEmpl_clicked()
    {
         QString email = ui->EmailForgotPssswdEmpl->text().trimmed();
        QString newPassword = ui->NewPasswordEmpl->text().trimmed();
        QString confirmPassword = ui->ConfirmNewPasswordEmpl->text().trimmed();
        Employe e;

        // Step 7: Validate the new password fields
        if (newPassword.isEmpty() || confirmPassword.isEmpty()) {
            QMessageBox::warning(this, "Password Required", "Please enter and confirm your new password.");
            return;
        }

        if (newPassword != confirmPassword) {
            QMessageBox::warning(this, "Password Mismatch", "The passwords do not match. Please try again.");
            return;
        }

        // Step 8: Update the password in the database
        bool success = e.updatePassword(email, newPassword);  // Assuming this function updates the password in the database

        if (success) {
            QMessageBox::information(this, "Password Reset", "Your password has been successfully updated.");
            // Optionally, navigate to the login page or reset the fields
            ui->stackedWidgetEmpl->setCurrentIndex(0);  // Go back to the login page
        } else {
            QMessageBox::warning(this, "Error", "There was an error updating your password. Please try again.");
        }
    }


    void Draftify::on_SignupEmpl_clicked()
    {

        // Step 1: Get the email entered by the user
        QString email = ui->EmailIDLoginEmpl->text().trimmed();

        // Step 2: Validate if the email is empty
        if (email.isEmpty()) {
            QMessageBox::warning(this, "Email Required", "Please enter an email address to proceed.");
            return;  // Stop if email is empty
        }

        // Step 3: Check if the email exists in the database
        Employe e;
        bool emailExists = e.isEmailExist(email);  // Check if email exists in the database

        if (emailExists) {
            // Step 4: Check if the email has a password or not
            bool hasPassword = e.isPasswordExist(email);  // Check if there is an associated password

            if (hasPassword) {
                // Account is fully registered and password exists, proceed to login
                QMessageBox::information(this, "Login", "This email is registered. Please log in.");
                ui->stackedWidgetEmpl->setCurrentIndex(1);  // Navigate to the Login page
            } else {
                // If the email exists but no password is set, guide the user to complete the sign-up
                QMessageBox::warning(this, "Sign Up", "This email exists, but the account is not fully set up. Please complete the sign-up process.");
                ui->stackedWidgetEmpl->setCurrentIndex(3);  // Navigate to the Sign-Up page
            }
        } else {
            // If the email doesn't exist, navigate to the Sign-Up page
            QMessageBox::warning(this, "Email Not Found", "This email address doesn't exist. Please proceed to Sign Up.");
            ui->stackedWidgetEmpl->setCurrentIndex(3);  // Navigate to the Sign-Up page
        }
    }



    void Draftify::on_ConfirmSecurityQstEmpl_clicked()
    {
        QString email = ui->EmailSignUpEmpl->text().trimmed();
        QString securityQuestion = ui->SecurityQstSignUpEmpl->text().trimmed();
        QString answer = ui->AnswerQstSignUpEmpl->text().trimmed();
        QString password = ui->PasswordSignUpEmpl->text().trimmed();
        QString confirmPassword = ui->ConfirmPasswordSignUpEmpl->text().trimmed();

        // Step 1: Validate Email
        Employe e;
        bool emailExists = e.isEmailExist(email);  // Check if the email exists in the database

        if (!emailExists) {
            QMessageBox::warning(this, "Email Not Found", "This email is not found in our system. Please register first.");
            return;
        }

        // Step 2: Validate Password
        if (password != confirmPassword) {
            QMessageBox::warning(this, "Password Mismatch", "Passwords do not match.");
            return;
        }

        // Step 3: Validate Password Strength
        if (!e.validatePassword(password)) {
            QMessageBox::warning(this, "Weak Password", "Password must contain at least one uppercase letter, one lowercase letter, one digit, and one special character.");
            return;
        }

        // Step 4: Validate Security Question
        if (securityQuestion.isEmpty()) {
            QMessageBox::warning(this, "Empty Question", "Please provide a security question.");
            return;
        }

        if (!securityQuestion[0].isUpper()) {  // Ensure the question starts with an uppercase letter
            QMessageBox::warning(this, "Invalid Question", "The security question must start with an uppercase letter.");
            return;
        }

        if (!securityQuestion.endsWith('?')) {  // Ensure the question ends with a "?"
            QMessageBox::warning(this, "Invalid Question", "The security question must end with a '?'");
            return;
        }
        if (answer.isEmpty()) {  // Check if the answer field is empty
            QMessageBox::warning(this, "Empty Answer", "Please provide an answer to the security question.");
            return;
        }

        // Step 5: Save Password & Security Question
        bool success = e.savePassword(email, password);  // Save the password (hashed)
        if (!success) {
            QMessageBox::critical(this, "Error", "Failed to update password. Please try again.");
            return;
        }

        bool successQst = e.saveSecurityQuestion(email, securityQuestion, answer);  // Save the security question and answer
        if (!successQst) {
            QMessageBox::critical(this, "Error", "Failed to set security question. Please try again.");
            return;
        }

        // Step 6: Provide Feedback to User
        QMessageBox::information(this, "Account Created", "Account created successfully!");

        // Step 7: Clear Fields
        ui->EmailSignUpEmpl->clear();
        ui->PasswordSignUpEmpl->clear();
        ui->ConfirmPasswordSignUpEmpl->clear();
        ui->SecurityQstSignUpEmpl->clear();
        ui->AnswerQstSignUpEmpl->clear();
        ui->stackedWidgetEmpl->setCurrentIndex(0);
    }


    void Draftify::on_ShowPasswordSignUpEmpl_stateChanged(int state)
    {
        if (state == Qt::Checked) {
            // If checkbox is checked, show the password
            ui->PasswordSignUpEmpl->setEchoMode(QLineEdit::Normal);
            ui->ConfirmPasswordSignUpEmpl->setEchoMode(QLineEdit::Normal);
        } else {
            // If checkbox is unchecked, hide the password
            ui->PasswordSignUpEmpl->setEchoMode(QLineEdit::Password);
            ui->ConfirmPasswordSignUpEmpl->setEchoMode(QLineEdit::Password);
        }
    }


    void Draftify::on_ShowPasswordLoginEmpl_stateChanged(int state)
    {
        if (state == Qt::Checked) {
            // If checkbox is checked, show the password
            ui->PasswordLoginEmpl->setEchoMode(QLineEdit::Normal);
        } else {
            // If checkbox is unchecked, hide the password
            ui->PasswordLoginEmpl->setEchoMode(QLineEdit::Password);
        }
    }

    int questionAttempts = 0;  // Track failed question attempts
    int answerAttempts = 0;    // Track failed answer attempts
    void Draftify::on_ConfirmPickQuestionEmpl_clicked()
    {
        QString email = ui->EmailForgotPssswdEmpl->text().trimmed();
        QString selectedQuestion = ui->comboBoxQstsEmpl->currentText().trimmed();


        // Step 2: Validate if the selected question is correct
        Employe e;
        bool isValidQuestion = e.validateSecurityQuestion(email, selectedQuestion);

        if (!isValidQuestion) {
            questionAttempts++;  // Increment question attempt count
            if (questionAttempts >= 3) {
                QMessageBox::warning(this, "Too Many Attempts", "You have exceeded the maximum number of attempts. Please contact HR.");
                ui->stackedWidgetEmpl->setCurrentIndex(0);
                return;
            }
            QMessageBox::warning(this, "Incorrect Question", "The security question is incorrect.");
            return;
        }

        // Step 3: If the question is correct, display the answer part and disable the question part
        ui->QstPickedRecoverEmpl->setText(selectedQuestion);  // Display the correct question in the lineEdit
        ui->comboBoxQstsEmpl->setDisabled(true);           // Disable the combo box
        ui->ConfirmPickQuestionEmpl->setDisabled(true);      // Disable the first confirm button
        ui->ConfirmAnswerQstRecoverEmpl->setEnabled(true);        // Enable the answer input field

        // Proceed to the next step
        QMessageBox::information(this, "Correct Question", "This is the correct security question. Please enter your answer.");
    }


    void Draftify::on_ConfirmAnswerQstRecoverEmpl_clicked()
    {
        QString email = ui->EmailForgotPssswdEmpl->text().trimmed();
        QString answer = ui->RightAnswerEmpl->text().trimmed();

        // Step 1: Check if the answer is empty
        if (answer.isEmpty()) {
            QMessageBox::warning(this, "Invalid Answer", "Please provide an answer.");
            return;
        }

        // Step 2: Validate if the answer is correct
        Employe e;
        bool isValidAnswer = e.validateSecurityAnswer(email, answer);

        if (!isValidAnswer) {
            answerAttempts++;  // Increment answer attempt count
            if (answerAttempts >= 3) {
                QMessageBox::warning(this, "Too Many Attempts", "You have exceeded the maximum number of attempts. Please contact HR.");
                ui->stackedWidgetEmpl->setCurrentIndex(0);
                return;
            }
            QMessageBox::warning(this, "Incorrect Answer", "The answer to the security question is incorrect.");
            return;
        }

        // Step 3: If the answer is correct, proceed to reset the password
        ui->stackedWidgetEmpl->setCurrentIndex(1);  // Assuming reset password page is at index 3

    }


    void Draftify::on_ShowPasswordChangePasswordEmpl_stateChanged(int state)
    {
        if (state == Qt::Checked) {
            // If checkbox is checked, show the password
            ui->NewPasswordEmpl->setEchoMode(QLineEdit::Normal);
            ui->ConfirmNewPasswordEmpl->setEchoMode(QLineEdit::Normal);
        } else {
            // If checkbox is unchecked, hide the password
            ui->NewPasswordEmpl->setEchoMode(QLineEdit::Password);
             ui->ConfirmNewPasswordEmpl->setEchoMode(QLineEdit::Password);
        }
    }
    void Draftify::on_pushButton_19_clicked()
    {
        QString texte = ui->lineEdit_19->text().trimmed();
        Client c;
        QSqlQueryModel *model;

        if (texte.isEmpty()) {
            model = c.afficher();
        } else {
            model = c.rechercher(texte);
        }

        if (!model) return;

        ui->tableWidgetcl->setRowCount(model->rowCount());
        ui->tableWidgetcl->setColumnCount(10);
        ui->tableWidgetcl->setHorizontalHeaderLabels(
            {"ID", "Nom", "Prénom", "Téléphone", "Email", "Type", "Followers", "Social", "Photo", "Rating"});

        for (int r = 0; r < model->rowCount(); r++) {
            for (int col = 0; col < 10; col++) {
                QString data = model->data(model->index(r, col)).toString();
                ui->tableWidgetcl->setItem(r, col, new QTableWidgetItem(data));
            }
        }
        // Memory management: model is created by new in Client methods, should be deleted or parented.
        // Since we just extract data, we should delete it.
        delete model;
    }
    void Draftify::chargerTableClients()
    {
        // Explicitly select columns to match Client::afficher layout
        QSqlQuery q("SELECT ID_CL, NOM, PRENOM, PHONE, EMAIL, TYPE, FOLLOWERS, SOCIAL, PHOTO_CL, RATING "
                    "FROM CLIENT ORDER BY NOM");

        if (!q.exec()) {
            qDebug() << "Erreur chargement tableWidgetcl:" << q.lastError().text();
            return;
        }

        ui->tableWidgetcl->setRowCount(0);
        ui->tableWidgetcl->setColumnCount(10); // Ensure 10 columns
        ui->tableWidgetcl->setHorizontalHeaderLabels(
            {"ID", "Nom", "Prénom", "Téléphone", "Email", "Type", "Followers", "Social", "Photo", "Rating"});

        int row = 0;

        while (q.next()) {
            ui->tableWidgetcl->insertRow(row);

            int idClient = q.value(0).toInt(); // ID_CL

            // Colonne 0 : ID (display) + UserRole
            QTableWidgetItem *idItem = new QTableWidgetItem(q.value(0).toString());
            idItem->setData(Qt::UserRole, idClient);
            ui->tableWidgetcl->setItem(row, 0, idItem);

            ui->tableWidgetcl->setItem(row, 1, new QTableWidgetItem(q.value(1).toString())); // NOM
            ui->tableWidgetcl->setItem(row, 2, new QTableWidgetItem(q.value(2).toString())); // PRENOM
            ui->tableWidgetcl->setItem(row, 3, new QTableWidgetItem(q.value(3).toString())); // PHONE
            ui->tableWidgetcl->setItem(row, 4, new QTableWidgetItem(q.value(4).toString())); // EMAIL
            ui->tableWidgetcl->setItem(row, 5, new QTableWidgetItem(q.value(5).toString())); // TYPE
            ui->tableWidgetcl->setItem(row, 6, new QTableWidgetItem(q.value(6).toString())); // FOLLOWERS
            ui->tableWidgetcl->setItem(row, 7, new QTableWidgetItem(q.value(7).toString())); // SOCIAL
            ui->tableWidgetcl->setItem(row, 8, new QTableWidgetItem(q.value(8).toString())); // PHOTO_CL

            // Colonne 9 : rating
            QTableWidgetItem *ratingItem = new QTableWidgetItem();
            if (!q.value(9).isNull()) {
                int rating = q.value(9).toInt();
                if (rating > 0)
                    ratingItem->setText(QString::number(rating));
                else
                    ratingItem->setText("0");
            } else {
                ratingItem->setText("0");
            }
            ratingItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidgetcl->setItem(row, 9, ratingItem);

            row++;
        }
    }


    void Draftify::on_Button_supprimer_cli_clicked()
    {
        on_supp_tab_clicked();
    }
    void Draftify::loadAllClients()
    {
        chargerClientsDansTable();
    }

    void Draftify::on_lineEdit_19_textChanged(const QString &text)
    {
        QString texte = text.trimmed();

        // 👉 Si le champ est vide → on recharge la liste normale
        if (texte.isEmpty()) {
            chargerClientsDansTable();
            return;
        }

        // Sinon, tu peux mettre ton code de recherche ici si tu veux,
        // ou juste laisser comme ça si la recherche ne se fait qu'au clic du bouton.
    }
    void Draftify::on_enreg_tab_clicked()
    {
        QString html = "<style>"
                       "body {"
                       "  font-family: 'Segoe UI', sans-serif;"
                       "  background: #e7f0ff;"
                       "  color: #00306e;"
                       "}"
                       "h1 {"
                       "  text-align: center;"
                       "  color: #003a80;"
                       "  border-bottom: 2px solid #0066cc;"
                       "  padding-bottom: 6px;"
                       "}"
                       "table {"
                       "  border-collapse: collapse;"
                       "  width: 100%;"
                       "  background: white;"
                       "  border: 1px solid #99c2ff;"
                       "}"
                       "th {"
                       "  background-color: #0066cc;"
                       "  color: white;"
                       "  padding: 8px;"
                       "  font-size: 12px;"
                       "  border: 1px solid #004c99;"
                       "}"
                       "td {"
                       "  padding: 8px;"
                       "  border: 1px solid #b3d1ff;"
                       "  font-size: 11px;"
                       "}"
                       "tr:nth-child(even) td {"
                       "  background-color: #e6f0ff;"
                       "}"
                       "tr:nth-child(odd) td {"
                       "  background-color: #f7faff;"
                       "}"
                       "</style>";

        html += "<h1>Liste des Clients</h1><br><table>";

        // En-têtes
        html += "<tr>";
        for (int col = 0; col < ui->tableWidgetcl->columnCount(); ++col) {
            QTableWidgetItem *headerItem = ui->tableWidgetcl->horizontalHeaderItem(col);
            html += "<th>" + (headerItem ? headerItem->text() : "") + "</th>";
        }
        html += "</tr>";

        // Lignes
        for (int row = 0; row < ui->tableWidgetcl->rowCount(); ++row) {
            html += "<tr>";
            for (int col = 0; col < ui->tableWidgetcl->columnCount(); ++col) {
                QTableWidgetItem *item = ui->tableWidgetcl->item(row, col);
                html += "<td>" + (item ? item->text() : "") + "</td>";
            }
            html += "</tr>";
        }

        html += "</table></body></html>";

        // Choix du fichier de sortie
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Enregistrer la liste des clients en PDF",
            "",
            "Fichiers PDF (*.pdf)");

        if (fileName.isEmpty())
            return;

        QTextDocument doc;
        doc.setHtml(html);

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);

        doc.print(&printer);

        QMessageBox::information(this, "Succès",
                                 "Liste des clients exportée avec succès !\nFichier : "
                                     + fileName);
    }

    void Draftify::trierClientFollowers(int index)
    {
        QString queryStr;

        // index 0 → croissant, index 1 → décroissant
        if (index == 0) {
            queryStr =
                "SELECT ID_CL, NOM, PRENOM, PHONE, EMAIL, TYPE, FOLLOWERS, SOCIAL, PHOTO_CL, RATING "
                "FROM CLIENT ORDER BY FOLLOWERS ASC";
        } else if (index == 1) {
            queryStr =
                "SELECT ID_CL, NOM, PRENOM, PHONE, EMAIL, TYPE, FOLLOWERS, SOCIAL, PHOTO_CL, RATING "
                "FROM CLIENT ORDER BY FOLLOWERS DESC";
        } else {
            // autre choix → on recharge la liste normale
            chargerClientsDansTable();
            return;
        }

        QSqlQuery q;
        if (!q.exec(queryStr)) {
            QMessageBox::critical(this, "Erreur",
                                  "Erreur lors du tri des clients : "
                                      + q.lastError().text());
            return;
        }

        ui->tableWidgetcl->setRowCount(0);
        ui->tableWidgetcl->setColumnCount(10); // Ensure 10 columns
        ui->tableWidgetcl->setHorizontalHeaderLabels(
            {"ID", "Nom", "Prénom", "Téléphone", "Email", "Type", "Followers", "Social", "Photo", "Rating"});

        int row = 0;
        while (q.next()) {
            ui->tableWidgetcl->insertRow(row);

            int idClient = q.value(0).toInt(); // ID_CL

            // Colonne 0 : ID + UserRole
            QTableWidgetItem *idItem = new QTableWidgetItem(q.value(0).toString());
            idItem->setData(Qt::UserRole, idClient);
            ui->tableWidgetcl->setItem(row, 0, idItem);

            ui->tableWidgetcl->setItem(row, 1, new QTableWidgetItem(q.value(1).toString())); // NOM
            ui->tableWidgetcl->setItem(row, 2, new QTableWidgetItem(q.value(2).toString())); // PRENOM
            ui->tableWidgetcl->setItem(row, 3, new QTableWidgetItem(q.value(3).toString())); // PHONE
            ui->tableWidgetcl->setItem(row, 4, new QTableWidgetItem(q.value(4).toString())); // EMAIL
            ui->tableWidgetcl->setItem(row, 5, new QTableWidgetItem(q.value(5).toString())); // TYPE
            ui->tableWidgetcl->setItem(row, 6, new QTableWidgetItem(q.value(6).toString())); // FOLLOWERS
            ui->tableWidgetcl->setItem(row, 7, new QTableWidgetItem(q.value(7).toString())); // SOCIAL
            ui->tableWidgetcl->setItem(row, 8, new QTableWidgetItem(q.value(8).toString())); // PHOTO_CL

            // Colonne 9 : rating
            QTableWidgetItem *ratingItem = new QTableWidgetItem();
            if (!q.value(9).isNull()) {
                int rating = q.value(9).toInt();
                if (rating > 0)
                    ratingItem->setText(QString::number(rating));
                else
                    ratingItem->setText("0");
            } else {
                ratingItem->setText("0");
            }
            ratingItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidgetcl->setItem(row, 9, ratingItem);

            row++;
        }
    }
    void Draftify::on_Button_sub_tri_clicked()
    {
        int index = ui->combo_tri->currentIndex();
        trierClientFollowers(index);
    }

    void Draftify::statParTypeClients()
    {
        QMap<QString,int> counts;

        QSqlQuery q;

        // On ne garde que human / societe
        QString sql =
            "SELECT TYPE, COUNT(*) "
            "FROM CLIENT "
            "WHERE LOWER(TYPE) IN ('human', 'societe') "
            "GROUP BY TYPE";

        if (!q.exec(sql)) {
            QString err = q.lastError().text();
            qDebug() << "Erreur requête stat clients par type :" << err;
            ui->labelstat->setText("Erreur SQL : " + err);
            ui->labelstat->setAlignment(Qt::AlignCenter);
            return;
        }

        while (q.next()) {
            QString rawType = q.value(0).toString();
            int cnt = q.value(1).toInt();

            // Normalisation de l'étiquette à afficher
            QString type;
            if (rawType.compare("human", Qt::CaseInsensitive) == 0)
                type = "Human";
            else if (rawType.compare("societe", Qt::CaseInsensitive) == 0
                     || rawType.compare("société", Qt::CaseInsensitive) == 0)
                type = "Société";
            else
                type = rawType;   // au cas où

            counts[type] += cnt;
        }

        if (counts.isEmpty()) {
            ui->labelstat->setText("Aucune donnée");
            ui->labelstat->setAlignment(Qt::AlignCenter);
            return;
        }

        QPixmap pie = drawPieChartPixmap(counts, QSize(600, 400));
        ui->labelstat->setPixmap(pie);
        ui->labelstat->setAlignment(Qt::AlignCenter);
        ui->labelstat->setScaledContents(true);
    }

    QPixmap Draftify::drawPieChartPixmap(const QMap<QString,int>& counts, const QSize &size)
    {
        QPixmap pix(size);
        pix.fill(Qt::white);

        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing);

        // Compute total
        int total = 0;
        for (auto v : counts) total += v;

        const int margin = 12;
        double legendWidth = qMin(320.0, double(size.width()) * 0.33);
        double availWidth = double(size.width()) - legendWidth - 3.0 * margin;
        double maxPieWidth = qMin(availWidth, double(size.height()) - 2*margin);
        if (maxPieWidth < 10) maxPieWidth = qMax(10.0, double(size.height()) - 2*margin);
        double pieX = margin + (availWidth - maxPieWidth) / 2.0;
        QRectF pieRect(pieX, (size.height() - maxPieWidth) / 2.0, maxPieWidth, maxPieWidth);

        if (total <= 0) {
            painter.drawText(pieRect, Qt::AlignCenter, "Aucune donnée");
            return pix;
        }

        QVector<QColor> colors = {
            QColor(138, 43, 226),  // Mauve / Violet
            QColor(30, 144, 255)   // Bleu
        };

        int startAngle16 = 0;
        int colorIndex = 0;

        // Tranches
        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
            int value = it.value();
            int span16 = qRound(360.0 * 16 * (double(value) / double(total)));
            QColor c = colors[colorIndex % colors.size()];
            painter.setBrush(c);
            painter.setPen(Qt::NoPen);
            painter.drawPie(pieRect, startAngle16, span16);
            startAngle16 += span16;
            colorIndex++;
        }

        // Légende
        int legendX = int(size.width() - legendWidth + margin/2.0);
        int legendY = 10;
        int sw = 12;
        colorIndex = 0;
        painter.setPen(Qt::black);
        QFont f = painter.font(); f.setPointSize(8); painter.setFont(f);

        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
            QColor c = colors[colorIndex % colors.size()];
            painter.setBrush(c);
            painter.setPen(Qt::NoPen);
            painter.drawRect(legendX, legendY, sw, sw);
            painter.setPen(Qt::black);

            double percent = 0.0;
            if (total > 0) percent = (double(it.value()) * 100.0) / double(total);
            QString percentStr = QString::number(percent, 'f', 1);
            QString label = QString("%1 (%2%)").arg(it.key()).arg(percentStr);
            painter.drawText(legendX + sw + 6, legendY + sw - 1, label);
            legendY += sw + 8;
            colorIndex++;
        }

        return pix;
    }
    void Draftify::on_btn_stat_clicked()
    {
        // 1. Générer la stat par type (human / societe)
        statParTypeClients();

        // 2. Aller à la page des stats (si tu utilises un QStackedWidget)
        ui->stackedWidget_client->setCurrentWidget(ui->clstat);
        // ou: ui->stackedWidget->setCurrentIndex(1);
    }

    void Draftify::on_retournerstat_clicked()
    {
        ui->stackedWidget_client->setCurrentWidget(ui->page_5);
    }

    void Draftify::on_btn_rating_clicked()
    {
        // 1. Ligne sélectionnée dans le tableau
        int row = ui->tableWidgetcl->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Rating",
                                 "Veuillez d'abord sélectionner un client dans le tableau.");
            return;
        }

        // 2. Récupérer le texte du combobox (★☆☆☆☆, ★★★☆☆, …)
        QString starsText = ui->rating->currentText();
        int ratingValue = starsText.count(QChar(u'★'));   // nombre d'étoiles pleines

        if (ratingValue <= 0) {
            QMessageBox::warning(this, "Rating",
                                 "Veuillez choisir un nombre d'étoiles valide.");
            return;
        }

        // 3. Récupérer l'ID du client stocké dans la colonne 0 (UserRole)
        QTableWidgetItem *nameItem = ui->tableWidgetcl->item(row, 0);
        if (!nameItem) {
            QMessageBox::warning(this, "Rating",
                                 "Impossible de récupérer le client sélectionné.");
            return;
        }
        int idClient = nameItem->data(Qt::UserRole).toInt();
        if (idClient == 0) {
            idClient = nameItem->text().toInt();
        }
        qDebug() << "Rating pour client id =" << idClient << " -> " << ratingValue;

        // 4. Met à jour la cellule "rating" dans le tableau (dernière colonne = Rating)
        int ratingColumn = 9; // Index 9 is Rating (0-based)
        QTableWidgetItem *ratingItem = ui->tableWidgetcl->item(row, ratingColumn);
        if (!ratingItem) {
            ratingItem = new QTableWidgetItem();
            ui->tableWidgetcl->setItem(row, ratingColumn, ratingItem);
        }
        ratingItem->setText(QString::number(ratingValue));
        ratingItem->setTextAlignment(Qt::AlignCenter);

        // 5. Sauvegarder dans la base
        QSqlQuery q;
        q.prepare("UPDATE CLIENT SET RATING = :rating WHERE ID_CL = :id");
        q.bindValue(":rating", ratingValue);
        q.bindValue(":id", idClient);

        if (!q.exec()) {
            QMessageBox::warning(this, "Erreur BD",
                                 "Erreur lors de la sauvegarde du rating :\n"
                                     + q.lastError().text());
            qDebug() << "SQL error rating:" << q.lastError().text();
        }
    }




    //arduino nash

    // draftify.cpp

     void Draftify::reloadClientTable()
    {
        // Get fresh data from DB
        QSqlQueryModel *model = clientManager.afficher();

        // Clear existing rows/columns
        ui->tableWidgetcl->clear();
        ui->tableWidgetcl->setRowCount(model->rowCount());
        ui->tableWidgetcl->setColumnCount(model->columnCount());

        // Set headers
        for (int col = 0; col < model->columnCount(); ++col) {
            ui->tableWidgetcl->setHorizontalHeaderItem(
                col,
                new QTableWidgetItem(model->headerData(col, Qt::Horizontal).toString()));
        }

        // Copy data from model to tableWidgetcl
        for (int row = 0; row < model->rowCount(); ++row) {
            for (int col = 0; col < model->columnCount(); ++col) {
                QString text = model->data(model->index(row, col)).toString();
                ui->tableWidgetcl->setItem(row, col, new QTableWidgetItem(text));
            }
        }

        delete model;  // avoid leak
    }







     void Draftify::on_importcl_clicked()
     {
         QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
         if (!fileName.isEmpty()) {
             selectedPhotoPath = fileName;
             ui->photocl->setText(fileName);
             QMessageBox::information(this, "Image Selected", "Image selected successfully: " + fileName);
         }
     }

