// clientidialog.h
#ifndef CLIENTIDIALOG_H
#define CLIENTIDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ClientIDialog;
}

class ClientIDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientIDialog(QWidget *parent = nullptr);
    ~ClientIDialog();

    void setId(const QString &id);  // show full ID from Arduino
    QString id() const;

    void showInsertMsg();           // "Insert client ID..."
    void showInvalidMsg();          // "Invalid ID..."
    void showAskRatingMsg();        // "ID valid, give rating..."

private:
    Ui::ClientIDialog *ui;
};

#endif // CLIENTIDIALOG_H
