#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <QtGui>
#include <QObjectList>
#include <player.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "videoprocessor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    videoProcessor(new VideoProcessor),
    ui(new Ui::MainWindow),
    player(new Player)
{
    ui->setupUi(this);
    ui->label->setText(tr("No video loaded"));

    // Setup SIGNALS and SLOTS
    QObject::connect(player, SIGNAL(processedImage(QImage, int)),this, SLOT(updatePlayerUI(QImage, int)));

    // Disable Button Areas
    ui->pushButton_2->setDisabled(true);
    ui->pushButton_3->setDisabled(true);
    ui->pushButton_4->setDisabled(true);
    ui->playerControlsBox->setDisabled(true);

    // Simulate open video action is executed
    on_actionOpen_Video_triggered();

}

MainWindow::~MainWindow()
{
    delete ui;
    delete player;
    delete videoProcessor;
}

void MainWindow::updatePlayerUI(QImage img, int frameNumber)
{
    if (!img.isNull())
    {
        ui->label->setAlignment(Qt::AlignCenter);
        ui->label->setPixmap(QPixmap::fromImage(img).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
        ui->currentFrameLabel->setText(QString("%1").arg(frameNumber));
    }
}

void MainWindow::on_actionReset_triggered()
{

}

void MainWindow::on_stepButton_clicked()
{
    if (player->isStopped()) {
        player->step();
    }
}

void MainWindow::newVideoLoaded()
{
    ui->label->setText(tr("Video loaded"));
    // Set-up player
    player->setVideo(&videoProcessor->getVideo());
    // Enable Player Controls
    ui->playerControlsBox->setEnabled(true);
    ui->checkBox->setDisabled(true);
    ui->frameCountLabel->setText(QString("%1").arg(videoProcessor->getVideo().getFrameCount()));
    // Enable 1st Processing Step Button
    ui->pushButton_2->setEnabled(true);
}


void MainWindow::on_actionOpen_Video_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Video"), "/Users/Oli");
    QFile file(path);
    if (file.exists()) {
        ui->label->setText(tr("Loading video"));
        videoProcessor->reset();
        if (videoProcessor->loadVideo(path.toStdString())) {
            newVideoLoaded();
        }
    }
}

void MainWindow::on_playButton_clicked()
{
    bool play = player->isStopped();
    if (play) {
        player->play();
        ui->playButton->setText(tr("Stop"));
    } else {
        player->stop();
        ui->playButton->setText(tr("Start"));
    }
    QList<QWidget*> list = ui->playerControlsBox->findChildren<QWidget*>();
    foreach(QWidget* w, list) {
        if (w == ui->playButton) {

        } else {
            w->setDisabled(play);
        }
    }

}

void MainWindow::on_rewindButton_clicked()
{
    if (player->isStopped()) {
        player->rewind();
    }
}
