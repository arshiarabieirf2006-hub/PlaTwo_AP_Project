#ifndef GAMEFORM_H
#define GAMEFORM_H

#include <QWidget>
#include <QGraphicsScene>
#include <QVector>

namespace Ui {
class GameForm;
}

class GameForm : public QWidget
{
    Q_OBJECT

public:
    explicit GameForm(QWidget *parent = nullptr);
    ~GameForm();

private slots:

    void onLineClicked(int row, int col, bool isHoriz);

private:
    Ui::GameForm *ui;
    QGraphicsScene *scene;

    int gridSize;


    int currentPlayer;
    int player1Score;
    int player2Score;


    QVector<QVector<bool>> hLines;
    QVector<QVector<bool>> vLines;
    QVector<QVector<int>> boxes;


    bool checkForCompletedBoxes(int row, int col, bool isHoriz);
};

#endif