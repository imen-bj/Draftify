// arduinorating.cpp
#include "arduinorating.h"
#include "clientidialog.h"
#include "qsqlerror.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ArduinoRating::ArduinoRating(QObject *parent)
    : QObject(parent)
{
    m_port = new QSerialPort(this);
}

void ArduinoRating::setPortName(const QString &name)
{
    m_port->setPortName(name);
}

void ArduinoRating::init(ClientIDialog *dialog)
{
    m_dialog = dialog;

    // serial to the keypad+buttons Arduino
    m_port->setBaudRate(QSerialPort::Baud9600);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (m_port->open(QIODevice::ReadOnly)) {
        qDebug() << "Opened Arduino on" << m_port->portName();
        connect(m_port, &QSerialPort::readyRead,
                this, &ArduinoRating::readRatingArduino);
    } else {
        qDebug() << "Cannot open rating Arduino:" << m_port->errorString();
    }
}


void ArduinoRating::readRatingArduino()
{
    if (!m_port)
        return;

    // Append all newly available bytes to the buffer
    m_buffer.append(m_port->readAll());

    // Process complete lines separated by '\n'
    int newlineIndex;
    while ((newlineIndex = m_buffer.indexOf('\n')) != -1) {

        QByteArray line = m_buffer.left(newlineIndex).trimmed();
        m_buffer.remove(0, newlineIndex + 1); // remove processed part

        if (line.isEmpty())
            continue;

        qDebug() << "RAW FROM ARDUINO (line):" << line;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "JSON error:" << err.errorString();
            continue;
        }

        QJsonObject obj = doc.object();
        const QString type = obj.value("type").toString();

        // SAFETY: make sure dialog exists
        if (!m_dialog)
            return;              // or create one elsewhere and always pass it

        // STATUS: A / C
        if (type == "status") {
            const QString msg = obj.value("message").toString();

            if (msg == "ID mode started") {
                m_currentClientId.clear();
                m_waitingForRating = false;
                m_dialog->showInsertMsg();
                m_dialog->setId("");
                m_dialog->show();
                m_dialog->raise();
                m_dialog->activateWindow();
            }
            else if (msg == "ID cancelled") {
                // C pressed: just clear and go back to normal, no error message
                m_currentClientId.clear();
                m_waitingForRating = false;
                m_dialog->showInsertMsg();
                m_dialog->setId("");
                m_dialog->show();
                m_dialog->raise();
                m_dialog->activateWindow();
            }
        }

        // LIVE TYPING (digits, B, *)
        else if (type == "id_update") {
            QString id = obj.value("id").toString();

            // Only update while entering ID (not after validation)
            if (m_waitingForRating)
                continue;

            m_currentClientId = id;

            m_dialog->showInsertMsg();
            m_dialog->setId(id);
            m_dialog->show();
            m_dialog->raise();
            m_dialog->activateWindow();
        }

        // D / # : VALIDATE
        else if (type == "id_submit") {
            QString id = obj.value("id").toString();
            m_currentClientId = id;

            // validateClientId will decide if we wait for rating or not
            m_waitingForRating = false;

            m_dialog->showInsertMsg();
            m_dialog->setId(id);
            m_dialog->show();
            m_dialog->raise();
            m_dialog->activateWindow();

            validateClientId(id);
        }

        // RATING BUTTONS 1–5
        else if (type == "button") {
            int rating = obj.value("value").toInt();
            if (m_waitingForRating && !m_currentClientId.isEmpty()
                && rating >= 1 && rating <= 5) {
                applyRating(m_currentClientId, rating);
            }
        }
    }
}

void ArduinoRating::validateClientId(const QString &id)
{
    QSqlQuery query;
    query.prepare("SELECT NOM, PRENOM FROM CLIENT WHERE ID_CL = :id");
    query.bindValue(":id", id.toInt());

    if (!query.exec()) {
        // DB error treated like invalid ID for UI
        m_currentClientId.clear();
        m_waitingForRating = true;           // block id_update so message stays
        if (!m_dialog)
            return;
        m_dialog->setId(id);
        m_dialog->showInvalidMsg();          // "Invalid ID. Please try again."
        m_dialog->show();
        m_dialog->raise();
        m_dialog->activateWindow();
        emit invalidId(id);
        return;
    }

    if (query.next()) {
        // ID exists -> allow rating
        m_currentClientId = id;
        m_waitingForRating = true;           // now rating buttons are accepted
        if (!m_dialog)
            return;
        m_dialog->setId(id);
        m_dialog->showAskRatingMsg();        // "ID valid. Give rating (1–5)."
        m_dialog->show();
        m_dialog->raise();
        m_dialog->activateWindow();
    } else {
        // ID does not exist -> show error, keep dialog visible
        m_currentClientId.clear();
        m_waitingForRating = true;           // block further id_update so text does not vanish
        if (!m_dialog)
            return;
        m_dialog->setId(id);
        m_dialog->showInvalidMsg();          // "Invalid ID. Please try again."
        m_dialog->show();
        m_dialog->raise();
        m_dialog->activateWindow();
        emit invalidId(id);
    }
}

void ArduinoRating::applyRating(const QString &id, int rating)
{
    QSqlQuery q;
    q.prepare("UPDATE CLIENT SET RATING = :r WHERE ID_CL = :id");
    q.bindValue(":r", rating);
    q.bindValue(":id", id.toInt());

    if (!q.exec()) {
        qDebug() << "DB error (update rating):" << q.lastError().text();
        if (m_dialog) {
            m_dialog->showInvalidMsg();   // or a dedicated DB error message
        }
        return;
    }

    if (q.numRowsAffected() > 0) {
        // Rating saved successfully
        m_waitingForRating = false;
        m_currentClientId.clear();

        // Notify Draftify so it can call reloadClientTable()
        emit ratingSaved(id.toInt(), rating);      // ← this is the place

        if (m_dialog) {
            m_dialog->hide();                      // close window after saving rating
        }
    } else {
        if (m_dialog) {
            m_dialog->showInvalidMsg();           // nothing updated
        }
    }
}


