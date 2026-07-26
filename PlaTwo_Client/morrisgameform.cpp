#include "morrisgameform.h"
#include "ui_morrisgameform.h"
#include <QGraphicsLineItem>
#include <QPen>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>

MorrisGameForm::MorrisGameForm(QTcpSocket *socket, QColor p1Color, QColor p2Color, int myPlayerId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MorrisGameForm),
    m_socket(socket),
    m_p1Color(p1Color),
    m_p2Color(p2Color),
    turnTimer(nullptr),
    statusLabel(nullptr),
    selectedNode(nullptr)
{
    ui->setupUi(this);


    this->myPlayerId = myPlayerId;

    gameState = 1;
    currentPlayer = 1;
    p1PlacedCount = 0;
    p2PlacedCount = 0;
    p1RemainingCount = 0;
    p2RemainingCount = 0;
    isRemovingPhase = false;
    isGameOver = false;

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    statusLabel = new QLabel(this);
    statusLabel->setGeometry(20, 10, 320, 40);
    statusLabel->setAlignment(Qt::AlignCenter);

    turnTimer = new QTimer(this);
    connect(turnTimer, &QTimer::timeout, this, &MorrisGameForm::onTurnTimerTimeout);

    for(int i = 0; i < 7; i++)
        for(int j = 0; j < 7; j++)
            boardNodes[i][j] = nullptr;

    drawBoard();

    if (m_socket) {
        connect(m_socket, &QTcpSocket::readyRead, this, &MorrisGameForm::onReadyRead);

        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            waitingForOpponent = true;
            m_socket->write("JOIN_GAME\n");
            statusLabel->setText("Waiting for opponent...");
            statusLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #BDC3C7; background-color: rgba(0,0,0,190); border-radius: 8px; padding: 5px;");
        } else {
            startTurnTimer();
            updateTimerDisplay();
        }
    } else {
        startTurnTimer();
        updateTimerDisplay();
    }
}

MorrisGameForm::~MorrisGameForm() {
    if(turnTimer) turnTimer->stop();
    delete ui;
}

void MorrisGameForm::drawBoard() {
    int spacing = 90;
    int offset = 50;

    QPen linePen(Qt::black, 4);

    for (int i = 0; i < 3; ++i) {
        int min = i;
        int max = 6 - i;
        scene->addLine(min * spacing + offset, min * spacing + offset, max * spacing + offset, min * spacing + offset, linePen);
        scene->addLine(min * spacing + offset, max * spacing + offset, max * spacing + offset, max * spacing + offset, linePen);
        scene->addLine(min * spacing + offset, min * spacing + offset, min * spacing + offset, max * spacing + offset, linePen);
        scene->addLine(max * spacing + offset, min * spacing + offset, max * spacing + offset, max * spacing + offset, linePen);
    }

    scene->addLine(3 * spacing + offset, 0 * spacing + offset, 3 * spacing + offset, 2 * spacing + offset, linePen);
    scene->addLine(3 * spacing + offset, 4 * spacing + offset, 3 * spacing + offset, 6 * spacing + offset, linePen);
    scene->addLine(0 * spacing + offset, 3 * spacing + offset, 2 * spacing + offset, 3 * spacing + offset, linePen);
    scene->addLine(4 * spacing + offset, 3 * spacing + offset, 6 * spacing + offset, 3 * spacing + offset, linePen);

    int validPoints[24][2] = {
        {0,0}, {0,3}, {0,6},
        {1,1}, {1,3}, {1,5},
        {2,2}, {2,3}, {2,4},
        {3,0}, {3,1}, {3,2},  {3,4}, {3,5}, {3,6},
        {4,2}, {4,3}, {4,4},
        {5,1}, {5,3}, {5,5},
        {6,0}, {6,3}, {6,6}
    };

    int radius = 18;
    for (int i = 0; i < 24; ++i) {
        int r = validPoints[i][0];
        int c = validPoints[i][1];
        int x = c * spacing + offset;
        int y = r * spacing + offset;

        MorrisNodeItem *node = new MorrisNodeItem(r, c, x, y, radius);
        scene->addItem(node);
        boardNodes[r][c] = node;

        connect(node, &MorrisNodeItem::nodeClicked, this, &MorrisGameForm::onNodeClicked);
    }
}

void MorrisGameForm::onNodeClicked(int row, int col) {

    if (isGameOver) return;
    if (waitingForOpponent) {
        qDebug() << "Still waiting for an opponent to join.";
        return;
    }

    if (currentPlayer != myPlayerId) {
        qDebug() << "It's not your turn! Waiting for Player" << currentPlayer;
        return;
    }

    MorrisNodeItem* node = boardNodes[row][col];
    if (!node) return;

    if (isRemovingPhase) {
        int opponent = (currentPlayer == 1) ? 2 : 1;
        if (node->getOwner() == opponent) {

            bool protectedByMill = isPieceInAnyMill(row, col, opponent) && !allOpponentPiecesInMills(opponent);
            if (protectedByMill) {
                qDebug() << "Cannot remove a piece that's part of a mill while the opponent has free pieces.";
                return;
            }
            sendNetworkMessage(QString("MORRIS_REMOVE %1 %2").arg(row).arg(col));
            applyRemove(row, col);
        }
        return;
    }

    if (gameState == 1) {
        if (node->getOwner() == 0) {
            sendNetworkMessage(QString("MORRIS_PLACE %1 %2").arg(row).arg(col));
            applyPlace(row, col, currentPlayer);
        }
        return;
    }

    if (gameState == 2) {
        if (selectedNode == nullptr) {
            if (node->getOwner() == currentPlayer) {
                selectedNode = node;
            }
        } else {
            if (selectedNode == node) {
                selectedNode = nullptr;
                return;
            }

            if (node->getOwner() == 0) {
                int curCount = (currentPlayer == 1) ? p1RemainingCount : p2RemainingCount;
                bool canMove = (curCount == 3) || areAdjacent(selectedNode->getRow(), selectedNode->getCol(), row, col);

                if (canMove) {
                    sendNetworkMessage(QString("MORRIS_MOVE %1 %2 %3 %4")
                                           .arg(selectedNode->getRow())
                                           .arg(selectedNode->getCol())
                                           .arg(row)
                                           .arg(col));
                    applyMove(selectedNode->getRow(), selectedNode->getCol(), row, col, currentPlayer);
                }
            } else if (node->getOwner() == currentPlayer) {
                selectedNode = node;
            }
        }
    }
}

const int MILLS[16][3][2] = {

{{0,0},{0,3},{0,6}}, {{1,1},{1,3},{1,5}}, {{2,2},{2,3},{2,4}},
    {{3,0},{3,1},{3,2}}, {{3,4},{3,5},{3,6}},
    {{4,2},{4,3},{4,4}}, {{5,1},{5,3},{5,5}}, {{6,0},{6,3},{6,6}},

    {{0,0},{3,0},{6,0}}, {{1,1},{3,1},{5,1}}, {{2,2},{3,2},{4,2}},
    {{0,3},{1,3},{2,3}}, {{4,3},{5,3},{6,3}},
    {{2,4},{3,4},{4,4}}, {{1,5},{3,5},{5,5}}, {{0,6},{3,6},{6,6}}
};

bool MorrisGameForm::checkMill(int row, int col, int player) {
    for (int i = 0; i < 16; ++i) {
        bool containsPoint = false;
        for (int j = 0; j < 3; ++j) {
            if (MILLS[i][j][0] == row && MILLS[i][j][1] == col) {
                containsPoint = true;
                break;
            }
        }

        if (containsPoint) {
            bool isMill = true;
            for (int j = 0; j < 3; ++j) {
                int r = MILLS[i][j][0];
                int c = MILLS[i][j][1];
                if (!boardNodes[r][c] || boardNodes[r][c]->getOwner() != player) {
                    isMill = false;
                    break;
                }
            }
            if (isMill) return true;
        }
    }
    return false;
}

bool MorrisGameForm::isPieceInAnyMill(int row, int col, int player) {
    return checkMill(row, col, player);
}

bool MorrisGameForm::allOpponentPiecesInMills(int opponent) {
    for (int r = 0; r < 7; ++r) {
        for (int c = 0; c < 7; ++c) {
            MorrisNodeItem *node = boardNodes[r][c];
            if (node && node->getOwner() == opponent) {
                if (!checkMill(r, c, opponent)) {
                    return false;
                }
            }
        }
    }
    return true;
}

void MorrisGameForm::switchTurn() {
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    qDebug() << "Now it's Player " << currentPlayer << "'s turn.";

    resetTurnTimer();
    updateTimerDisplay();
}

void MorrisGameForm::checkGameOver() {
    if (isGameOver) return;

    if (gameState == 2) {
        QString winnerMessage;
        if (p1RemainingCount < 3) {
            winnerMessage = "Player 2 Wins! Player 1 has less than 3 pieces.";
        } else if (p2RemainingCount < 3) {
            winnerMessage = "Player 1 Wins! Player 2 has less than 3 pieces.";
        } else {
            return;
        }


        isGameOver = true;
        turnTimer->stop();
        qDebug() << winnerMessage;
        QMessageBox::information(this, "Game Over", winnerMessage);
        this->close();
    }
}

bool MorrisGameForm::areAdjacent(int r1, int c1, int r2, int c2) {

    int dr = abs(r1 - r2);
    int dc = abs(c1 - c2);

    if (r1 == r2) {
        if (r1 == 0 || r1 == 6) return dc == 3;
        if (r1 == 1 || r1 == 5) return dc == 2;
        if (r1 == 2 || r1 == 4) return dc == 1;
        if (r1 == 3) return (c1 < 3 && c2 < 3) ? dc == 1 : ((c1 > 3 && c2 > 3) ? dc == 1 : false);
    }

    if (c1 == c2) {
        if (c1 == 0 || c1 == 6) return dr == 3;
        if (c1 == 1 || c1 == 5) return dr == 2;
        if (c1 == 2 || c1 == 4) return dr == 1;
        if (c1 == 3) return (r1 < 3 && r2 < 3) ? dr == 1 : ((r1 > 3 && r2 > 3) ? dr == 1 : false);
    }

    return false;
}

void MorrisGameForm::startTurnTimer() {
    turnTimeLeft = TURN_LIMIT;
    turnTimer->start(1000);
    qDebug() << "Timer started for Player " << currentPlayer << " - Time: " << turnTimeLeft;
}

void MorrisGameForm::resetTurnTimer() {
    turnTimer->stop();
    startTurnTimer();
}

void MorrisGameForm::onTurnTimerTimeout() {
    if (isGameOver) return;

    turnTimeLeft--;
    updateTimerDisplay();
    qDebug() << "Player " << currentPlayer << " Time Left: " << turnTimeLeft;

    if (turnTimeLeft <= 0) {
        qDebug() << "Time's up for Player " << currentPlayer << "! Switching turn.";
        selectedNode = nullptr;
        isRemovingPhase = false;

        switchTurn();
        resetTurnTimer();
    }
}

void MorrisGameForm::updateTimerDisplay() {
    if (!statusLabel) return;

    bool isMyTurn = (currentPlayer == myPlayerId);

    QString turnStatus;
    if (isMyTurn) {
        turnStatus = "YOUR TURN";
    } else {
        turnStatus = "Opponent's Turn...";
    }

    QString phaseStr = (gameState == 1) ? "Placement" : "Movement";

    if (isRemovingPhase) {
        if (isMyTurn) {
            statusLabel->setText(QString("YOUR TURN: Select opponent's piece to REMOVE! (%1s)").arg(turnTimeLeft));
            statusLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #FF4D4D; background-color: rgba(0,0,0,190); border-radius: 8px; padding: 5px;");
        } else {
            statusLabel->setText(QString("Opponent is removing a piece... (%1s)").arg(turnTimeLeft));
            statusLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #FFA500; background-color: rgba(0,0,0,190); border-radius: 8px; padding: 5px;");
        }
    }
    else {
        statusLabel->setText(QString("[%1] %2 - Time: %3s").arg(phaseStr).arg(turnStatus).arg(turnTimeLeft));

        if (isMyTurn) {
            statusLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #2ECC71; background-color: rgba(0,0,0,190); border-radius: 8px; padding: 5px;");
        } else {
            statusLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #BDC3C7; background-color: rgba(0,0,0,190); border-radius: 8px; padding: 5px;");
        }
    }
}

void MorrisGameForm::sendNetworkMessage(const QString &msg) {
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = (msg + "\n").toUtf8();
        m_socket->write(data);
        m_socket->flush();
    } else {
        qDebug() << "Socket is not connected. Local mode active.";
    }
}

void MorrisGameForm::onReadyRead() {
    if (!m_socket) return;

    while (m_socket->canReadLine()) {
        QString line = QString::fromUtf8(m_socket->readLine()).trimmed();
        qDebug() << "Network Received:" << line;
        processNetworkMessage(line);
    }
}

void MorrisGameForm::processNetworkMessage(const QString &msg) {
    QString trimmed = msg.trimmed();

    if (trimmed.startsWith("GAMEID:")) {
        bool ok = false;
        int id = trimmed.section(':', 1, 1).toInt(&ok);
        if (ok && (id == 1 || id == 2)) {
            myPlayerId = id;
            qDebug() << "Assigned player id:" << myPlayerId;
            updateTimerDisplay();
        }
        return;
    }

    if (trimmed == "OPPONENT_JOINED") {
        waitingForOpponent = false;
        startTurnTimer();
        updateTimerDisplay();
        return;
    }

    if (trimmed == "OPPONENT_DISCONNECTED") {
        waitingForOpponent = true;
        turnTimer->stop();
        statusLabel->setText("Opponent disconnected");
        QMessageBox::information(this, "Opponent Disconnected", "Your opponent has left the game.");
        return;
    }

    QStringList parts = trimmed.split(" ");
    if (parts.isEmpty()) return;

    QString cmd = parts[0];

    if (cmd == "MORRIS_PLACE" && parts.size() == 3) {
        int r = parts[1].toInt();
        int c = parts[2].toInt();
        if (r >= 0 && r < 7 && c >= 0 && c < 7) {
            applyPlace(r, c, currentPlayer);
        }
    }
    else if (cmd == "MORRIS_MOVE" && parts.size() == 5) {
        int r1 = parts[1].toInt();
        int c1 = parts[2].toInt();
        int r2 = parts[3].toInt();
        int c2 = parts[4].toInt();
        if (r1 >= 0 && r1 < 7 && c1 >= 0 && c1 < 7 && r2 >= 0 && r2 < 7 && c2 >= 0 && c2 < 7) {
            applyMove(r1, c1, r2, c2, currentPlayer);
        }
    }
    else if (cmd == "MORRIS_REMOVE" && parts.size() == 3) {
        int r = parts[1].toInt();
        int c = parts[2].toInt();
        if (r >= 0 && r < 7 && c >= 0 && c < 7) {
            applyRemove(r, c);
        }
    }
}

void MorrisGameForm::applyPlace(int r, int c, int player) {
    MorrisNodeItem* node = boardNodes[r][c];
    if (!node) return;

    node->setOwner(player);
    if (player == 1) { p1PlacedCount++; p1RemainingCount++; }
    else { p2PlacedCount++; p2RemainingCount++; }

    if (checkMill(r, c, player)) {
        isRemovingPhase = true;
    } else {
        switchTurn();
    }

    if (p1PlacedCount == 9 && p2PlacedCount == 9) {
        gameState = 2;
    }
    updateTimerDisplay();
}

void MorrisGameForm::applyMove(int r1, int c1, int r2, int c2, int player) {
    MorrisNodeItem* src = boardNodes[r1][c1];
    MorrisNodeItem* dst = boardNodes[r2][c2];
    if (!src || !dst) return;

    src->setOwner(0);
    dst->setOwner(player);
    selectedNode = nullptr;

    if (checkMill(r2, c2, player)) {
        isRemovingPhase = true;
    } else {
        switchTurn();
    }
    updateTimerDisplay();
}

void MorrisGameForm::applyRemove(int r, int c) {
    MorrisNodeItem* node = boardNodes[r][c];
    if (!node) return;

    int opponent = node->getOwner();
    node->setOwner(0);

    if (opponent == 1) p1RemainingCount--;
    else if (opponent == 2) p2RemainingCount--;

    isRemovingPhase = false;
    switchTurn();
    checkGameOver();
    updateTimerDisplay();
}