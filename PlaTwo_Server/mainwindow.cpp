#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    server = new QTcpServer(this);

    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    if (server->listen(QHostAddress::Any, 8080)) {
        qDebug() << "Server started on port 8080.";
    } else {
        qDebug() << "Server failed to start:" << server->errorString();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewConnection()
{

    QTcpSocket *clientSocket = server->nextPendingConnection();
    clients.append(clientSocket);

    qDebug() << "A player connected! Total players:" << clients.size();

    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
}

void MainWindow::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;


    QByteArray data = clientSocket->readAll();
    QString message = QString::fromUtf8(data);

    qDebug() << "Received from client:" << message;
}