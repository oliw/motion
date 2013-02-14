#include "cropwindowdialog.h"
#include "ui_cropwindowdialog.h"
#include "subselectimage.h"
#include "video.h"
#include "MatToQImage.h"
#include <QDebug>
#include <QGraphicsPixmapItem>

CropWindowDialog::CropWindowDialog(Video* video, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CropWindowDialog)
{
    ui->setupUi(this);
    ui->horizontalLayout_2->addWidget(&label);
    QObject::connect(&label, SIGNAL(mouseHoveringOver(int,int)),this, SLOT(displayMouseLocation(int,int)));
    const Frame* f = video->getFrameAt(0);
    setImage(MatToQImage(f->getOriginalData()));
    this->video = video;
}

void CropWindowDialog::setImage(const QImage& img) {
    label.setFixedSize(img.size());
    label.setPixmap(QPixmap::fromImage(img).scaled(label.size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}

CropWindowDialog::~CropWindowDialog()
{
    delete ui;
}

void CropWindowDialog::displayMouseLocation(int x, int y)
{
    QString s = QString("Mouse Coords x: %1,y: %2").arg(QString::number(x),QString::number(y));
    ui->mouseCoords->setText(s);
}

void CropWindowDialog::on_buttonBox_accepted()
{
    QRect rect = label.getSelection();
    video->setCropBox(rect.x(),rect.y(),rect.width(),rect.height());
    this->accept();
}
