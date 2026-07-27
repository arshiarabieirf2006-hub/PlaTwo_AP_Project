#ifndef MORRISGAMEFORM_H
#define MORRISGAMEFORM_H

#include <QWidget>
#include <QGraphicsScene>
#include <QTcpSocket>
#include <QTimer>
#include <QLabel>
#include "morrisnodeitem.h"
#include "chatwidget.h"

namespace Ui {
class MorrisGameForm;
}

class MorrisGameForm : public QWidget {
    Q_OBJECT

public:
    explicit MorrisGameForm(QTcpSocket *socket, QColor p1Color, QColor p2Color, int myPlayerId = 1, QWidget *parent = nullptr);
    void setMyPlayerId(int id) { myPlayerId = id; updateTimerDisplay(); }
    ~MorrisGameForm();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onNodeClicked(int row, int col);
    void onTurnTimerTimeout();
    void onReadyRead();

private:

    Ui::MorrisGameForm *ui;
    QGraphicsScene *scene;
    QTcpSocket *m_socket;
    QColor m_p1Color;
    QColor m_p2Color;

    QTimer *turnTimer;
    QLabel *statusLabel;
    int turnTimeLeft;
    const int TURN_LIMIT = 20;

    ChatWidget *chatWidget;

    MorrisNodeItem* boardNodes[7][7];
    int myPlayerId;
    int gameState;
    int currentPlayer;
    int p1PlacedCount;
    int p2PlacedCount;
    int p1RemainingCount;
    int p2RemainingCount;
    bool isRemovingPhase;
    bool isGameOver;
    MorrisNodeItem* selectedNode;

    bool waitingForOpponent = false;

    void drawBoard();
    bool checkMill(int row, int col, int player);
    bool isPieceInAnyMill(int row, int col, int player);
    bool allOpponentPiecesInMills(int opponent);
    bool areAdjacent(int r1, int c1, int r2, int c2);
    void switchTurn();
    void checkGameOver();
    void startTurnTimer();
    void resetTurnTimer();
    void updateTimerDisplay();
    void sendNetworkMessage(const QString &msg);
    void processNetworkMessage(const QString &msg);
    void applyPlace(int r, int c, int player);
    void applyMove(int r1, int c1, int r2, int c2, int player);
    void applyRemove(int r, int c);
};

#endif // MORRISGAMEFORM_H