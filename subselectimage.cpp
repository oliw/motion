#include "subselectimage.h"
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>

SubSelectImage::SubSelectImage(QWidget *parent) :
    QLabel(parent)
{
    this->setMouseTracking(true);
}

SubSelectImage::~SubSelectImage()
{

}

void SubSelectImage::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
    QPainter painter(this);
    painter.setPen(QPen(QBrush(QColor(0,0,0,180)),1,Qt::DashLine));
    painter.drawRect(selectionRect);
}

void SubSelectImage::mousePressEvent(QMouseEvent *e)
{
    selectionStarted=true;
    selectionRect.setTopLeft(e->pos());
    selectionRect.setBottomRight(e->pos());
}

void SubSelectImage::mouseMoveEvent(QMouseEvent *e)
{
    emit mouseHoveringOver(e->pos().x(), e->pos().y());
    bool valid = e->pos().x() >= selectionRect.topLeft().x() && e->pos().y() >= selectionRect.topLeft().y();
    if (selectionStarted && valid)
    {
        selectionRect.setBottomRight(e->pos());
        repaint();
    }
}

void SubSelectImage::mouseReleaseEvent(QMouseEvent *e)
{
    selectionStarted=false;
}
