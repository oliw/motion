#include "mainwindow.h"
#include "evaluator.h"
#include <QApplication>
#include <QObject>
#include <video.h>
#include <graphdrawer.h>

#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Create New Thread for Non-GUI Work
    QThread* nonGuiThread = new QThread();
    QObject::connect(&a, SIGNAL(aboutToQuit()),nonGuiThread, SLOT(quit()));

    VideoProcessor vp;              // Handles Video and Processing
    vp.moveToThread(nonGuiThread);

    Evaluator evaluator(vp);        // Evaluates video and end video
    evaluator.moveToThread(nonGuiThread);

    MainWindow w;
    qRegisterMetaType<Video*>("Video*");
    // Link Video Processor to Main Window
    QObject::connect(&vp, SIGNAL(videoLoaded(Video*)),&w, SLOT(newVideoLoaded(Video*)));
    QObject::connect(&vp, SIGNAL(processStarted(int)),&w, SLOT(processStarted(int)));
    QObject::connect(&vp, SIGNAL(processFinished(int)),&w, SLOT(processFinished(int)));
    QObject::connect(&vp, SIGNAL(progressMade(int,int)),&w, SLOT(showProgress(int,int)));
    QObject::connect(&vp, SIGNAL(scoredOriginalVideo(float)),&w, SLOT(setOriginalScore(float)));
    QObject::connect(&vp, SIGNAL(scoredNewVideo(float)),&w, SLOT(setNewScore(float)));
    // Link Main Window to Video Processor
    QObject::connect(&w, SIGNAL(videoChosen(QString)),&vp, SLOT(loadVideo(QString)));
    QObject::connect(&w, SIGNAL(globalMotionButtonPressed()),&vp, SLOT(calculateGlobalMotion()));
    QObject::connect(&w, SIGNAL(stillMotionButtonPressed()),&vp, SLOT(calculateUpdateTransform()));
    QObject::connect(&w, SIGNAL(saveResultPressed(QString)),&vp, SLOT(saveCroppedVideo(QString)));
    // Link Video Processor to Player
    const Player* player = w.getPlayer();
    QObject::connect(&vp, SIGNAL(videoLoaded(Video*)),player, SLOT(setVideo(Video*)));
    QObject::connect(&vp, SIGNAL(videoUpdated(Video*)),player, SLOT(setVideo(Video*)));
    // Link Main Window to Graph Drawer
    QObject::connect(&w, SIGNAL(showOriginalPath(int,int)), &gd, SLOT(drawOriginalMotionGraph(int,int)));

    nonGuiThread->start();

    w.show();
    return a.exec();
}
