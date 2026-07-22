#include "morrisnodeitem.h"

MorrisNodeItem::MorrisNodeItem(int row, int col, int x, int y, int radius, QGraphicsItem *parent)
    : QObject(), QGraphicsEllipseItem(x - radius, y - radius, radius * 2, radius * 2, parent),
    m_row(row), m_col(col), m_owner(0)
{
    setBrush(QBrush(Qt::darkGray));
    setPen(QPen(Qt::black, 2));
    setAcceptHoverEvents(true);
}

void MorrisNodeItem::setOwner(int player) {
    m_owner = player;
    if (player == 1) {
        setBrush(QBrush(Qt::red));
    } else if (player == 2) {
        setBrush(QBrush(Qt::blue));
    } else {
        setBrush(QBrush(Qt::darkGray));
    }
}

int MorrisNodeItem::getOwner() const { return m_owner; }
int MorrisNodeItem::getRow() const { return m_row; }
int MorrisNodeItem::getCol() const { return m_col; }

void MorrisNodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    emit nodeClicked(m_row, m_col);
    QGraphicsEllipseItem::mousePressEvent(event);
}