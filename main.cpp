#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <video.h>
#include <graphdrawer.h>

#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Create New Thread for Video Processor
    QThread* videoProcessorThread = new QThread();
    QObject::connect(&a, SIGNAL(aboutToQuit()),videoProcessorThread, SLOT(quit()));
    VideoProcessor vp;
    vp.moveToThread(videoProcessorThread);
    GraphDrawer gd(&vp);
    gd.moveToThread(videoProcessorThread);

    MainWindow w;

    qRegisterMetaType<Video*>("Video*");
    // Link Video Processor to Main Window
    QObject::connect(&vp, SIGNAL(videoLoaded(Video*)),&w, SLOT(newVideoLoaded(Video*)));
    QObject::connect(&vp, SIGNAL(processStarted(int)),&w, SLOT(processStarted(int)));
    QObject::connect(&vp, SIGNAL(processFinished(int)),&w, SLOT(processFinished(int)));
    QObject::connect(&vp, SIGNAL(progressMade(int,int)),&w, SLOT(showProgress(int,int)));
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

    videoProcessorThread->start();

    w.show();
    return a.exec();
}
