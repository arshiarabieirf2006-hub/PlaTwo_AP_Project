#ifndef LINEITEM_H
#define LINEITEM_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

class LineItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    
    LineItem(int x, int y, int width, int height, bool isHoriz, int r, int c, QGraphicsItem *parent = nullptr);

signals:
    
    void lineClicked(int row, int col, bool isHoriz);

protected:
    
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool isClicked;
    bool isHorizontal;
    int row;
    int col;
};

#endif 