#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <QtGui>
#include "player.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "videoprocessor.h"

MainWindow::MainWindow(VideoProcessor vp, QWidget *parent) :
    QMainWindow(parent),
    videoProcessor(vp),
    ui(new Ui::MainWindow),
    player(new Player)
{
    ui->setupUi(this);
    ui->label->setText(tr("No video loaded"));
    QObject::connect(player, SIGNAL(processedImage(QImage)),this, SLOT(updatePlayerUI(QImage)));
    ui->pushButton_2->setDisabled(true);
    ui->pushButton_3->setDisabled(true);
    ui->pushButton_4->setDisabled(true);
    ui->pushButton_5->setDisabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete player;
}



void MainWindow::on_pushButton_clicked()
{
    QString path = QFileDialog::getOpenFileName();
    ui->label->setText(tr("Loading video"));
    videoProcessor.reset();
    videoProcessor.loadVideo(path.toStdString());
    ui->label->setText(tr("Video loaded"));
    player->setVideo(&videoProcessor.getVideo());
    ui->pushButton_5->setDisabled(false);
    Video& video = videoProcessor.getVideo();
    const cv::Mat& img = video.getImageAt(60);
    if (!img.data)
    {
        qDebug() << "No image data found in frame ";
    }
}

void MainWindow::updatePlayerUI(QImage img)
{
    if (!img.isNull())
    {
        ui->label->setAlignment(Qt::AlignCenter);
        ui->label->setPixmap(QPixmap::fromImage(img).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    if (player->isStopped()) {
        player->play();
        ui->pushButton_5->setText(tr("Stop"));
    } else {
        player->stop();
        ui->pushButton_5->setText(tr("Start"));
    }
}
