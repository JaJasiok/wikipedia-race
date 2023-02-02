#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include "mainwindow.h"
#include <QtNetwork>
#include <QRegularExpression>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void on_pushButton_Login_clicked();

    void isConnected();
    void sendData(QString);
    void readAnswer();
    void connError();


private:
    Ui::LoginWindow *ui;
    MainWindow *mainWindow;

    QTcpSocket socket;
    QString username;
};

#endif // LOGINWINDOW_H
