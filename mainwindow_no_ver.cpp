#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qmessagebox.h"

MainWindow::MainWindow(QWidget *parent, QTcpSocket *socket, QString username) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    this->username = username;
    this->socket = socket;

    socket->setReadBufferSize(1024);

    connect(socket, &QTcpSocket::connected, this, &MainWindow::isConnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readAnswer);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::connError);

    ui->setupUi(this);

    ui->listView->setModel(playersList);

    QStringList players = {"player1"};

    QStandardItem *item = new QStandardItem(QString(username));
    item->setEditable(false);
    playersList->appendRow(item);

    ui->webEngineView->setHtml("<html><body>Waiting for players...</body></html>");


    connect(ui->webEngineView, &QWebEngineView::urlChanged, [this](const QUrl &url) {
        QString newUrl = url.toString();
        if(!newUrl.startsWith("data:") && newUrl.startsWith("https://en.wikipedia.org/wiki/"))
        {
            MainWindow::sendData(newUrl);
            qDebug() << "New URL:" << newUrl;
        }
    });


    ui->gridLayout->addWidget(ui->webEngineView, 1, 0, 2, 3);
    ui->gridLayout->setColumnStretch(0, 1);
    ui->gridLayout->setColumnStretch(1, 0);
    ui->gridLayout->setColumnStretch(2, 0);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::isConnected(){
    qDebug() << "Connected";
}

void MainWindow::readAnswer(){
    QString answear = socket->readAll();
    answear.remove('\u0000');
    qDebug() << answear;
    if(answear.startsWith("Joined:")){
        qDebug() << answear.sliced(7);
        QStandardItem *player = new QStandardItem(answear.sliced(7));
        player->setEditable(false);
        playersList->appendRow(player);
    }
    else if(answear.startsWith("Quited:")){
        qDebug() << answear.sliced(7, answear.length()-7);
        QString player = answear.sliced(7, answear.length()-7);
        if(playersList->findItems(player)[0])
        {
            playersList->removeRow(playersList->findItems(player)[0]->row());
        }
    }
    else if(answear.startsWith("Start:")){
        answear = answear.sliced(6);
        qDebug() << answear;
        this->currentUrl = answear;
        ui->webEngineView->setUrl(QUrl(answear));
        qDebug() << QString(ui->webEngineView->url().toString());
    }
    else if(answear.startsWith("Dest:")){
        answear = answear.sliced(35);
        answear = answear.replace("_", " ").toLocal8Bit();
        qDebug() << answear;
        ui->destination->setText("Destination: " + answear);
    }
    else if(answear.startsWith("Win:")){
        QString name = answear.sliced(4);
        QString path = name.sliced(name.indexOf(" ") + 1);
        name = name.sliced(0, name.indexOf(" "));
        qDebug() << name;
        path = path.sliced(0, path.length()-1);
        path = path.replace(" ", " -> ");
        path = path.replace("_", " ");
        qDebug() << path;
        QMessageBox::information(this, "End!", "User " + name + " has won the game!\n" + "Victorious path: " + path);
        socket->close();
        this->close();
    }

}
void MainWindow::connError(){
    QString err = ("Error or disconnect on socket: "+socket->errorString());
    qDebug() << err;
    QMessageBox::warning(this, "Connection error!", "Connection with the server lost!");
    socket->close();
    this->close();
}

void MainWindow::sendData(QString data){
    qDebug() << data;
    socket->write(qUtf8Printable(data));
}

