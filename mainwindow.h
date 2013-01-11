#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
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

signals:
    
private slots:

    void updatePlayerUI(QImage img,int frameNumber);

    void on_actionReset_triggered();

    void on_stepButton_clicked();

    void on_actionOpen_Video_triggered();

    void on_playButton_clicked();

    void on_rewindButton_clicked();

private:
    VideoProcessor* videoProcessor;
    Ui::MainWindow* ui;
    Player* player;

    bool playing;
    int currentFrameNumber;

    void newVideoLoaded();

};

#endif // MAINWINDOW_H
