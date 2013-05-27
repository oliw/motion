#include "manualmotiontracker.h"
#include "ui_manualmotiontracker.h"
#include "video.h"
#include "tools.h"
#include "MatToQImage.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QMainWindow>
#include <QFileDialog>

ManualMotionTracker::ManualMotionTracker(Video* v, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualMotionTracker),
    v(v)
{
    for (int f = 0; f < v->getFrameCount(); f++) {
        Frame* frame = v->accessFrameAt(f);
        if (frame->getFeature()) {
            QPoint p(frame->getFeature()->x, frame->getFeature()->y);
            locations.insert(f, p);
        }
    }


    setFocusPolicy ( Qt::StrongFocus );
    ui->setupUi(this);
    setStepRate(1);
    setFrame(0);
}

void ManualMotionTracker::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Left) {
        previousFrame();
    } else if (e->key() == Qt::Key_Right) {
        nextFrame();
    } else if (e->key() == Qt::Key_C) {
        clearPoint();
    }
}

ManualMotionTracker::~ManualMotionTracker()
{
    delete ui;
}

void ManualMotionTracker::setStepRate(int step) {
    stepRate = step;
    ui->stepLineEdit->setText(QString::number(step));
}


void ManualMotionTracker::previousFrame() {
    if (currentFrameNumber >= stepRate) {
        setFrame(currentFrameNumber-stepRate);
    }
}

void ManualMotionTracker::nextFrame() {
    if (currentFrameNumber < v->getFrameCount()-stepRate) {
        setFrame(currentFrameNumber+stepRate);
    }
}

void ManualMotionTracker::setFrame(int frame) {
    currentFrameNumber = frame;
    ui->frameLineEdit->setText(QString::number(frame+1)+"/"+QString::number(v->getFrameCount()));

    const Mat& fr = v->getFrameAt(frame)->getOriginalData();
    const QImage& img = MatToQImage(fr);
    ui->imageFrame->setFixedSize(img.size());
    ui->imageFrame->setPixmap(QPixmap::fromImage(img).scaled(ui->imageFrame->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

    displayPoint();
}

void ManualMotionTracker::registerPoint(const QPoint& point)
{
    locations[currentFrameNumber] = point;
}

void ManualMotionTracker::on_imageFrame_pointSelected(const QPoint& point)
{
    registerPoint(point);
    displayPoint();
}

void ManualMotionTracker::on_buttonBox_accepted()
{
    if (locations.size() >= v->getFrameCount()) {
        if (locations.size() != v->getFrameCount()) {
            qWarning() << "Warning! The manual file has recordings for a greater number of frames than the number of frames grabbed by OpenCV.";
            qWarning() << "Advice: Make sure you are using OpenCV 2.4+";
        }
        for (int i = 0; i < v->getFrameCount(); i++) {
            Frame* f = v->accessFrameAt(i);
            Point2f* point = new Point2f(Tools::QPointToPoint2f(locations.value(i)));
            f->setFeature(point);
        }
        this->accept();
    }
}

void ManualMotionTracker::displayPoint() {
    if (locations.contains(currentFrameNumber)) {
        QPoint p = locations[currentFrameNumber];
        ui->coordsLineEdit->setText("("+QString::number(p.x())+","+QString::number(p.y())+")");
        ui->imageFrame->setPoint(p);
    } else {
        ui->coordsLineEdit->clear();
        ui->imageFrame->setPoint(QPoint());
    }
}

void ManualMotionTracker::clearPoint() {
    locations.remove(currentFrameNumber);
    displayPoint();
}

void ManualMotionTracker::loadMarkings() {
    QString path = QFileDialog::getOpenFileName(this, tr("Open Markings"), "/Users/Oli");
    QFile file(path);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList lineParts = line.split(",");
            int frame = lineParts.at(0).toInt();
            QPoint p(lineParts.at(1).toInt(), lineParts.at(2).toInt());
            locations.insert(frame,p);
        }
    } else {
        QMessageBox msgBox;
         msgBox.setText("Invalid Message Box");
         msgBox.exec();
    }
    displayPoint();
}

void ManualMotionTracker::exportMarkings() {
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save Markings", "");
    QFile file(saveFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QMapIterator<int, QPoint> i(locations);
    while (i.hasNext()) {
        i.next();
        QPoint p = i.value();
        out << i.key() << "," << p.x() << "," << p.y() << "\n";
    }
    file.close();
}


void ManualMotionTracker::on_pushButton_clicked()
{
    exportMarkings();
}

void ManualMotionTracker::on_pushButton_2_clicked()
{
    loadMarkings();
}

void ManualMotionTracker::on_stepLineEdit_textChanged(const QString &arg1)
{
    bool success = false;
    int step = arg1.toInt(&success);
    if (success) {
        setStepRate(step);
    } else {
        setStepRate(this->stepRate);
    }
}

void ManualMotionTracker::on_buttonBox_rejected()
{
    this->reject();
}
