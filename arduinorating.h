// arduinorating.h
#ifndef ARDUINORATING_H
#define ARDUINORATING_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class ClientIDialog;

class ArduinoRating : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoRating(QObject *parent = nullptr);

    // Call once from Draftify to set UI + open port
    void init(ClientIDialog *dialog);
    void setPortName(const QString &name);

signals:
    void ratingSaved(int clientId, int rating);   // to refresh table
    void invalidId(const QString &id);            // if you want extra handling

private slots:
    void readRatingArduino();                     // connected to QSerialPort::readyRead

private:
    void validateClientId(const QString &id);
    void applyRating(const QString &id, int rating);
    void reloadClientTable();                    // or emit a signal instead

    QSerialPort *m_port = nullptr;
    ClientIDialog *m_dialog = nullptr;

    QString m_currentClientId;
    bool m_waitingForRating = false;
    QByteArray m_buffer;
};

#endif // ARDUINORATING_H
