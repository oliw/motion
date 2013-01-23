#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QProgressBar>
#include <opencv2/core/core.hpp>
#include "videoprocessor.h"
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
    void newVideoLoaded(const Video& video);
    void showProgress(int current, int outof);

    //// Slots for player activity
    void player_stopped();
    void updatePlayerUI(QImage img,int frameNumber);

signals:
   void signalResize(QResizeEvent *);
   void videoChosen(QString path);
   void globalMotionButtonPressed();

protected:
   void resizeEvent(QResizeEvent *);  // virtual

private:
    Ui::MainWindow* ui;
    Player* player;
    QProgressBar* progress;

    bool playing;
    bool featuresDetected, featuresTracked, outliersRejected;
    int currentFrameNumber;

    void togglePlayControls(bool show);
    void toggleActionControls(bool show);

};

#endif // MAINWINDOW_H
