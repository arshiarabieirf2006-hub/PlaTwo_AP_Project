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
#include <QMap>
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


        QByteArray &buffer = recvBuffers[clientSocket];
        buffer += clientSocket->readAll();

        int newlineIdx;
        while ((newlineIdx = buffer.indexOf('\n')) != -1) {
            QByteArray lineBytes = buffer.left(newlineIdx);
            buffer.remove(0, newlineIdx + 1);
            QString line = QString::fromUtf8(lineBytes).trimmed();
            if (!line.isEmpty()) {
                processMessage(clientSocket, line);
            }
        }
    }

    void onDisconnected() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;

        clients.removeOne(clientSocket);
        recvBuffers.remove(clientSocket);
        playerIdOf.remove(clientSocket);

        if (pendingPlayer == clientSocket) {
            pendingPlayer = nullptr;
        }

        QTcpSocket *opponent = opponentMap.value(clientSocket, nullptr);
        if (opponent) {
            opponent->write("OPPONENT_DISCONNECTED\n");
            opponentMap.remove(opponent);
        }
        opponentMap.remove(clientSocket);

        clientSocket->deleteLater();
    }

private:
    QTcpServer *server;
    QVector<QTcpSocket*> clients;

    QMap<QTcpSocket*, QByteArray> recvBuffers;


    QTcpSocket *pendingPlayer = nullptr;
    QMap<QTcpSocket*, QTcpSocket*> opponentMap;
    QMap<QTcpSocket*, int> playerIdOf;

    void routeToOpponent(QTcpSocket *sender, const QString &message) {
        QTcpSocket *opponent = opponentMap.value(sender, nullptr);
        if (opponent) {
            opponent->write((message + "\n").toUtf8());
        }
    }

    void processMessage(QTcpSocket *clientSocket, const QString &message) {
        if (message.isEmpty()) return;

        if (message.startsWith("MOVE")) {
            routeToOpponent(clientSocket, message);
            return;
        }
        if (message.startsWith("MORRIS_PLACE") || message.startsWith("MORRIS_MOVE") || message.startsWith("MORRIS_REMOVE")) {
            routeToOpponent(clientSocket, message);
            return;
        }
        if (message == "SKIP_TURN") {

            routeToOpponent(clientSocket, message);
            return;
        }
        if (message == "JOIN_GAME") {

            if (playerIdOf.contains(clientSocket)) {
                clientSocket->write(QString("GAMEID:%1\n").arg(playerIdOf[clientSocket]).toUtf8());
                if (opponentMap.contains(clientSocket)) {
                    clientSocket->write("OPPONENT_JOINED\n");
                }
                return;
            }

            if (pendingPlayer == nullptr) {
                pendingPlayer = clientSocket;
                playerIdOf[clientSocket] = 1;
                clientSocket->write("GAMEID:1\n");
            } else {
                QTcpSocket *opponent = pendingPlayer;
                pendingPlayer = nullptr;

                playerIdOf[clientSocket] = 2;
                opponentMap[clientSocket] = opponent;
                opponentMap[opponent] = clientSocket;

                clientSocket->write("GAMEID:2\n");
                clientSocket->write("OPPONENT_JOINED\n");
                opponent->write("OPPONENT_JOINED\n");
            }
            return;
        }

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

            clientSocket->write(success ? "LOGIN_SUCCESS\n" : "LOGIN_FAILED\n");
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
        else if (command == "FORGOT_PASS" && parts.size() >= 3) {
            QString username = parts[1];
            QString newPasswordHash = parts[2];

            QFile file("server_users.txt");
            QStringList lines;
            bool userFound = false;

            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    lines.append(in.readLine());
                }
                file.close();
            }

            for (int i = 0; i < lines.size(); ++i) {
                QStringList details = lines[i].split(",");
                if (!details.isEmpty() && details[0] == username) {
                    details[1] = newPasswordHash;
                    lines[i] = details.join(",");
                    userFound = true;
                    break;
                }
            }

            if (userFound) {
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream out(&file);
                    for (const QString &line : std::as_const(lines)) {
                        out << line << "\n";
                    }
                    file.close();
                }
                clientSocket->write("FORGOT_PASS_SUCCESS\n");
            } else {
                clientSocket->write("FORGOT_PASS_FAILED\n");
            }
        }
        else if (command == "PROFILE_GET" && parts.size() >= 2) {

            QString username = parts[1];
            QFile file("server_users.txt");
            bool found = false;

            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList details = line.split(",");
                    if (details.size() >= 6 && details[0] == username) {
                        clientSocket->write(QString("PROFILE_DATA:" + line + "\n").toUtf8());
                        found = true;
                        break;
                    }
                }
                file.close();
            }
            if (!found) {
                clientSocket->write("PROFILE_NOTFOUND\n");
            }
        }
        else if (command == "PROFILE_UPDATE" && parts.size() >= 7) {
            QString oldUsername = parts[1];
            QString newUsername = parts[2];
            QString newPasswordHash = parts[3];
            QString newEmail = parts[4];
            QString newName = parts[5];
            QString newPhone = parts[6];

            QFile file("server_users.txt");
            QStringList lines;
            bool userFound = false;

            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    lines.append(in.readLine());
                }
                file.close();
            }

            for (int i = 0; i < lines.size(); ++i) {
                if (lines[i].trimmed().isEmpty()) continue;
                QStringList details = lines[i].split(",");
                if (details.size() >= 6 && details[0] == oldUsername) {
                    details[0] = newUsername;
                    if (!newPasswordHash.isEmpty()) details[1] = newPasswordHash;
                    details[2] = newEmail;
                    details[3] = newName;
                    details[4] = newPhone;
                    lines[i] = details.join(",");
                    userFound = true;
                    break;
                }
            }

            if (userFound) {
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream out(&file);
                    for (const QString &line : std::as_const(lines)) {
                        out << line << "\n";
                    }
                    file.close();
                }
                clientSocket->write("PROFILE_UPDATE_SUCCESS\n");
            } else {
                clientSocket->write("PROFILE_UPDATE_FAILED\n");
            }
        }
    }
};

#endif // SERVERCONTROLLER_H