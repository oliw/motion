#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "cropwindowdialog.h"
#include <QMainWindow>
#include <QImage>
#include <QProgressBar>
#include <QCheckBox>
#include <opencv2/core/core.hpp>
#include "videoprocessor.h"
#include "manualmotiontracker.h"
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
    ManualMotionTracker* getManualMotionTracker() {return manMotionTracker;}

private slots:

    // Menu Actions
    void on_actionOpen_Video_triggered();
    void on_actionMark_Original_Movement_triggered();
    void on_actionCrop_Box_triggered();
    void on_actionSave_Result_triggered();

    // Core Application UI Slots
    void on_originalMotionButton_clicked();
    void on_newMotionButton_clicked();

    // Player UI Slots
    void on_playButton_clicked();
    void on_rewindButton_clicked();
    void on_stepButton_clicked();
    void on_showFeaturesCheckbox_stateChanged(int arg1);
    void on_showTrackedCheckbox_stateChanged(int arg1);
    void on_showOutliersCheckbox_stateChanged(int arg1);
    void on_showCropboxCheckbox_stateChanged(int arg1);
    void on_frameRate_editingFinished();

    //// Slots for VP activity
    void showProcessStatus(int processCode, bool started);
    void registerOriginalVideo(Video* video);
    void registerNewVideo(Video* video);
    void showProcessProgress(float amount);

    //// Slots for player activity
    void player_stopped();
    void updatePlayerUI(QImage img,int frameNumber);
    void on_videoCombobox_activated(const QString &arg1);

    // Slots for Actions Activity
    void manualTrackingFinished(QMap<int, QPoint> locations);


    void on_drawGraphButton_clicked();

    void on_actionMatlab_Function_triggered();

    // Slots for Process Tab
    void on_gfttRadioButton_clicked();
    void on_gftthRadioButton_clicked();
    void on_siftRadioButton_clicked();
    void on_surfRadioButton_clicked();
    void on_fastRadioButton_clicked();

signals:
   void signalResize(QResizeEvent *);
   void videoChosen(QString path);
   void originalMotionButtonPressed();
   void newMotionButtonPressed(bool salient, bool centered);
   void evaluateButtonPressed();
   void graphButtonPressed();
   void saveNewVideoButtonPressed(QString path);
   void pointsSelected(QMap<int, QPoint> locations);
   void matLabFunctionPathSelected(QString path);
   void drawGraphButtonPressed(bool usePointOriginal, bool showOriginal, bool showNew, bool x, bool y);

   // Video Process Signals
   void gfttRadioButtonPressed();
   void gftthRadioButtonPressed();
   void surfRadioButtonPressed();
   void siftRadioButtonPressed();
   void fastRadioButtonPressed();


protected:
   void resizeEvent(QResizeEvent *);  // virtual

private:
    Ui::MainWindow* ui;
    Player* player;
    QProgressBar* progress;

    // Helper Windows
    ManualMotionTracker* manMotionTracker;

    Video* originalVideo;
    Video* newVideo;

    bool playing;
    bool featuresDetected, featuresTracked, outliersRejected,originalMotion, cropBox;
    int currentFrameNumber;

    void togglePlayControls(bool show);
    void uncheckOtherPlayerButtons(QCheckBox* option);
    void toggleActionControls(bool show);

    void resetUI();

};

#endif // MAINWINDOW_H
