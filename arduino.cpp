#include "arduino.h"
#include <QMessageBox>

Arduino::Arduino()
{
    data="";
    arduino_port_name="";
    arduino_is_available=false;
    serial=new QSerialPort;
}

QString Arduino::getarduino_port_name()
{
    return arduino_port_name;
}

QSerialPort *Arduino::getserial()
{
    return serial;
}

int Arduino::connect_arduino()
{
    // recherche du port sur lequel la carte arduino identifée par arduino_uno_vendor_id est connectée
    foreach (const QSerialPortInfo &serial_port_info, QSerialPortInfo::availablePorts()){
        if(serial_port_info.hasVendorIdentifier() && serial_port_info.hasProductIdentifier()){
            if(serial_port_info.vendorIdentifier() == arduino_uno_vendor_id && serial_port_info.productIdentifier() == arduino_uno_producy_id) {
                arduino_is_available = true;
                arduino_port_name=serial_port_info.portName();
            }
        }
    }

    if(arduino_is_available){ // configuration de la communication
        serial->setPortName(arduino_port_name);
        if(serial->open(QSerialPort::ReadWrite)){
            serial->setBaudRate(QSerialPort::Baud9600); // débit : 9600 bits/s
            serial->setDataBits(QSerialPort::Data8); //Longueur des données : 8 bits
            serial->setParity(QSerialPort::NoParity); //1 bit de parité optionnel
            serial->setStopBits(QSerialPort::OneStop); //Nombre de bits de stop : 1
            serial->setFlowControl(QSerialPort::NoFlowControl);

            // Replaced qDebug with QMessageBox
            QMessageBox::information(nullptr, "Connection Status", "Arduino is connected to " + arduino_port_name);
            return 0;
        }
        // Replaced qDebug with QMessageBox for failure
        QMessageBox::critical(nullptr, "Connection Error", "Failed to open serial port.");
        return 1;
    }

    // Replaced qDebug with QMessageBox for failure
    QMessageBox::critical(nullptr, "Connection Error", "Arduino not available.");
    return -1;
}

int Arduino::close_arduino()
{
    if(serial->isOpen()){
        serial->close();
        return 0;
    }
    return 1;
}

QByteArray Arduino::read_from_arduino()
{
    if(serial->isReadable()){
        data=serial->readAll(); //récupérer les données reçues
        return data;
    }
    return QByteArray();
}

int Arduino::write_to_arduino(QByteArray d)
{
    if(serial->isWritable()){
        serial->write(d);  // envoyer des donnés vers Arduino
    } else {
        // Replaced qDebug with QMessageBox
        QMessageBox::critical(nullptr, "Write Error", "Couldn't write to serial!");
    }
}

void Arduino::process_uid(const QString &uid)
{
    qDebug() << "Checking UID in database:" << uid;

    QSqlQuery query;
    query.prepare("SELECT NOM, PRENOM FROM EMPLOYE WHERE ID = :uid");
    query.bindValue(":uid", uid);

    if (query.exec() && query.next()) {
        QString nom = query.value("NOM").toString();
        QString prenom = query.value("PRENOM").toString();

        QMessageBox::information(nullptr, "ACCESS GRANTED !",
                                 "Welcome\n" + prenom + " " + nom + "\n\nDoor opening...");

        write_to_arduino("open\n");
        qDebug() << "ACCESS GRANTED → sent 'open'";
    }
    else {
        qDebug() << "Query failed or no match:" << query.lastError().text();
        QMessageBox::warning(nullptr, "Access Denied", "Card not registered!");
        write_to_arduino("deny\n");
    }
}
