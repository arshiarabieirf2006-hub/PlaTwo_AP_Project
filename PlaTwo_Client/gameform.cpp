#include "gameform.h"
#include "ui_gameform.h"
#include <QGraphicsEllipseItem>
#include <QBrush>

GameForm::GameForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameForm)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    int gridSize = 6;
    int dotSpacing = 50;
    int dotRadius = 8;

    QBrush blackBrush(Qt::black);

    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize; ++col) {

            int x = col * dotSpacing;
            int y = row * dotSpacing;


            scene->addEllipse(x, y, dotRadius, dotRadius, QPen(Qt::NoPen), blackBrush);
        }
    }
}

GameForm::~GameForm()
{
    delete ui;
}