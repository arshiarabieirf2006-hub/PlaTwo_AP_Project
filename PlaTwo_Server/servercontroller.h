#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QList>
#include <QMap>
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

        const QStringList lines = QString::fromUtf8(data).split("\n", Qt::SkipEmptyParts);
        for (const QString &raw : lines) {
            processMessage(clientSocket, raw.trimmed());
        }
    }

    void onDisconnected() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;

        fanoronaQueue.removeAll(clientSocket);
        if (fanoronaOpponents.contains(clientSocket)) {
            QTcpSocket* opponent = fanoronaOpponents.take(clientSocket);
            fanoronaOpponents.remove(opponent);
            opponent->write("OPPONENT_DISCONNECTED\n");
        }

        dotsQueue.removeAll(clientSocket);
        if (dotsOpponents.contains(clientSocket)) {
            QTcpSocket* opponent = dotsOpponents.take(clientSocket);
            dotsOpponents.remove(opponent);
        }

        morrisQueue.removeAll(clientSocket);
        if (morrisOpponents.contains(clientSocket)) {
            QTcpSocket* opponent = morrisOpponents.take(clientSocket);
            morrisOpponents.remove(opponent);
            opponent->write("OPPONENT_DISCONNECTED\n");
        }

        clients.removeOne(clientSocket);
        clientSocket->deleteLater();
    }

private:
    QTcpServer *server;
    QVector<QTcpSocket*> clients;

    QList<QTcpSocket*> fanoronaQueue;
    QMap<QTcpSocket*, QTcpSocket*> fanoronaOpponents;


    QList<QTcpSocket*> dotsQueue;
    QMap<QTcpSocket*, QTcpSocket*> dotsOpponents;

    QList<QTcpSocket*> morrisQueue;
    QMap<QTcpSocket*, QTcpSocket*> morrisOpponents;

    void broadcastExcept(QTcpSocket *sender, const QString &message) {
        for (QTcpSocket* otherClient : std::as_const(clients)) {
            if (otherClient != sender) {
                otherClient->write((message + "\n").toUtf8());
            }
        }
    }

    void processMessage(QTcpSocket *clientSocket, const QString &message) {
        if (message.isEmpty()) return;


        if (message.startsWith("CHAT_TEXT:") || message.startsWith("CHAT_STICKER:")) {
            QTcpSocket* opponent = nullptr;
            if (dotsOpponents.contains(clientSocket)) opponent = dotsOpponents[clientSocket];
            else if (morrisOpponents.contains(clientSocket)) opponent = morrisOpponents[clientSocket];
            else if (fanoronaOpponents.contains(clientSocket)) opponent = fanoronaOpponents[clientSocket];

            if (opponent) {
                opponent->write((message + "\n").toUtf8());
            }
            return;
        }

        // --- Dots and Boxes ---
        if (message == "REQUEST_DOTS") {
            if (!dotsQueue.contains(clientSocket)) {
                dotsQueue.append(clientSocket);
            }
            if (dotsQueue.size() >= 2) {
                QTcpSocket* player1 = dotsQueue.takeFirst();
                QTcpSocket* player2 = dotsQueue.takeFirst();

                dotsOpponents[player1] = player2;
                dotsOpponents[player2] = player1;

                player1->write("START_DOTS|1\n");
                player2->write("START_DOTS|2\n");
            }
            return;
        }
        if (message.startsWith("MOVE")) {

            if (dotsOpponents.contains(clientSocket)) {
                dotsOpponents[clientSocket]->write((message + "\n").toUtf8());
            }
            return;
        }
        if (message == "SKIP_TURN") {
            if (dotsOpponents.contains(clientSocket)) {
                dotsOpponents[clientSocket]->write("SKIP_TURN\n");
            }
            return;
        }

        if (message == "REQUEST_MORRIS") {

            if (!morrisQueue.contains(clientSocket)) {
                morrisQueue.append(clientSocket);
            }
            if (morrisQueue.size() >= 2) {
                QTcpSocket* player1 = morrisQueue.takeFirst();
                QTcpSocket* player2 = morrisQueue.takeFirst();

                morrisOpponents[player1] = player2;
                morrisOpponents[player2] = player1;

                player1->write("START_MORRIS|1\n");
                player2->write("START_MORRIS|2\n");
            }
            return;
        }
        if (message.startsWith("MORRIS_PLACE") || message.startsWith("MORRIS_MOVE") || message.startsWith("MORRIS_REMOVE")) {
            if (morrisOpponents.contains(clientSocket)) {
                morrisOpponents[clientSocket]->write((message + "\n").toUtf8());
            }
            return;
        }

        if (message == "REQUEST_FANORONA") {
            if (!fanoronaQueue.contains(clientSocket)) {
                fanoronaQueue.append(clientSocket);
            }

            if (fanoronaQueue.size() >= 2) {
                QTcpSocket* player1 = fanoronaQueue.takeFirst();
                QTcpSocket* player2 = fanoronaQueue.takeFirst();

                fanoronaOpponents[player1] = player2;
                fanoronaOpponents[player2] = player1;

                player1->write("START_FANORONA|1\n");
                player2->write("START_FANORONA|2\n");
            }
            return;
        }
        if (message.startsWith("FANORONA_MOVE") || message.startsWith("FANORONA_PASS")) {
            if (fanoronaOpponents.contains(clientSocket)) {
                QTcpSocket* opponent = fanoronaOpponents[clientSocket];
                opponent->write((message + "\n").toUtf8());
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