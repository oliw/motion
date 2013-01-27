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
    GraphDrawer gd;
    gd.moveToThread(videoProcessorThread);

    MainWindow w;

    qRegisterMetaType<Video*>("Video*");
    // Link Video Processor to Main Window
    QObject::connect(&vp, SIGNAL(videoLoaded(const Video*)),&w, SLOT(newVideoLoaded(const Video*)));
    QObject::connect(&vp, SIGNAL(processStarted(int)),&w, SLOT(processStarted(int)));
    QObject::connect(&vp, SIGNAL(processFinished(int)),&w, SLOT(processFinished(int)));
    QObject::connect(&vp, SIGNAL(progressMade(int,int)),&w, SLOT(showProgress(int,int)));
    // Link Main Window to Video Processor
    QObject::connect(&w, SIGNAL(videoChosen(QString)),&vp, SLOT(loadVideo(QString)));
    QObject::connect(&w, SIGNAL(globalMotionButtonPressed()),&vp, SLOT(calculateGlobalMotion()));
    // Link Video Processor to Player
    const Player* player = w.getPlayer();
    QObject::connect(&vp, SIGNAL(videoLoaded(const Video*)),player, SLOT(setVideo(const Video*)));
    QObject::connect(&vp, SIGNAL(videoUpdated(const Video*)),player, SLOT(setVideo(const Video*)));
    // Link Video Processor to Graph Drawer
    QObject::connect(&vp, SIGNAL(videoLoaded(const Video*)),&gd, SLOT(setVideo(const Video*)));
    // Link Main Window to Graph Drawer
    QObject::connect(&w, SIGNAL(showOriginalPath(int,int)), &gd, SLOT(drawOriginalMotionGraph(int,int)));

    videoProcessorThread->start();

    w.show();
    return a.exec();
}
