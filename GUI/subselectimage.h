#ifndef SUBSELECTIMAGE_H
#define SUBSELECTIMAGE_H

#include <QLabel>
#include "opencv2/core/core.hpp"

class SubSelectImage : public QLabel
{
    Q_OBJECT
public:
    explicit SubSelectImage(QWidget *parent = 0);
    ~SubSelectImage();
    QRect getSelection() {return selectionRect;}


protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    bool selectionStarted;
    QRect selectionRect;
    
signals:
    void mouseHoveringOver(int x, int y);


public slots:
    
};

#endif // SUBSELECTIMAGE_H
