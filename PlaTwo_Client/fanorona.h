#ifndef FANORONA_H
#define FANORONA_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTimer>
#include <QLabel>
#include <QMessageBox>

class Fanorona : public QWidget
{
    Q_OBJECT

public:

    explicit Fanorona(int playerRole = 1, QWidget *parent = nullptr);
    ~Fanorona();
    void handleDisconnect();

signals:
    void movePlayed(int startRow, int startCol, int endRow, int endCol);
    void turnPassed();

public slots:
    void applyOpponentMove(int startRow, int startCol, int endRow, int endCol);
    void applyOpponentPass();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateTimer();

private:
    QGraphicsScene *scene;
    QGraphicsView *view;
    QVBoxLayout *layout;

    int board[5][9];
    const int CELL_SIZE = 80;

    int currentPlayer;
    int myPlayerRole;
    int selectedRow;
    int selectedCol;

    bool isComboActive;
    int comboRow;
    int comboCol;

    QTimer *gameTimer;
    QLabel *timerLabel;
    QLabel *turnLabel;
    int timeRemaining;

    bool handleCapture(int startRow, int startCol, int endRow, int endCol);
    void initializeBoard();
    void drawBoard();
    void updateBoardUI();

    void checkGameOver();
    void updateTurnLabel();
};

#endif