#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QPair>
#include <algorithm>

class ServerController : public QObject {
    Q_OBJECT
public:
    ServerController() {
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &ServerController::onNewConnection);

        if (server->listen(QHostAddress::Any, 8080)) {
            qDebug() << "Server started on port 8080";
        } else {
            qDebug() << "Server failed to start:" << server->errorString();
        }
    }

private slots:
    void onNewConnection() {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        clients.append(clientSocket);

        connect(clientSocket, &QTcpSocket::readyRead, this, &ServerController::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &ServerController::onDisconnected);
    }

    void onReadyRead() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;

        QByteArray data = clientSocket->readAll();
        QString message = QString::fromUtf8(data).trimmed();

        QStringList parts = message.split(":");
        if (parts.isEmpty()) return;

        QString command = parts[0];

        if (command == "SIGNUP" && parts.size() >= 6) {
            QString username = parts[1];
            QString passwordHash = parts[2];
            QString email = parts[3];
            QString name = parts[4];
            QString phone = parts[5];

            QFile checkFile("server_users.txt");
            bool duplicate = false;

            if (checkFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&checkFile);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList details = line.split(",");
                    if (!details.isEmpty() && details[0] == username) {
                        duplicate = true;
                        break;
                    }
                }
                checkFile.close();
            }

            if (duplicate) {
                clientSocket->write("SIGNUP_DUPLICATE\n");
            } else {
                QFile file("server_users.txt");
                if (file.open(QIODevice::Append | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << username << "," << passwordHash << "," << email << "," << name << "," << phone << ",100\n";
                    file.close();
                    clientSocket->write("SIGNUP_SUCCESS\n");
                } else {
                    clientSocket->write("SIGNUP_FAILED\n");
                }
            }
        }
        else if (command == "LOGIN" && parts.size() >= 3) {
            QString username = parts[1];
            QString passwordHash = parts[2];
            bool success = false;

            QFile file("server_users.txt");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList details = line.split(",");
                    if (details.size() >= 2) {
                        if (details[0] == username && details[1] == passwordHash) {
                            success = true;
                            break;
                        }
                    }
                }
                file.close();
            }

            if (success) {
                clientSocket->write("LOGIN_SUCCESS\n");
            } else {
                clientSocket->write("LOGIN_FAILED\n");
            }
        }
        else if (command == "GET_COINS" && parts.size() >= 2) {
            QString username = parts[1];
            int coins = 0;

            QFile file("server_users.txt");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList details = line.split(",");
                    if (details.size() >= 6 && details[0] == username) {
                        coins = details[5].toInt();
                        break;
                    }
                }
                file.close();
            }
            clientSocket->write(QString("COINS_RESULT:" + QString::number(coins) + "\n").toUtf8());
        }
        else if (command == "BUY_ITEM" && parts.size() >= 3) {
            QString username = parts[1];
            int cost = parts[2].toInt();

            QFile file("server_users.txt");
            QStringList lines;
            bool success = false;
            int newBalance = 0;

            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    lines.append(in.readLine());
                }
                file.close();
            }

            for (int i = 0; i < lines.size(); ++i) {
                QStringList details = lines[i].split(",");
                if (details.size() >= 6 && details[0] == username) {
                    int currentCoins = details[5].toInt();
                    if (currentCoins >= cost) {
                        currentCoins -= cost;
                        newBalance = currentCoins;
                        details[5] = QString::number(currentCoins);
                        lines[i] = details.join(",");
                        success = true;
                    }
                    break;
                }
            }

            if (success) {
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream out(&file);
                    for (const QString &line : std::as_const(lines)) {
                        out << line << "\n";
                    }
                    file.close();
                }
                clientSocket->write(QString("BUY_SUCCESS:" + QString::number(newBalance) + "\n").toUtf8());
            } else {
                clientSocket->write("BUY_FAILED_NO_COINS\n");
            }
        }
        else if (command == "GET_LEADERBOARD") {
            QVector<QPair<QString, int>> players;

            QFile file("server_users.txt");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList details = line.split(",");
                    if (details.size() >= 6) {
                        QString username = details[0].trimmed();
                        int score = details[5].trimmed().toInt();
                        players.append(qMakePair(username, score));
                    }
                }
                file.close();
            }

            std::sort(players.begin(), players.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                return a.second > b.second;
            });

            QString response = "LEADERBOARD_RESULT:";
            QStringList entryList;
            for (const auto &player : std::as_const(players)) {
                entryList.append(player.first + "," + QString::number(player.second));
            }
            response += entryList.join(";") + "\n";

            clientSocket->write(response.toUtf8());
        }
    }

    void onDisconnected() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;

        clients.removeOne(clientSocket);
        clientSocket->deleteLater();
    }

private:
    QTcpServer *server;
    QVector<QTcpSocket*> clients;
};

#endif // SERVERCONTROLLER_H