#ifndef CONNECTION_H
#define CONNECTION_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>


class Connection
{
public:
    static Connection& createInstance();
    bool createconnect();

private:
      QSqlDatabase db;

      //methodes prives
      Connection();
      ~Connection();

      //interdir la copie et l'affectation garantir l'unicité
      Connection(const Connection&) = delete;
      Connection& operator=(const Connection&) = delete;
};

#endif // CONNECTION_H
