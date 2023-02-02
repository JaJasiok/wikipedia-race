#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    socket.connectToHost("localhost", 12345);

    socket.setReadBufferSize(1024);

    connect(&socket, &QTcpSocket::connected, this, &LoginWindow::isConnected);
    connect(&socket, &QTcpSocket::readyRead, this, &LoginWindow::readAnswer);
    connect(&socket, &QTcpSocket::errorOccurred, this, &LoginWindow::connError);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::isConnected(){
    qDebug() << socket.readBufferSize() << "Connected";
}

void LoginWindow::readAnswer(){
    QString answear = socket.readAll();
//    qDebug() << answear << " login";
    if(answear.startsWith(QString("OK"))){
        disconnect(&socket, &QTcpSocket::readyRead, this, &LoginWindow::readAnswer);
        disconnect(&socket, &QTcpSocket::connected, this, &LoginWindow::isConnected);
        disconnect(&socket, &QTcpSocket::errorOccurred, this, &LoginWindow::connError);
        this->mainWindow = new MainWindow(nullptr, &this->socket, this->username);
        this->mainWindow->show();
        this->close();
    }
    if(answear.startsWith(QString("NO"))){
        QMessageBox::warning(this,"Login", "Username is already taken");
    }
}
void LoginWindow::connError(){
    QString err = "Error or disconnect on socket: "+socket.errorString();
    qDebug() << err << "login";
    QMessageBox::warning(this, "Connection error!", "Connection with the server lost!");
    socket.close();
    this->close();
}

void LoginWindow::sendData(QString data){
    socket.write(qUtf8Printable(data));
}

void LoginWindow::on_pushButton_Login_clicked()
{
    this->username = ui->lineEdit_username->text();

    QRegularExpression expression("^[a-zA-Z0-9_]+$");

    if(username.length() < 6) {
        QMessageBox::warning(this,"Login", "Username must be above 6 characters!");
    }
    else if(!expression.match(username).hasMatch()) {
        QMessageBox::warning(this,"Login", "Username must contain only letters, numbers or underscore character!");
    }
    else if(username.length() > 255){
        QMessageBox::warning(this,"Login", "Username must be under 256 characters!");
    }
    else{
        LoginWindow::sendData(username);
    }
}
