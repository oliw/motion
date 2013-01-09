#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <opencv2/core/core.hpp>
#include "videoprocessor.h"
#include "player.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(VideoProcessor videoProcessor, QWidget *parent = 0);
    ~MainWindow();

signals:
    
private slots:

    void on_pushButton_clicked();
    void updatePlayerUI(QImage img);

    void on_pushButton_5_clicked();

private:
    VideoProcessor& videoProcessor;
    Ui::MainWindow *ui;
    Player* player;

    bool playing;
    int currentFrameNumber;
};

#endif // MAINWINDOW_H
