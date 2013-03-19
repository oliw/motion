#include "selectpointimage.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPoint>

SelectPointImage::SelectPointImage(QWidget *parent) :
    QLabel(parent)
{
}

void SelectPointImage::mousePressEvent(QMouseEvent *ev)
{
    currPoint = ev->pos();
    emit pointSelected(currPoint);
    QLabel::mousePressEvent(ev);
}

void SelectPointImage::setPoint(const QPoint& point) {
    currPoint = point;
    emit update();
}

void SelectPointImage::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
    if (!currPoint.isNull()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(Qt::green, 2, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(currPoint, 5, 5);
    }
}

