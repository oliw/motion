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

ManualMotionTracker::ManualMotionTracker(const Video* v, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualMotionTracker),
    v(v)
{
    setFocusPolicy ( Qt::StrongFocus );
    ui->setupUi(this);
    point = new Point2f[v->getFrameCount()];
    for (int i = 0; i < v->getFrameCount(); i++) {
        point[i] = Point2f(-1,-1);
    }
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
    delete point;
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
    Point2f newPoint = Tools::QPointToPoint2f(point);
    this->point[currentFrameNumber] = newPoint;
}

void ManualMotionTracker::on_imageFrame_pointSelected(const QPoint& point)
{
    registerPoint(point);
    displayPoint();
}

void ManualMotionTracker::on_buttonBox_accepted()
{
    this->accept();
}

void ManualMotionTracker::displayPoint() {
    if (point[currentFrameNumber].x >= 0) {
        Point2f currPoint = point[currentFrameNumber];
        QPoint p = QPoint(currPoint.x, currPoint.y);
        ui->coordsLineEdit->setText("("+QString::number(p.x())+","+QString::number(p.y())+")");
        ui->imageFrame->setPoint(p);
    } else {
        ui->coordsLineEdit->clear();
        ui->imageFrame->setPoint(QPoint());
    }
}

void ManualMotionTracker::clearPoint() {
    Point2f nullPoint;
    nullPoint.x = -1;
    nullPoint.y = -1;
    point[currentFrameNumber] = nullPoint;
    displayPoint();
}

void ManualMotionTracker::loadMarkings() {
    QString path = QFileDialog::getOpenFileName(this, tr("Open Markings"), "/Users/Oli");
    QFile file(path);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        int i = 0;
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList coords = line.split(",");
            Point2f point;
            point.x = coords.at(0).toFloat();
            point.y = coords.at(1).toFloat();
            this->point[i] = point;
            i++;
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
    for (int i = 0; i < v->getFrameCount(); i++) {
        out << point[i].x << "," << point[i].y << "\n";
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
