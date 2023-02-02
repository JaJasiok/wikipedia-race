#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QtNetwork>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, QTcpSocket *socket = nullptr, QString username = "");
    ~MainWindow();

private slots:
    void isConnected();
    void sendData(QString);
    void readAnswer();
    void connError();

protected:
    QTcpSocket *socket;

private:
    Ui::MainWindow *ui;

    QString username;
    QString currentUrl;

    QStandardItemModel *playersList = new QStandardItemModel(this);
};
#endif // MAINWINDOW_H
