// clientidialog.cpp
#include "clientidialog.h"
#include "ui_clientidialog.h"

ClientIDialog::ClientIDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientIDialog)
{
    ui->setupUi(this);
    setWindowTitle("Client ID");
    ui->lineEditId->setReadOnly(true);   // ID comes only from Arduino
    showInsertMsg();
}

ClientIDialog::~ClientIDialog()
{
    delete ui;
}

void ClientIDialog::setId(const QString &id)
{
    ui->lineEditId->setText(id);
}

QString ClientIDialog::id() const
{
    return ui->lineEditId->text();
}

void ClientIDialog::showInsertMsg()
{
    ui->labelInfo->setText("Insert client ID with keypad, press D to validate.");
}

void ClientIDialog::showInvalidMsg()
{
    ui->labelInfo->setText("Invalid ID. Please try again.");
}

void ClientIDialog::showAskRatingMsg()
{
    ui->labelInfo->setText("ID valid. Give rating (1–5).");
}
