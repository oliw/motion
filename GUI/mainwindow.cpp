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
#include "manualmotiontracker.h"
#include "coreapplication.h"

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
    progress->setMaximum(100);
    progress->hide();

    featuresDetected = false;
    featuresTracked = false;
    outliersRejected = false;
    originalMotion = false;
    cropBox = false;

    // Setup SIGNALS and SLOTS
    QObject::connect(player, SIGNAL(processedImage(QImage, int)),this, SLOT(updatePlayerUI(QImage, int)));
    QObject::connect(player, SIGNAL(playerStopped()),this, SLOT(player_stopped()));
   // QObject::connect(&cropDialog, SIGNAL(cropBoxChosen(int,int,int,int)), this, SIGNAL(cropBoxChosen(int,int,int,int)));

    resetUI();

    // Disable Player Buttons
    ui->playerControlsBox->setDisabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete player;
    delete manMotionTracker;
}

/*
 *
 *  CoreApplication Activity
 *
 */
void MainWindow::showProcessStatus(int processCode, bool started)
{
    if (started) {
        progress->show();
        QString processMessage;
        switch (processCode){
            case CoreApplication::FEATURE_DETECTION:
                processMessage = "Detecting Features";
                break;
            case CoreApplication::FEATURE_TRACKING:
            processMessage = "Tracking Features";
                break;
            case CoreApplication::OUTLIER_REJECTION:
            processMessage = "Detecting Outliers";
                break;
            case CoreApplication::ORIGINAL_MOTION:
            processMessage = "Detecting Original Movement";
                break;
            case CoreApplication::LOAD_VIDEO:
            processMessage = "Loading Original Video";
                break;
            case CoreApplication::SAVE_VIDEO:
            processMessage = "Saving New Cropped Video";
            case CoreApplication::ANALYSE_CROP_VIDEO:
            processMessage = "Analysing Movement in Cropped Video";
        case CoreApplication::PLOTTING_MOVEMENT:
            processMessage = "Plotting Movement in Video";
            default:
            processMessage = "Busy";
                break;
        }
        ui->statusBar->showMessage(processMessage);
    } else {
        progress->reset();
        progress->hide();
        ui->statusBar->clearMessage();
        switch (processCode){
            case CoreApplication::FEATURE_DETECTION:
                featuresDetected = true;
                break;
            case CoreApplication::FEATURE_TRACKING:
                featuresTracked = true;
                break;
            case CoreApplication::OUTLIER_REJECTION:
                outliersRejected = true;
                break;
            case CoreApplication::ORIGINAL_MOTION:
                originalMotion = true;
                ui->originalMotionButton->setEnabled(false);
                ui->newMotionButton->setEnabled(true);
                ui->showOriginalGlobalMotionCheckbox->setEnabled(true);
                ui->originalMotionSourceComboBox->addItem("Global");
                ui->drawGraphButton->setEnabled(true);
                ui->exportDataToMatlabButton->setEnabled(true);
                ui->showOriginalGlobalMotionCheckbox->setEnabled(true);
                break;
            case CoreApplication::NEW_MOTION:
                cropBox = true;
                ui->actionSave_Result->setEnabled(true);
                ui->showNewGlobalMotionCheckbox->setEnabled(true);
                break;
            default:
                break;
        }
    }
    togglePlayControls(!started);
}

/*
 *
 *  Player Activity
 *
 */

/*
 *
 *  Actions
 *
 */

void MainWindow::on_actionOpen_Video_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Video"), QDir::homePath());
    QFile file(path);
    if (file.exists()) {
        emit videoChosen(path);
    }
}

void MainWindow::on_actionCrop_Box_triggered()
{
    CropWindowDialog cropDialog(originalVideo);
    cropDialog.setModal(true);
    cropDialog.exec();
}

void MainWindow::on_actionSave_Result_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save Cropped Video", "");
    emit saveNewVideoButtonPressed(saveFileName);
}

void MainWindow::on_actionMark_Original_Movement_triggered()
{
   manMotionTracker = new ManualMotionTracker(originalVideo);
   manMotionTracker->setAttribute( Qt::WA_DeleteOnClose, true );
   QObject::connect(manMotionTracker,SIGNAL(pointsSelected(QMap<int, QPoint>)),this,SLOT(manualTrackingFinished(QMap<int, QPoint>)));
   manMotionTracker->show();
}

/*
 *
 *  Player Controls UI
 *
 */
void MainWindow::on_playButton_clicked()
{
    bool isPlaying = !(player->isStopped());
    if (isPlaying) {
        player->stop();
    } else {
        player->play();
        toggleActionControls(false);
    }
    togglePlayControls(isPlaying);
    ui->playButton->setEnabled(true);
}

void MainWindow::on_rewindButton_clicked()
{
    if (player->isStopped()) {
        player->rewind();
    }
}

void MainWindow::on_stepButton_clicked()
{
    if (player->isStopped()) {
        player->step();
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

void MainWindow::on_showFeaturesCheckbox_stateChanged(int checked)
{
    bool enableFeatures = checked == Qt::Checked;
    player->setFeaturesEnabled(enableFeatures);
    if (enableFeatures) {
    uncheckOtherPlayerButtons(ui->showFeaturesCheckbox);
    }
}

void MainWindow::on_showTrackedCheckbox_stateChanged(int checked)
{
    bool enableTracking = checked == Qt::Checked;
    player->setTrackingEnabled(enableTracking);
    if (enableTracking) {
    uncheckOtherPlayerButtons(ui->showTrackedCheckbox);
    }

}

void MainWindow::on_showOutliersCheckbox_stateChanged(int checked)
{
    bool enableOutliers = checked == Qt::Checked;
    player->setOutliersEnabled(enableOutliers);
    if (enableOutliers) {
    uncheckOtherPlayerButtons(ui->showOutliersCheckbox);
    }

}

void MainWindow::on_showCropboxCheckbox_stateChanged(int checked)
{
    bool enableTracking = checked == Qt::Checked;
    player->setCropboxEnabled(enableTracking);
    if (enableTracking) {
        uncheckOtherPlayerButtons(ui->showCropboxCheckbox);
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


/*
 *
 *  Core App Controls UI
 *
 */
void MainWindow::on_newMotionButton_clicked()
{
    bool showSalient = ui->maintainSalientFeatureCheckbox->isChecked();
    bool showCenter = ui->centeredSalientFeatureCheckbox->isChecked();
    emit newMotionButtonPressed(showSalient, showCenter);
}

void MainWindow::on_originalMotionButton_clicked()
{
    emit originalMotionButtonPressed();
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

void MainWindow::registerOriginalVideo(Video* video)
{
    resetUI();

    originalVideo = video;
    featuresDetected = false;
    featuresTracked = false;
    outliersRejected = false;
    originalMotion = false;
    cropBox = false;

    // Set-up player
    player->setVideo(video);

    // Enable Player Controls
    ui->playerControlsBox->setEnabled(true);
    ui->frameCountLabel->setText(QString::number(video->getFrameCount()-1));

    // Enable 1st Processing Step Button
    ui->originalMotionButton->setEnabled(true);

    // Enable Actions
    ui->actionCrop_Box->setEnabled(true);
    ui->actionMark_Original_Movement->setEnabled(true);

    // Set Video Info
    ui->videoNameLineEdit->setText(video->getVideoName());
    ui->dimensionsLineEdit->setText(QString::number(video->getWidth())+"x"+QString::number(video->getHeight()));
    ui->frameCountLineEdit->setText(QString::number(video->getFrameCount()));

    ui->videoCombobox->clear();
    ui->videoCombobox->addItem("Original Video");
}

void MainWindow::registerNewVideo(Video* video)
{
    newVideo = video;
    ui->videoCombobox->addItem("New Video");
    ui->videoCombobox->addItem("Both");
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
    if (ui->videoCombobox->currentText() == "Original Video") {
        ui->showFeaturesCheckbox->setEnabled(featuresDetected);
        ui->showTrackedCheckbox->setEnabled(featuresTracked);
        ui->showOutliersCheckbox->setEnabled(outliersRejected);
        ui->showCropboxCheckbox->setEnabled(cropBox);
    } else {
        ui->showFeaturesCheckbox->setEnabled(false);
        ui->showTrackedCheckbox->setEnabled(false);
        ui->showOutliersCheckbox->setEnabled(false);
        ui->showCropboxCheckbox->setEnabled(false);
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
  // call the base method
  QMainWindow::resizeEvent(e);
  player->refresh();
}

void MainWindow::showProcessProgress(float amount)
{
    progress->setValue(amount*100);
}

void MainWindow::on_videoCombobox_activated(const QString &option)
{
    bool origVid = option == "Original Video";
    if (origVid) {
        player->setVideo(originalVideo);
    } else if (option == "New Video") {
        player->setVideo(newVideo);
    } else if (option == "Both") {

    }
    QList<QCheckBox*> list = ui->playerControlsBox->findChildren<QCheckBox*>();
    foreach(QCheckBox* box, list) {
        if (origVid) {
            box->setEnabled(true);
        } else {
            box->setChecked(false);
            box->setEnabled(false);
        }
    }
}

void MainWindow::on_drawGraphButton_clicked()
{
    bool x = ui->showXAxisCheckbox->isChecked();
    bool y = ui->showYAxisCheckbox->isChecked();
    bool useOriginalPoint = ui->originalMotionSourceComboBox->currentText() == "Point";
    bool showOriginal = ui->showOriginalGlobalMotionCheckbox->isChecked();
    bool showNew = ui->showNewGlobalMotionCheckbox->isChecked();
    emit drawGraphButtonPressed(useOriginalPoint,showOriginal,showNew,x,y);
}

void MainWindow::manualTrackingFinished(QMap<int, QPoint> locations) {
    emit pointsSelected(locations);
    ui->originalMotionSourceComboBox->addItem("Point");
    ui->drawGraphButton->setEnabled(true);
    ui->exportDataToMatlabButton->setEnabled(true);
    ui->showOriginalGlobalMotionCheckbox->setEnabled(true);
}


void MainWindow::resetUI()
{
    // Reset Actions
    ui->actionCrop_Box->setEnabled(false);
    ui->actionMark_Original_Movement->setEnabled(false);

    // Reset Video Tab
    QList<QLineEdit*> list = ui->videoInfoTab->findChildren<QLineEdit*>();
    foreach(QLineEdit* infoLine, list) {
         infoLine->clear();
    }
    // Reset Process Tab
    QList<QPushButton*>processList = ui->processTab->findChildren<QPushButton*>();
    foreach(QPushButton* button, processList) {
         button->setEnabled(false);
    }
    // Reset Evaluate Tab
    // TODO
    // Reset Matlab Tab
    ui->showXAxisCheckbox->setChecked(true);
    ui->showYAxisCheckbox->setChecked(true);
    ui->originalMotionSourceComboBox->clear();
    ui->showOriginalGlobalMotionCheckbox->setEnabled(false);
    ui->showNewGlobalMotionCheckbox->setEnabled(false);
    ui->drawGraphButton->setEnabled(false);
    ui->exportDataToMatlabButton->setEnabled(false);

}


void MainWindow::on_actionMatlab_Function_triggered()
{
    QString path = QFileDialog::getExistingDirectory(this,tr("Select Path For Matlab Function"), QDir::homePath());
    QFile file(path);
    if (file.exists()) {
        emit matLabFunctionPathSelected(path);
    }
}

void MainWindow::on_gfttRadioButton_clicked()
{
    emit gfttRadioButtonPressed();
}

void MainWindow::on_gftthRadioButton_clicked()
{
    emit gftthRadioButtonPressed();
}

void MainWindow::on_siftRadioButton_clicked()
{
    emit siftRadioButtonPressed();
}

void MainWindow::on_surfRadioButton_clicked()
{
    emit surfRadioButtonPressed();
}

void MainWindow::on_fastRadioButton_clicked()
{
    emit fastRadioButtonPressed();
}



void MainWindow::on_exportDataToMatlabButton_clicked()
{
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save Original Affine and Updates", "");
    emit exportDataToMatlabPressed(saveFileName);
}
