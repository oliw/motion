#ifndef SELECTPOINTIMAGE_H
#define SELECTPOINTIMAGE_H

#include <QLabel>

class SelectPointImage : public QLabel
{
    Q_OBJECT
public:
    explicit SelectPointImage(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *e);
    //void mouseMoveEvent(QMouseEvent *ev);
    //void mouseReleaseEvent(QMouseEvent *ev);
    
signals:
    void pointSelected(QPoint currPoint);
    
public slots:
    void setPoint(const QPoint& point);

private:
    QPoint currPoint;
    
};

#endif // SELECTPOINTIMAGE_H
