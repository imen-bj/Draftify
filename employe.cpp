#include "employe.h"
#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <QVariant>
#include <QRegularExpression>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
Employe::Employe() {

}
Employe::Employe(int id ,QString n ,QString pn ,QString m ,QString p ,QString adr,int c,int num , int sl,QString rl  ){
    ID_EMPL = id;
    NOM = n;
    PRENOM = pn;
    MAIL = m;
    POSTE = p;
    CIN = c;
    NUM_TEL = num;
    ADRESSE = adr;
    SALARY = sl;
    ROLE = rl;
}

QSqlQueryModel * Employe::afficher()
{
    // Créer une instance du modèle QSqlQueryModel
    QSqlQueryModel *model = new QSqlQueryModel();

    // Requête SQL pour récupérer les données des employés
    QSqlQuery query;
    query.exec("SELECT ID_EMPL, NOM, PRENOM, MAIL, POSTE, CIN, NUM_TEL,ADRESSE, SALARY , ROLE FROM employe");

    // Associer les résultats de la requête au modèle
    model->setQuery("select ID_EMPL, NOM, PRENOM, MAIL, POSTE, CIN, NUM_TEL,ADRESSE, SALARY , ROLE from employe");

    // Définir les en-têtes pour chaque colonne (nom des attributs de la table)
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Family Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Post"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Phone"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Adress"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Salary"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Role"));


    // Retourner le modèle avec les données
    return model;
}

bool Employe::ajouter()
{
    QSqlQuery query;
    QString id_str = QString::number(ID_EMPL);      // Convertir ID_EMPL en QString
    QString cin_str = QString::number(CIN);         // Convertir CIN en QString
    QString num_tel_str = QString::number(NUM_TEL); // Convertir NUM_TEL en QString
    QString salary_str = QString::number(SALARY); // Convertir Salary en QString

    // Préparer la requête avec des placeholders pour les valeurs
    query.prepare("INSERT INTO employe (ID_EMPL, NOM, PRENOM, MAIL, POSTE, CIN, NUM_TEL, ADRESSE , SALARY , ROLE) "
                  "VALUES (:id, :nom, :prenom, :mail, :poste, :cin, :num_tel, :adresse , :salary , :role)");

    // Lier les valeurs aux placeholders
    query.bindValue(":id", id_str);           // Lier ID_EMPL converti en QString
    query.bindValue(":nom", NOM);
    query.bindValue(":prenom", PRENOM);
    query.bindValue(":mail", MAIL);
    query.bindValue(":poste", POSTE);
    query.bindValue(":cin", cin_str);         // Lier CIN converti en QString
    query.bindValue(":num_tel", num_tel_str); // Lier NUM_TEL converti en QString
    query.bindValue(":adresse", ADRESSE);   // Lier POINTS converti en QString
    query.bindValue(":salary", salary_str); // Lier NUM_TEL converti en QString
    query.bindValue(":role", ROLE);
    // Exécuter la requête
    return query.exec();
}


QSqlQueryModel* Employe::rechercher(const QString& critere)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QString queryStr = QString("SELECT * FROM employe WHERE NOM LIKE '%%1%' OR PRENOM LIKE '%%1%' OR ID_EMPL LIKE '%%1%' OR "
                               "MAIL LIKE '%%1%' OR POSTE LIKE '%%1%' OR CIN LIKE '%%1%' OR NUM_TEL LIKE '%%1%' OR "
                               "ADRESSE LIKE '%%1%' OR  SALARY LIKE '%%1%' OR  ROLE LIKE '%%1%'").arg(critere);

    model->setQuery(queryStr);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Family Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Position"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Phone"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Address"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Salary"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Role"));

    return model;
}

bool Employe::supprimer(int id){
    QSqlQuery query;
    QString id_str = QString::number(id);      // Convertir ID_EMPL en QString
    query.prepare("DELETE FROM EMPLOYE WHERE ID_EMPL = :id");
    query.bindValue(":id", id_str);
    return query.exec();
}

//sort_______
QSqlQueryModel* Employe::trier(const QString& critere)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QString queryStr;

    if (critere == "post") {
        queryStr = "SELECT ID_EMPL, NOM, PRENOM, MAIL, POSTE, CIN, NUM_TEL, ADRESSE, SALARY , ROLE "
                   "FROM employe ORDER BY POSTE ASC";
    }
    else if (critere == "nom") {
        queryStr = "SELECT ID_EMPL, NOM, PRENOM, MAIL, POSTE, CIN, NUM_TEL, ADRESSE, SALARY , ROLE "
                   "FROM employe ORDER BY NOM ASC";
    }
    else {
        queryStr = "SELECT ID_EMPL, NOM, PRENOM, MAIL, POSTE, CIN, NUM_TEL, ADRESSE, SALARY , ROLE "
                   "FROM employe";
    }

    model->setQuery(queryStr);

    // Optional: headers for better readability
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Family Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Post"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Phone"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Adress"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Salary"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Role"));


    return model;
}

//export_____________________________________________________________
static QString csvEscape(QString s, QChar sep)
{
    // Double quotes inside fields must be doubled (" → "")
    s.replace("\"", "\"\"");

    // If field contains separator, quote, or newline → wrap in quotes
    if (s.contains(sep) || s.contains("\"") || s.contains("\n"))
    {
        s = "\"" + s + "\"";
    }

    return s;
}
bool Employe::exportCsvFromModel(const QAbstractItemModel* model,
                                 const QString& filePath,
                                 QChar sep)
{
    if (!model) return false;  // Ensure model is valid

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))  // Open file for writing
        return false;  // Return false if file can't be opened


    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << u"\uFEFF"_qs; // UTF-8 BOM so Excel keeps accents correctly

    const int rows = model->rowCount();  // Get the number of rows
    const int cols = model->columnCount();  // Get the number of columns

    // Write the header row
    for (int c = 0; c < cols; ++c) {
        QString header = model->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString();
        if (c) out << sep;  // Add separator if it's not the first column
        out << csvEscape(header, sep);  // Escape header text for CSV format
    }
    out << '\n';  // Newline after header row

    // Write data rows
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (c) out << sep;  // Add separator if it's not the first column
            QString cell = model->data(model->index(r, c), Qt::DisplayRole).toString();

            // Handle large numbers (e.g., CIN or phone) to be treated as text in Excel
            // Remove spaces, plus signs, minus signs, and dots
            cell.remove(QChar(' '));  // Remove spaces
            cell.remove(QChar('+'));   // Remove plus signs
            cell.remove(QChar('-'));   // Remove minus signs
            cell.remove(QChar('.'));   // Remove dots

            // Now remove 'E' and 'e' (case-insensitive)
            cell.remove(QChar('E'));   // Remove uppercase 'E'
            cell.remove(QChar('e'));   // Remove lowercase 'e'

            // If the cell size is large (like a phone number), prepend a quote for Excel to treat as text
            if (cell.size() >= 10) {
                cell.prepend(u"'"); // Add a single quote before the number for Excel to treat as text
            }
            out << csvEscape(cell, sep);  // Escape cell text for CSV format
        }
        out << '\n';  // Newline after each data row
    }

    out.flush();  // Ensure data is written to file
    f.close();  // Close the file
    return true;  // Return true to indicate success
}
//stats________________________________________________________________

void Employe::getSalaireParPoste(QStringList &posts, QVector<int> &totals)
{
    posts.clear();
    totals.clear();

    QSqlQuery q;
    q.prepare(
        "SELECT POSTE, SUM(SALARY) AS total_salary "
        "FROM employe "
        "GROUP BY POSTE "
        "ORDER BY total_salary DESC"
        );

    if (!q.exec()) {
        qDebug() << "Erreur SQL getSalaireParPoste:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        QString poste    = q.value(0).toString();
        int totalSalaire = q.value(1).toInt();

        posts.append(poste);
        totals.append(totalSalaire);
    }
}


//______signUp__________
// Check if email already exists in the database
bool Employe::isEmailExist(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT MAIL FROM EMPLOYE WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.exec();

    return query.next();  // Returns true if the email exists, false if it doesn't
}

// Validate the password strength
bool Employe::validatePassword(const QString &password) {
    QRegularExpression regex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,}$");
    return regex.match(password).hasMatch();  // Check if the password matches the regex pattern
}

// Save the security question and answer for the email
bool Employe::saveSecurityQuestion(const QString &email, const QString &question, const QString &answer) {
    QSqlQuery query;
    query.prepare("UPDATE EMPLOYE SET QUESTION = :question, ANSWER = :answer WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.bindValue(":question", question);
    query.bindValue(":answer", answer);

    return query.exec();  // Execute the query to update the security question and answer
}
bool Employe::savePassword(const QString &email, const QString &password) {
    // Hash the password before saving (optional, for security reasons)
    QString Password = password;

    // Prepare the SQL query to update the password
    QSqlQuery query;
    query.prepare("UPDATE EMPLOYE SET PASSWORD = :password WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.bindValue(":password", Password);

    // Execute the query and return whether it succeeded or not
    return query.exec();
}
bool Employe::isPasswordExist(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT PASSWORD FROM employe WHERE EMAIL = :email");
    query.bindValue(":email", email);
    query.exec();

    if (query.next()) {
        QString password = query.value(0).toString();
        return !password.isEmpty();  // If password exists, return true; otherwise false
    }
    return false;  // If no record found
}

//____login______
QString Employe::getPasswordByEmail(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT PASSWORD FROM EMPLOYE WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.exec();

    if (query.next()) {
        return query.value(0).toString();  // Return the password
    }
    return "";  // Return empty if the email doesn't exist
}
QString Employe::getRoleByEmail(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT ROLE FROM EMPLOYE WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.exec();

    if (query.next()) {
        return query.value(0).toString();  // Return the role
    }
    return "";  // Return empty if the email doesn't exist
}

//forgot password_________________
// Function to retrieve the security question for the given email
QString Employe::getSecurityQuestion(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT QUESTION FROM EMPLOYE WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.exec();

    if (query.next()) {
        return query.value(0).toString();
    }
    return "";  // Return empty if no question is found
}

// Function to validate the selected security question
bool Employe::validateSecurityQuestion(const QString &email, const QString &question) {
    QSqlQuery query;
    query.prepare("SELECT QUESTION FROM EMPLOYE WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.exec();

    if (query.next()) {
        return query.value(0).toString() == question;
    }
    return false;
}
// Function to validate the answer to the security question
bool Employe::validateSecurityAnswer(const QString &email, const QString &answer) {
    QSqlQuery query;
    query.prepare("SELECT ANSWER FROM EMPLOYE WHERE MAIL = :email");
    query.bindValue(":email", email);
    query.exec();

    if (query.next()) {
        return query.value(0).toString() == answer;
    }
    return false;
}

//update password
bool Employe::updatePassword(const QString& email, const QString& newPassword)
{
    // Prepare the SQL query to update the password based on the email
    QSqlQuery query;
    query.prepare("UPDATE EMPLOYE SET PASSWORD = :newPassword WHERE MAIL = :email");
    query.bindValue(":newPassword", newPassword);
    query.bindValue(":email", email);

    // Execute the query and return whether it was successful
    if (query.exec()) {
        return true;
    } else {
        qDebug() << "Error updating password: " << query.lastError().text();
        return false;
    }
}

