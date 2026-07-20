#include "lineitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>

LineItem::LineItem(int x, int y, int width, int height, bool isHoriz, int r, int c, QGraphicsItem *parent)
    : QObject(), QGraphicsRectItem(x, y, width, height, parent), 
      isClicked(false), isHorizontal(isHoriz), row(r), col(c) 
{
    
    setAcceptHoverEvents(true); 
    
    
    setPen(QPen(Qt::NoPen));    
    setBrush(Qt::NoBrush);
    
    
    setCursor(Qt::PointingHandCursor); 
}

void LineItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (!isClicked) {
        
        QPen pen(Qt::gray);
        pen.setStyle(Qt::DashLine);
        pen.setWidth(4);
        setPen(pen);
    }
    QGraphicsRectItem::hoverEnterEvent(event);
}

void LineItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isClicked) {
        
        setPen(QPen(Qt::NoPen));
    }
    QGraphicsRectItem::hoverLeaveEvent(event);
}

void LineItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (!isClicked) {
        isClicked = true;
        
       
        QPen pen(Qt::black);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(6);
        setPen(pen);
        
       
        emit lineClicked(row, col, isHorizontal);
    }
    QGraphicsRectItem::mousePressEvent(event);
}