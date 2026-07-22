#ifndef MORRISGAMEFORM_H
#define MORRISGAMEFORM_H

#include <QWidget>
#include <QGraphicsScene>
#include <QTcpSocket>
#include "morrisnodeitem.h"

namespace Ui {
class MorrisGameForm;
}

class MorrisGameForm : public QWidget {
    Q_OBJECT

public:
    explicit MorrisGameForm(QTcpSocket *socket, QColor p1Color, QColor p2Color, QWidget *parent = nullptr);
    ~MorrisGameForm();

private slots:
    void onNodeClicked(int row, int col);

private:
    Ui::MorrisGameForm *ui;
    QGraphicsScene *scene;
    QTcpSocket *m_socket;
    QColor m_p1Color;
    QColor m_p2Color;

    MorrisNodeItem* boardNodes[7][7];

    void drawBoard();
};

#endif // MORRISGAMEFORM_H