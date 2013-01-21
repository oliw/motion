#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <QtGui>
#include <QDebug>
#include <QObjectList>
#include <player.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "videoprocessor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    player(new Player)
{
    ui->setupUi(this);
    ui->label->setText(tr("No video loaded"));
    featuresDetected = false;
    featuresTracked = false;
    outliersRejected = false;

    // Setup SIGNALS and SLOTS
    QObject::connect(player, SIGNAL(processedImage(QImage, int)),this, SLOT(updatePlayerUI(QImage, int)));
    QObject::connect(player, SIGNAL(playerStopped()),this, SLOT(player_stopped()));

    // Disable Button Areas
    ui->pushButton_2->setDisabled(true);
    ui->pushButton_3->setDisabled(true);
    ui->pushButton_4->setDisabled(true);
    ui->playerControlsBox->setDisabled(true);

    ui->frameRate->setText(QString::number(player->getFrameRate()));

    // TODO Simulate open video action is executed
    //on_actionOpen_Video_triggered(); BREAKS EVENT LOOP

}

MainWindow::~MainWindow()
{
    delete ui;
    delete player;
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

void MainWindow::newVideoLoaded(const Video& video)
{
    ui->label->setText(tr("Video loaded"));
    // Set-up player
    player->setVideo(video);
    // Enable Player Controls
    ui->playerControlsBox->setEnabled(true);
    ui->checkBox->setDisabled(true);
    ui->checkBox_2->setDisabled(true);
    ui->checkBox_3->setDisabled(true);
    ui->frameCountLabel->setText(QString::number(video.getFrameCount()-1));
    // Enable 1st Processing Step Button
    ui->pushButton_2->setEnabled(true);
}

void MainWindow::player_stopped()
{
    togglePlayControls(true);
}


void MainWindow::on_actionOpen_Video_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Video"), "/Users/Oli");
    QFile file(path);
    if (file.exists()) {
        ui->label->setText(tr("Loading video"));
        emit videoChosen(path);
    }
}

void MainWindow::on_playButton_clicked()
{
    bool play = player->isStopped();
    if (play) {
        player->play();
    } else {
        player->stop();
    }
    togglePlayControls(false);
}

void MainWindow::togglePlayControls(bool show)
{
//    QList<QWidget*> list = ui->playerControlsBox->findChildren<QWidget*>();
//    foreach(QWidget* w, list) {
//        w->setEnabled(show);
//    }
    ui->playerControlsBox->setEnabled(show);
    bool play = player->isStopped();
    if (!play) {
        ui->playButton->setText(tr("Stop"));
    } else {
        ui->playButton->setText(tr("Start"));
    }
    ui->checkBox->setEnabled(featuresDetected);
    ui->checkBox_2->setEnabled(featuresTracked);
    ui->checkBox_3->setEnabled(outliersRejected);
}

void MainWindow::on_rewindButton_clicked()
{
    if (player->isStopped()) {
        player->rewind();
    }
}

void MainWindow::on_frameRate_editingFinished()
{
    QString fps = ui->frameRate->text().trimmed();
    fps = fps.trimmed();
    int frameRate = fps.toInt();
    if (frameRate != 0) {
        player->setFrameRate(frameRate);
    } else {
        qDebug() << "Invalid Framerate";
        frameRate = player->getFrameRate();
    }
    ui->frameRate->setText(QString::number(frameRate));
}

void MainWindow::on_checkBox_stateChanged(int checked)
{
    bool enableFeatures = checked == Qt::Checked;
    player->setFeaturesEnabled(enableFeatures);
    if (enableFeatures) {
        ui->checkBox_2->setChecked(false);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    emit globalMotionButtonPressed();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
  // call the base method
  QMainWindow::resizeEvent(e);
  player->refresh();
}


void MainWindow::on_checkBox_2_stateChanged(int checked)
{
    bool enableTracking = checked == Qt::Checked;
    player->setTrackingEnabled(enableTracking);
    if (enableTracking) {
        ui->checkBox->setChecked(false);
        ui->checkBox_3->setChecked(false);
    }
}

void MainWindow::on_checkBox_3_stateChanged(int checked)
{
    bool enableOutliers = checked == Qt::Checked;
    player->setOutliersEnabled(enableOutliers);
    if (enableOutliers) {
        ui->checkBox->setChecked(false);
        ui->checkBox_2->setChecked(false);
    }
}

void MainWindow::processStarted(int processCode)
{
    ui->playerControlsBox->setDisabled(true);
}

void MainWindow::processFinished(int processCode)
{
    switch (processCode){
        case VideoProcessor::FEATURE_DETECTION:
            featuresDetected = true;
            break;
        case VideoProcessor::FEATURE_TRACKING:
            featuresTracked = true;
            break;
        case VideoProcessor::OUTLIER_REJECTION:
            outliersRejected = true;
            break;
        default:
            break;
    }
    qDebug() << "Enabling play controls";
    togglePlayControls(true);
}
