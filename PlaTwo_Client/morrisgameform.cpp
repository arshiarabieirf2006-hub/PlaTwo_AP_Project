#include "morrisgameform.h"
#include "ui_morrisgameform.h"
#include <QGraphicsLineItem>
#include <QPen>
#include <QDebug>

MorrisGameForm::MorrisGameForm(QTcpSocket *socket, QColor p1Color, QColor p2Color, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MorrisGameForm),
    m_socket(socket),
    m_p1Color(p1Color),
    m_p2Color(p2Color)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    for(int i = 0; i < 7; i++)
        for(int j = 0; j < 7; j++)
            boardNodes[i][j] = nullptr;

    drawBoard();
}

MorrisGameForm::~MorrisGameForm() {
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
    qDebug() << "Clicked on Node -> Row:" << row << " Col:" << col;

    MorrisNodeItem* node = boardNodes[row][col];
    if(node && node->getOwner() == 0) {
        node->setOwner(1);
    }
}