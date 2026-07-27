#ifndef GAMEFORM_H
#define GAMEFORM_H

#include <QWidget>
#include <QGraphicsScene>
#include <QVector>
#include <QTcpSocket>
#include <QTimer>
#include <QLabel>
#include "lineitem.h"
#include "chatwidget.h"
#include <QDateTime>
namespace Ui {
class GameForm;
}

class GameForm : public QWidget
{
    Q_OBJECT

public:

    ~GameForm();

    explicit GameForm(QTcpSocket *serverSocket, QColor color1 = Qt::red, QColor color2 = Qt::blue, int myPlayerId = 1, QWidget *parent = nullptr);

    void setUsername(const QString &user) { myUsername = user; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:

    void onLineClicked(int row, int col, bool isHoriz);
    void onServerMessage();
    void onTurnTimerTimeout();

private:
    Ui::GameForm *ui;
    QGraphicsScene *scene;


    QTcpSocket *socket;
    int gridSize;


    int currentPlayer;
    int player1Score;
    int player2Score;
    int myPlayerId;

    QString myUsername;
    QVector<QVector<bool>> hLines;
    QVector<QVector<bool>> vLines;
    QVector<QVector<int>> boxes;


    QVector<QVector<LineItem*>> hLineItems;
    QVector<QVector<LineItem*>> vLineItems;

    QTimer *turnTimer;
    QLabel *statusLabel;
    int turnTimeLeft;
    const int TURN_LIMIT = 20;

    ChatWidget *chatWidget;


    bool checkForCompletedBoxes(int row, int col, bool isHoriz);
    bool isProcessingNetworkMove = false;


    bool waitingForOpponent = false;
    QByteArray recvBuffer;

    void switchTurn();
    void startTurnTimer();
    void resetTurnTimer();
    void updateTimerDisplay();

    QColor p1Color;
    QColor p2Color;


};

#endif