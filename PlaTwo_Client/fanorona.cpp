#include "fanorona.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>
#include <cmath>

Fanorona::Fanorona(int playerRole, QWidget *parent) : QWidget(parent), myPlayerRole(playerRole)
{
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);

    layout = new QVBoxLayout(this);
    timeRemaining = 60;

    timerLabel = new QLabel("Time: 60", this);
    timerLabel->setAlignment(Qt::AlignCenter);
    timerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #D32F2F;");

    turnLabel = new QLabel("", this);
    turnLabel->setAlignment(Qt::AlignCenter);

    layout->insertWidget(0, timerLabel);
    layout->insertWidget(1, turnLabel);

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &Fanorona::updateTimer);
    gameTimer->start(1000);

    layout->addWidget(view);
    this->setLayout(layout);

    scene->setBackgroundBrush(QBrush(QColor(232, 195, 150)));

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    currentPlayer = 1;
    selectedRow = -1;
    selectedCol = -1;

    isComboActive = false;
    comboRow = -1;
    comboCol = -1;

    initializeBoard();
    drawBoard();
    updateTurnLabel();
}

Fanorona::~Fanorona()
{
}

void Fanorona::updateTurnLabel()
{
    if (currentPlayer == myPlayerRole) {
        turnLabel->setText("Your Turn!");
        turnLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2E7D32;");
    } else {
        turnLabel->setText("Opponent's Turn...");
        turnLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1565C0;");
    }
}

void Fanorona::checkGameOver()
{
    int p1Count = 0;
    int p2Count = 0;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 9; c++) {
            if (board[r][c] == 1) p1Count++;
            if (board[r][c] == 2) p2Count++;
        }
    }

    if (p1Count == 0 || p2Count == 0) {
        gameTimer->stop();
        QString winnerMsg = (p1Count == 0) ? "Black (Player 2) Wins!" : "White (Player 1) Wins!";
        QMessageBox::information(this, "Game Over", winnerMsg);
        this->close();
    }
}

void Fanorona::initializeBoard()
{
    int initialSetup[5][9] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 2, 1, 2, 0, 1, 2, 1, 2},
        {2, 2, 2, 2, 2, 2, 2, 2, 2},
        {2, 2, 2, 2, 2, 2, 2, 2, 2}
    };

    for(int r = 0; r < 5; r++) {
        for(int c = 0; c < 9; c++) {
            board[r][c] = initialSetup[r][c];
        }
    }
}

void Fanorona::drawBoard()
{
    QPen linePen(Qt::black);
    linePen.setWidth(2);

    for(int r = 0; r < 5; r++) {
        for(int c = 0; c < 9; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE;

            if (c < 8) scene->addLine(x, y, x + CELL_SIZE, y, linePen);
            if (r < 4) scene->addLine(x, y, x, y + CELL_SIZE, linePen);
            if ((r + c) % 2 == 0) {
                if (r < 4 && c < 8) scene->addLine(x, y, x + CELL_SIZE, y + CELL_SIZE, linePen);
                if (r < 4 && c > 0) scene->addLine(x, y, x - CELL_SIZE, y + CELL_SIZE, linePen);
            }
        }
    }

    int pieceRadius = 26;
    for(int r = 0; r < 5; r++) {
        for(int c = 0; c < 9; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE;

            if (board[r][c] != 0) {
                QGraphicsEllipseItem *piece = new QGraphicsEllipseItem(x - pieceRadius, y - pieceRadius, pieceRadius * 2, pieceRadius * 2);
                if (board[r][c] == 1) piece->setBrush(QBrush(Qt::white));
                else if (board[r][c] == 2) piece->setBrush(QBrush(Qt::black));
                piece->setPen(QPen(Qt::black, 2));
                scene->addItem(piece);

                if (r == selectedRow && c == selectedCol) {
                    QGraphicsEllipseItem *highlight = new QGraphicsEllipseItem(x - pieceRadius - 4, y - pieceRadius - 4, (pieceRadius + 4) * 2, (pieceRadius + 4) * 2);
                    highlight->setPen(QPen(Qt::green, 3));
                    scene->addItem(highlight);
                }
            }
            else {
                int dotRadius = 4;
                QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(x - dotRadius, y - dotRadius, dotRadius * 2, dotRadius * 2);
                dot->setBrush(QBrush(Qt::black));
                scene->addItem(dot);
            }
        }
    }
}

void Fanorona::updateBoardUI()
{
    scene->clear();
    drawBoard();
}

void Fanorona::mousePressEvent(QMouseEvent *event)
{

    if (currentPlayer != myPlayerRole) {
        return;
    }

    if (event->button() == Qt::RightButton) {
        if (isComboActive) {
            isComboActive = false;
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            selectedRow = -1;
            selectedCol = -1;
            updateBoardUI();
            updateTurnLabel();
            emit turnPassed();

            timeRemaining = 60;
            timerLabel->setText("Time: 60");
            gameTimer->start(1000);
        }
        return;
    }

    if (event->button() != Qt::LeftButton) return;

    QPointF scenePos = view->mapToScene(event->pos());
    int c = (int)(scenePos.x() + CELL_SIZE / 2) / CELL_SIZE;
    int r = (int)(scenePos.y() + CELL_SIZE / 2) / CELL_SIZE;

    if (r >= 0 && r < 5 && c >= 0 && c < 9) {
        if (selectedRow == -1) {
            if (board[r][c] == currentPlayer) {
                if (isComboActive && (r != comboRow || c != comboCol)) return;
                selectedRow = r;
                selectedCol = c;
                updateBoardUI();
            }
        }
        else {
            if (board[r][c] == 0) {
                int rowDiff = std::abs(r - selectedRow);
                int colDiff = std::abs(c - selectedCol);

                if (rowDiff <= 1 && colDiff <= 1 && (rowDiff + colDiff > 0)) {
                    bool isValidMove = true;
                    if ((selectedRow + selectedCol) % 2 != 0 && (rowDiff == 1 && colDiff == 1)) {
                        isValidMove = false;
                    }

                    if (isValidMove) {
                        board[r][c] = currentPlayer;
                        board[selectedRow][selectedCol] = 0;

                        emit movePlayed(selectedRow, selectedCol, r, c);

                        bool didCapture = handleCapture(selectedRow, selectedCol, r, c);

                        if (didCapture) {
                            isComboActive = true;
                            comboRow = r;
                            comboCol = c;
                        } else {
                            isComboActive = false;
                            currentPlayer = (currentPlayer == 1) ? 2 : 1;
                            updateTurnLabel();
                        }

                        selectedRow = -1;
                        selectedCol = -1;
                        updateBoardUI();
                        checkGameOver();

                        timeRemaining = 60;
                        timerLabel->setText("Time: 60");
                        gameTimer->start(1000);
                    }
                }
            }
            else if (board[r][c] == currentPlayer) {
                if (!isComboActive) {
                    selectedRow = r;
                    selectedCol = c;
                    updateBoardUI();
                }
            }
        }
    }
}

bool Fanorona::handleCapture(int startRow, int startCol, int endRow, int endCol)
{
    int dr = endRow - startRow;
    int dc = endCol - startCol;
    int opponent = (currentPlayer == 1) ? 2 : 1;
    bool captured = false;

    int currR = endRow + dr;
    int currC = endCol + dc;

    while (currR >= 0 && currR < 5 && currC >= 0 && currC < 9 && board[currR][currC] == opponent) {
        board[currR][currC] = 0;
        captured = true;
        currR += dr;
        currC += dc;
    }

    if (!captured) {
        currR = startRow - dr;
        currC = startCol - dc;

        while (currR >= 0 && currR < 5 && currC >= 0 && currC < 9 && board[currR][currC] == opponent) {
            board[currR][currC] = 0;
            captured = true;
            currR -= dr;
            currC -= dc;
        }
    }

    return captured;
}

void Fanorona::applyOpponentMove(int startRow, int startCol, int endRow, int endCol)
{
    board[endRow][endCol] = currentPlayer;
    board[startRow][startCol] = 0;

    bool didCapture = handleCapture(startRow, startCol, endRow, endCol);

    if (didCapture) {
        isComboActive = true;
        comboRow = endRow;
        comboCol = endCol;
    } else {
        isComboActive = false;
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
        updateTurnLabel();
    }

    selectedRow = -1;
    selectedCol = -1;
    updateBoardUI();
    checkGameOver();

    timeRemaining = 60;
    timerLabel->setText("Time: 60");
    gameTimer->start(1000);
}

void Fanorona::applyOpponentPass()
{
    isComboActive = false;
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    selectedRow = -1;
    selectedCol = -1;
    updateBoardUI();
    updateTurnLabel();

    timeRemaining = 60;
    timerLabel->setText("Time: 60");
    gameTimer->start(1000);
}

void Fanorona::updateTimer()
{
    timeRemaining--;
    timerLabel->setText(QString("Time: %1").arg(timeRemaining));

    if (timeRemaining <= 0) {
        gameTimer->stop();
        timerLabel->setText("Time's up!");


        if (currentPlayer == myPlayerRole) {
            emit turnPassed();
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            updateTurnLabel();
        }

        timeRemaining = 60;
        gameTimer->start(1000);
    }
}
void Fanorona::handleDisconnect()
{
    gameTimer->stop();
    QMessageBox::information(this, "Game Over", "Opponent disconnected. You Win!");
    this->close();
}