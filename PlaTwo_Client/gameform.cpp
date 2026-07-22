#include "gameform.h"
#include "ui_gameform.h"
#include "lineitem.h"
#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QDebug>
#include <QMessageBox>
GameForm::GameForm(QTcpSocket *serverSocket, QColor color1, QColor color2, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameForm)
    , socket(serverSocket)
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
    for (int i = 0; i < gridSize; ++i) hLines[i].resize(gridSize - 1, false);

    vLines.resize(gridSize - 1);
    for (int i = 0; i < gridSize - 1; ++i) vLines[i].resize(gridSize, false);

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

            connect(hLine, &LineItem::lineClicked, this, &GameForm::onLineClicked);
        }
    }

    for (int row = 0; row < gridSize - 1; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            int x = col * dotSpacing + dotSize / 2 - lineThickness / 2;
            int y = row * dotSpacing + dotSize / 2;

            LineItem *vLine = new LineItem(x, y, lineThickness, dotSpacing, false, row, col);
            scene->addItem(vLine);

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
}

GameForm::~GameForm()
{
    delete ui;
}

void GameForm::onLineClicked(int row, int col, bool isHoriz)
{
    if (!isProcessingNetworkMove) {
        QString msg = QString("MOVE:%1:%2:%3\n").arg(row).arg(col).arg(isHoriz ? 1 : 0);
        socket->write(msg.toUtf8());
    }
    if (isHoriz) {
        hLines[row][col] = true;
    } else {
        vLines[row][col] = true;
    }

    bool boxCompleted = checkForCompletedBoxes(row, col, isHoriz);

    if (!boxCompleted) {
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
        qDebug() << "Turn changed. Current player: " << currentPlayer;
    } else {
        qDebug() << "Box completed! Bonus turn for player: " << currentPlayer;
        qDebug() << "Scores -> Player 1: " << player1Score << " | Player 2: " << player2Score;
    }
    int totalBoxes = (gridSize - 1) * (gridSize - 1);

    if (player1Score + player2Score == totalBoxes) {
        QString winnerMessage;

        if (player1Score > player2Score) {
            winnerMessage = "Congratulations! Player 1 wins!";
        } else if (player2Score > player1Score) {
            winnerMessage = "Congratulations! Player 2 wins!";
        } else {
            winnerMessage = "It's a draw!";
        }

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
    QByteArray data = socket->readAll();
    QStringList messages = QString::fromUtf8(data).split("\n", Qt::SkipEmptyParts);

    for (const QString& msg : messages) {
        QStringList parts = msg.trimmed().split(":");
        if (parts.size() >= 4 && parts[0] == "MOVE") {
            int r = parts[1].toInt();
            int c = parts[2].toInt();
            bool isH = (parts[3] == "1");


            isProcessingNetworkMove = true;
            onLineClicked(r, c, isH);
            isProcessingNetworkMove = false;
        }
    }
}