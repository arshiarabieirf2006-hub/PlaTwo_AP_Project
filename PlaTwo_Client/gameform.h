#ifndef GAMEFORM_H
#define GAMEFORM_H

#include <QWidget>
#include <QGraphicsScene>
#include <QVector>
#include <QTcpSocket>
namespace Ui {
class GameForm;
}

class GameForm : public QWidget
{
    Q_OBJECT

public:

    ~GameForm();

   explicit GameForm(QTcpSocket *serverSocket, QColor color1 = Qt::red, QColor color2 = Qt::blue, QWidget *parent = nullptr);

private slots:

    void onLineClicked(int row, int col, bool isHoriz);
    void onServerMessage();

private:
    Ui::GameForm *ui;
    QGraphicsScene *scene;


    QTcpSocket *socket;
    int gridSize;


    int currentPlayer;
    int player1Score;
    int player2Score;


    QVector<QVector<bool>> hLines;
    QVector<QVector<bool>> vLines;
    QVector<QVector<int>> boxes;


    bool checkForCompletedBoxes(int row, int col, bool isHoriz);
    bool isProcessingNetworkMove = false;

    QColor p1Color;
    QColor p2Color;


};

#endif