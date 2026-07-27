#include "gameform.h"
#include "ui_gameform.h"
#include "lineitem.h"
#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QDebug>
#include <QMessageBox>
#include <QBoxLayout>
#include <QResizeEvent>
#include <QDateTime>

GameForm::GameForm(QTcpSocket *serverSocket, QColor color1, QColor color2, int myPlayerId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameForm)
    , socket(serverSocket)
    , myPlayerId(myPlayerId)
    , p1Color(color1)
    , p2Color(color2)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    gridSize = 6;
    currentPlayer = 1;
    player1Score = 0;
    player2Score = 0;

    hLines.resize(gridSize);
    hLineItems.resize(gridSize);
    for (int i = 0; i < gridSize; ++i) {
        hLines[i].resize(gridSize - 1, false);
        hLineItems[i].resize(gridSize - 1, nullptr);
    }

    vLines.resize(gridSize - 1);
    vLineItems.resize(gridSize - 1);
    for (int i = 0; i < gridSize - 1; ++i) {
        vLines[i].resize(gridSize, false);
        vLineItems[i].resize(gridSize, nullptr);
    }

    boxes.resize(gridSize - 1);
    for (int i = 0; i < gridSize - 1; ++i) boxes[i].resize(gridSize - 1, 0);

    int dotSpacing = 70;
    int dotSize = 12;
    int lineThickness = 14;

    QBrush blackBrush(Qt::black);

    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize - 1; ++col) {
            int x = col * dotSpacing + dotSize / 2;
            int y = row * dotSpacing + dotSize / 2 - lineThickness / 2;

            LineItem *hLine = new LineItem(x, y, dotSpacing, lineThickness, true, row, col);
            scene->addItem(hLine);
            hLineItems[row][col] = hLine;

            connect(hLine, &LineItem::lineClicked, this, &GameForm::onLineClicked);
        }
    }

    for (int row = 0; row < gridSize - 1; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            int x = col * dotSpacing + dotSize / 2 - lineThickness / 2;
            int y = row * dotSpacing + dotSize / 2;

            LineItem *vLine = new LineItem(x, y, lineThickness, dotSpacing, false, row, col);
            scene->addItem(vLine);
            vLineItems[row][col] = vLine;

            connect(vLine, &LineItem::lineClicked, this, &GameForm::onLineClicked);
        }
    }

    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            int x = col * dotSpacing;
            int y = row * dotSpacing;
            scene->addEllipse(x, y, dotSize, dotSize, QPen(Qt::NoPen), blackBrush);
        }
    }

    connect(socket, &QTcpSocket::readyRead, this, &GameForm::onServerMessage);

    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #D32F2F;");

    statusLabel->setGeometry(0, 20, this->width(), 40);
    statusLabel->show();

    turnTimer = new QTimer(this);
    connect(turnTimer, &QTimer::timeout, this, &GameForm::onTurnTimerTimeout);

    chatWidget = new ChatWidget(this);
    chatWidget->resize(260, 260);
    chatWidget->move(this->width() - chatWidget->width() - 10, this->height() - chatWidget->height() - 10);
    chatWidget->show();

    connect(chatWidget, &ChatWidget::sendTextRequested, this, [this](const QString &text) {
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(("CHAT_TEXT:" + text + "\n").toUtf8());
        }
    });
    connect(chatWidget, &ChatWidget::sendStickerRequested, this, [this](int index) {
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(QString("CHAT_STICKER:%1\n").arg(index).toUtf8());
        }
    });

    startTurnTimer();
}

void GameForm::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (chatWidget) {
        chatWidget->move(this->width() - chatWidget->width() - 10, this->height() - chatWidget->height() - 10);
    }

    if (statusLabel) {
        statusLabel->setGeometry(0, 20, this->width(), 40);
    }
}

GameForm::~GameForm()
{
    if (turnTimer) turnTimer->stop();
    delete ui;
}

void GameForm::onLineClicked(int row, int col, bool isHoriz)
{
    if (isHoriz && hLines[row][col]) return;
    if (!isHoriz && vLines[row][col]) return;

    if (!isProcessingNetworkMove) {
        if (currentPlayer != myPlayerId) {
            qDebug() << "It's not your turn! Waiting for Player" << currentPlayer;
            return;
        }
        QString msg = QString("MOVE:%1:%2:%3\n").arg(row).arg(col).arg(isHoriz ? 1 : 0);
        socket->write(msg.toUtf8());
    }

    LineItem *item = isHoriz ? hLineItems[row][col] : vLineItems[row][col];
    QColor moveColor = (currentPlayer == 1) ? p1Color : p2Color;
    if (item) item->confirmClick(moveColor);

    if (isHoriz) {
        hLines[row][col] = true;
    } else {
        vLines[row][col] = true;
    }

    bool boxCompleted = checkForCompletedBoxes(row, col, isHoriz);

    if (!boxCompleted) {
        switchTurn();
        qDebug() << "Turn changed. Current player: " << currentPlayer;
    } else {
        resetTurnTimer();
        qDebug() << "Box completed! Bonus turn for player: " << currentPlayer;
        qDebug() << "Scores -> Player 1: " << player1Score << " | Player 2: " << player2Score;
    }
    int totalBoxes = (gridSize - 1) * (gridSize - 1);

    if (player1Score + player2Score == totalBoxes) {
        QString winnerMessage;
        QString winnerStr;

        if (player1Score > player2Score) {
            winnerMessage = "Congratulations! Player 1 wins!";
            winnerStr = "Player 1";
        } else if (player2Score > player1Score) {
            winnerMessage = "Congratulations! Player 2 wins!";
            winnerStr = "Player 2";
        } else {
            winnerMessage = "It's a draw!";
            winnerStr = "Draw";
        }

        if (!myUsername.isEmpty() && socket && socket->state() == QAbstractSocket::ConnectedState) {
            QString roleStr = (myPlayerId == 1) ? "Player 1" : "Player 2";
            QString scoreStr = QString("%1-%2").arg(player1Score).arg(player2Score);

            // رفع باگ: جایگزینی : با - برای جلوگیری از مشکل در split سرور
            QString dateStr = QDateTime::currentDateTime().toString("yyyy/MM/dd hh-mm");

            QString historyCmd = QString("SAVE_HISTORY:%1:Dots And Boxes:Opponent:%2:%3:%4:%5\n")
                                     .arg(myUsername, roleStr, winnerStr, scoreStr, dateStr);
            socket->write(historyCmd.toUtf8());
        }

        if (turnTimer) turnTimer->stop();

        QMessageBox::information(this, "Game Over", winnerMessage);

        this->close();
    }
}

bool GameForm::checkForCompletedBoxes(int row, int col, bool isHoriz)
{
    bool completed = false;
    int dotSpacing = 70;

    auto checkAndClaimBox = [&](int r, int c) {
        if (r >= 0 && r < gridSize - 1 && c >= 0 && c < gridSize - 1) {
            if (boxes[r][c] == 0) {
                if (hLines[r][c] && hLines[r + 1][c] && vLines[r][c] && vLines[r][c + 1]) {
                    boxes[r][c] = currentPlayer;

                    if (currentPlayer == 1) player1Score++;
                    else player2Score++;

                    QString playerText = (currentPlayer == 1) ? "P1" : "P2";
                    QGraphicsTextItem *text = new QGraphicsTextItem(playerText);

                    text->setDefaultTextColor(currentPlayer == 1 ? p1Color : p2Color);

                    QFont font("Arial", 18, QFont::Bold);
                    text->setFont(font);

                    int xPos = c * dotSpacing + (dotSpacing / 2) - 16;
                    int yPos = r * dotSpacing + (dotSpacing / 2) - 16;
                    text->setPos(xPos, yPos);

                    scene->addItem(text);

                    return true;
                }
            }
        }
        return false;
    };

    if (isHoriz) {
        if (checkAndClaimBox(row - 1, col)) completed = true;
        if (checkAndClaimBox(row, col)) completed = true;
    }
    else {
        if (checkAndClaimBox(row, col - 1)) completed = true;
        if (checkAndClaimBox(row, col)) completed = true;
    }

    return completed;
}

void GameForm::onServerMessage() {

    recvBuffer += socket->readAll();

    int newlineIndex;
    while ((newlineIndex = recvBuffer.indexOf('\n')) != -1) {
        QByteArray lineData = recvBuffer.left(newlineIndex);
        recvBuffer.remove(0, newlineIndex + 1);

        QString msg = QString::fromUtf8(lineData).trimmed();
        if (msg.isEmpty()) continue;

        if (msg == "SKIP_TURN") {
            switchTurn();
            continue;
        }

        if (msg.startsWith("CHAT_TEXT:")) {
            chatWidget->addIncomingText(msg.mid(QString("CHAT_TEXT:").length()));
            continue;
        }
        if (msg.startsWith("CHAT_STICKER:")) {
            chatWidget->addIncomingSticker(msg.mid(QString("CHAT_STICKER:").length()).toInt());
            continue;
        }

        QStringList parts = msg.split(":");
        if (parts.size() >= 4 && parts[0] == "MOVE") {
            int r = parts[1].toInt();
            int c = parts[2].toInt();
            bool isH = (parts[3] == "1");

            int maxRow = isH ? gridSize : gridSize - 1;
            int maxCol = isH ? gridSize - 1 : gridSize;
            if (r < 0 || r >= maxRow || c < 0 || c >= maxCol) {
                qDebug() << "Ignoring out-of-range network move:" << r << c << isH;
                continue;
            }

            LineItem *item = isH ? hLineItems[r][c] : vLineItems[r][c];
            if (item && item->clicked()) {
                qDebug() << "Ignoring network move for an already-claimed line:" << r << c << isH;
                continue;
            }

            isProcessingNetworkMove = true;
            onLineClicked(r, c, isH);
            isProcessingNetworkMove = false;
        }
    }
}

void GameForm::switchTurn()
{
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    resetTurnTimer();
}

void GameForm::startTurnTimer()
{
    turnTimeLeft = TURN_LIMIT;
    updateTimerDisplay();
    if (turnTimer) turnTimer->start(1000);
}

void GameForm::resetTurnTimer()
{
    turnTimeLeft = TURN_LIMIT;
    updateTimerDisplay();
    if (turnTimer && !turnTimer->isActive()) {
        turnTimer->start(1000);
    }
}

void GameForm::updateTimerDisplay()
{
    if (!statusLabel) return;

    QString playerText = (currentPlayer == myPlayerId) ? "Your Turn" : "Opponent's Turn";
    statusLabel->setText(QString("%1 — Time: %2").arg(playerText).arg(turnTimeLeft));
}

void GameForm::onTurnTimerTimeout()
{
    if (turnTimeLeft > 0) {
        turnTimeLeft--;
        updateTimerDisplay();
        return;
    }

    qDebug() << "Turn timer expired for player" << currentPlayer;

    if (currentPlayer == myPlayerId) {
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            socket->write("SKIP_TURN\n");
        }
        switchTurn();
    } else {
        resetTurnTimer();
    }
}