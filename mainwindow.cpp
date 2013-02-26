#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <QtGui>
#include <QStatusBar>
#include <QDebug>
#include <QObjectList>
#include <QProgressBar>
#include <QCheckBox>
#include <player.h>
#include "tools.h"
#include "MatToQImage.h"
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

    // Initialise Progress bar
    progress = new QProgressBar(this);
    ui->statusBar->addPermanentWidget(progress);
    progress->setTextVisible(false);
    progress->hide();

    ui->label->setText(tr("No video loaded"));
    featuresDetected = false;
    featuresTracked = false;
    outliersRejected = false;
    originalMotion = false;

    // Setup SIGNALS and SLOTS
    QObject::connect(player, SIGNAL(processedImage(QImage, int)),this, SLOT(updatePlayerUI(QImage, int)));
    QObject::connect(player, SIGNAL(playerStopped()),this, SLOT(player_stopped()));
   // QObject::connect(&cropDialog, SIGNAL(cropBoxChosen(int,int,int,int)), this, SIGNAL(cropBoxChosen(int,int,int,int)));

    // Disable Button Areas
    ui->pushButton_2->setDisabled(true);
    ui->playerControlsBox->setDisabled(true);

    ui->frameRate->setText(QString::number(player->getFrameRate()));

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

void MainWindow::newVideoLoaded(Video* video)
{
    this->video = video;
    ui->label->setText(tr("Video loaded"));
    // Set-up player
    player->setVideo(video);
    // Enable Player Controls
    ui->playerControlsBox->setEnabled(true);
    ui->checkBox->setDisabled(true);
    ui->checkBox_2->setDisabled(true);
    ui->checkBox_3->setDisabled(true);
    ui->frameCountLabel->setText(QString::number(video->getFrameCount()-1));
    // Enable 1st Processing Step Button
    ui->pushButton_2->setEnabled(true);
    // Enable Crop Box Dialog
    ui->actionCrop_Box->setEnabled(true);
}

void MainWindow::player_stopped()
{
    togglePlayControls(true);
    toggleActionControls(true);
}

void MainWindow::toggleActionControls(bool show)
{
    ui->groupBox_2->setEnabled(show);
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
        toggleActionControls(false);
    } else {
        player->stop();
    }
    togglePlayControls(!play);
    ui->playButton->setEnabled(true);
}

void MainWindow::togglePlayControls(bool show)
{
    QList<QWidget*> list = ui->playerControlsBox->findChildren<QWidget*>();
    foreach(QWidget* w, list) {
        w->setEnabled(show);
    }
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
    uncheckOtherPlayerButtons(ui->checkBox);
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
    uncheckOtherPlayerButtons(ui->checkBox_2);
    }

}

void MainWindow::on_checkBox_3_stateChanged(int checked)
{
    bool enableOutliers = checked == Qt::Checked;
    player->setOutliersEnabled(enableOutliers);
    if (enableOutliers) {
    uncheckOtherPlayerButtons(ui->checkBox_3);
    }

}

void MainWindow::processStarted(int processCode)
{
    togglePlayControls(false);
    progress->show();
    QString processMessage;
    switch (processCode){
        case VideoProcessor::FEATURE_DETECTION:
            processMessage = "Detecting Features";
            break;
        case VideoProcessor::FEATURE_TRACKING:
        processMessage = "Tracking Features";
            break;
        case VideoProcessor::OUTLIER_REJECTION:
        processMessage = "Detecting Outliers";
            break;
        case VideoProcessor::ORIGINAL_MOTION:
        processMessage = "Detecting Original Movement";
            break;
        case VideoProcessor::VIDEO_LOADING:
        processMessage = "Loading Video";
            break;
        case VideoProcessor::SAVING_VIDEO:
        processMessage = "Saving Video";
        default:
        processMessage = "Busy";
            break;
    }
    ui->statusBar->showMessage(processMessage);
}

void MainWindow::processFinished(int processCode)
{
    progress->reset();
    progress->hide();
    ui->statusBar->clearMessage();
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
        case VideoProcessor::ORIGINAL_MOTION:
            originalMotion = true;
            ui->pushButton->setEnabled(true);
            break;
        case VideoProcessor::CROP_TRANSFORM:
            ui->actionSave_Result->setEnabled(true);
            break;
        default:
            break;
    }
    togglePlayControls(true);
}

void MainWindow::showProgress(int current, int outof)
{
    progress->setMaximum(outof);
    progress->setValue(current);
}


void MainWindow::on_pushButton_clicked()
{
    emit showOriginalPath(0,0);
}

void MainWindow::on_calcStillPathButton_clicked()
{
    emit stillMotionButtonPressed();
}

void MainWindow::on_actionCrop_Box_triggered()
{
    CropWindowDialog cropDialog(video);
    cropDialog.setModal(true);
    cropDialog.exec();
}

void MainWindow::on_actionSave_Result_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save Cropped Video", "");
    emit saveResultPressed(saveFileName);
}


void MainWindow::on_checkBox_4_stateChanged(int checked)
{
    bool enableTracking = checked == Qt::Checked;
    player->setCropboxEnabled(enableTracking);
    if (enableTracking) {
        uncheckOtherPlayerButtons(ui->checkBox_4);
    }
}

void MainWindow::uncheckOtherPlayerButtons(QCheckBox* option)
{
    QList<QCheckBox*> list = ui->playerControlsBox->findChildren<QCheckBox*>();
    foreach(QCheckBox* cb, list) {
        if (cb != option) {
            cb->setChecked(!option->isChecked());
        }
    }
}

