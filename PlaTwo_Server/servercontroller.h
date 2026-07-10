#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QDebug>

class ServerController : public QObject {
    Q_OBJECT
public:
    ServerController() {
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &ServerController::onNewConnection);

        if (server->listen(QHostAddress::Any, 8080)) {
            qDebug() << "============================================";
            qDebug() << "Server started successfully on port 8080!";
            qDebug() << "Waiting for players to connect...";
            qDebug() << "============================================";
        } else {
            qDebug() << "Server failed to start:" << server->errorString();
        }
    }

private slots:
    void onNewConnection() {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        clients.append(clientSocket);
        qDebug() << "[CONNECT] A new player connected! Total players:" << clients.size();

        connect(clientSocket, &QTcpSocket::readyRead, this, &ServerController::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &ServerController::onDisconnected);
    }

    void onReadyRead() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;

        QByteArray data = clientSocket->readAll();
        QString message = QString::fromUtf8(data);
        qDebug() << "[DATA] Received from client:" << message;
    }

    void onDisconnected() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;

        clients.removeOne(clientSocket);
        qDebug() << "[DISCONNECT] A player left. Total players:" << clients.size();
        clientSocket->deleteLater();
    }

private:
    QTcpServer *server;
    QVector<QTcpSocket*> clients;
};

#endif // SERVERCONTROLLER_H