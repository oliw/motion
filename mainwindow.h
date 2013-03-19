#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "cropwindowdialog.h"
#include <QMainWindow>
#include <QImage>
#include <QProgressBar>
#include <QCheckBox>
#include <opencv2/core/core.hpp>
#include "videoprocessor.h"
#include "graphdrawer.h"
#include <video.h>
#include <player.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Player* getPlayer() {return player;}

signals:
    
private slots:

    void on_actionReset_triggered();

    void on_actionOpen_Video_triggered();

    // VP UI Slots
    void on_pushButton_2_clicked();

    // Player UI Slots
    void on_playButton_clicked();
    void on_rewindButton_clicked();
    void on_stepButton_clicked();
    void on_checkBox_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);
    void on_checkBox_3_stateChanged(int arg1);
    void on_frameRate_editingFinished();

    //// Slots for VP activity
    // When the processor has finished working on something
    void processStarted(int processCode);
    void processFinished(int processCode);
    void newVideoLoaded(Video* video);
    void showProgress(int current, int outof);
    void setOriginalScore(float score);
    void setNewScore(float score);

    //// Slots for player activity
    void player_stopped();
    void updatePlayerUI(QImage img,int frameNumber);
    void on_calcStillPathButton_clicked();
    void on_actionCrop_Box_triggered();
    void on_actionSave_Result_triggered();
    void on_checkBox_4_stateChanged(int arg1);

    void on_actionMark_Original_Movement_triggered();

    void on_loadVideoButton_clicked();

signals:
   void signalResize(QResizeEvent *);
   void videoChosen(QString path);
   void globalMotionButtonPressed();
   void stillMotionButtonPressed();
   void showOriginalPath(int x, int y);
   void saveResultPressed(QString path);

protected:
   void resizeEvent(QResizeEvent *);  // virtual

private:
    Ui::MainWindow* ui;
    Player* player;
    QProgressBar* progress;

    Video* video;

    bool playing;
    bool featuresDetected, featuresTracked, outliersRejected,originalMotion, cropBox;
    int currentFrameNumber;

    void togglePlayControls(bool show);
    void uncheckOtherPlayerButtons(QCheckBox* option);
    void toggleActionControls(bool show);

};

#endif // MAINWINDOW_H
