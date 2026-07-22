#ifndef MORRISNODEITEM_H
#define MORRISNODEITEM_H

#include <QGraphicsEllipseItem>
#include <QObject>
#include <QBrush>
#include <QPen>

class MorrisNodeItem : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
public:
    MorrisNodeItem(int row, int col, int x, int y, int radius, QGraphicsItem *parent = nullptr);

    void setOwner(int player);
    int getOwner() const;
    int getRow() const;
    int getCol() const;

signals:
    void nodeClicked(int row, int col);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int m_row;
    int m_col;
    int m_owner;
};

#endif // MORRISNODEITEM_H