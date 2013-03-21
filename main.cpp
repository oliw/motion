#include "mainwindow.h"
#include "evaluator.h"
#include <QApplication>
#include <QObject>
#include <video.h>
#include <coreapplication.h>

#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QThread* nonGuiThread = new QThread();
    QObject::connect(&a, SIGNAL(aboutToQuit()),nonGuiThread, SLOT(quit()));

    CoreApplication app;
    app.moveToThread(nonGuiThread);

    MainWindow w;

    // Connect GUI events to CoreApp
    QObject::connect(&w, SIGNAL(videoChosen(QString)),&app, SLOT(loadOriginalVideo(QString)));
    QObject::connect(&w, SIGNAL(saveNewVideoButtonPressed(QString)),&app, SLOT(saveNewVideo(QString)));

    QObject::connect(&w, SIGNAL(originalMotionButtonPressed()),&app, SLOT(calculateOriginalMotion()));
    QObject::connect(&w, SIGNAL(newMotionButtonPressed()),&app, SLOT(calculateNewMotion()));
    QObject::connect(&w, SIGNAL(evaluateButtonPressed()),&app, SLOT(evaluateNewMotion()));
    QObject::connect(&w, SIGNAL(graphButtonPressed()),&app, SLOT(drawGraph()));

    typedef QMap<int, QPoint> LocationMap;
    qRegisterMetaType<LocationMap>("QMap<int, QPoint>");
    QObject::connect(&w, SIGNAL(pointsSelected(QMap<int, QPoint>)),&app, SLOT(registerOriginalPointLocations(QMap<int, QPoint>)));

    // Connect CoreApp events to GUI
    QObject::connect(&app, SIGNAL(originalVideoLoaded(Video*)),&w, SLOT(registerOriginalVideo(Video*)));
    QObject::connect(&app, SIGNAL(newVideoCreated(Video*)),&w, SLOT(registerNewVideo(Video*)));
    QObject::connect(&app, SIGNAL(processStatusChanged(int,bool)),&w, SLOT(showProcessStatus(int,bool)));
    QObject::connect(&app, SIGNAL(processProgressChanged(float)),&w, SLOT(showProcessProgress(float)));

    qRegisterMetaType<Video*>("Video*");
    // Link Video Processor to Main Window
    // QObject::connect(&vp, SIGNAL(videoLoaded(Video*)),&w, SLOT(newVideoLoaded(Video*)));
    // QObject::connect(&vp, SIGNAL(processStarted(int)),&w, SLOT(processStarted(int)));
    // QObject::connect(&vp, SIGNAL(processFinished(int)),&w, SLOT(processFinished(int)));
    // QObject::connect(&vp, SIGNAL(progressMade(int,int)),&w, SLOT(showProgress(int,int)));
    // QObject::connect(&vp, SIGNAL(scoredOriginalVideo(float)),&w, SLOT(setOriginalScore(float)));
    // QObject::connect(&vp, SIGNAL(scoredNewVideo(float)),&w, SLOT(setNewScore(float)));
    // Link Main Window to Video Processor
    // QObject::connect(&w, SIGNAL(stillMotionButtonPressed()),&vp, SLOT(calculateUpdateTransform()));
    // QObject::connect(&w, SIGNAL(saveResultPressed(QString)),&vp, SLOT(saveCroppedVideo(QString)));
    // Link Video Processor to Player
    // const Player* player = w.getPlayer();
    // QObject::connect(&vp, SIGNAL(videoLoaded(Video*)),player, SLOT(setVideo(Video*)));
    // QObject::connect(&vp, SIGNAL(videoUpdated(Video*)),player, SLOT(setVideo(Video*)));
    // Link Main Window to Graph Drawer
    // QObject::connect(&w, SIGNAL(showOriginalPath(int,int)), &gd, SLOT(drawOriginalMotionGraph(int,int)));

    nonGuiThread->start();

    w.show();
    return a.exec();
}
