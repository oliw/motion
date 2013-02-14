#ifndef CROPWINDOWDIALOG_H
#define CROPWINDOWDIALOG_H

#include <QDialog>
#include <QtGui>
#include <QGraphicsScene>
#include <QRubberBand>
#include "subselectimage.h"
#include "video.h"

namespace Ui {
class CropWindowDialog;
}

class CropWindowDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CropWindowDialog(Video* video, QWidget *parent = 0);
    ~CropWindowDialog();

    void setImage(const QImage& i);
    
private:
    Ui::CropWindowDialog *ui;
    Video* video;
    SubSelectImage label;

signals:
    void cropBoxChosen(int,int,int,int);

public slots:
    void displayMouseLocation(int x, int y);
private slots:
    void on_buttonBox_accepted();
};

#endif // CROPWINDOWDIALOG_H
