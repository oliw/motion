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
    QObject::connect(&w, SIGNAL(newSalientMotionButtonPressed()),&app, SLOT(calculateSalientUpdateTransform()));
    QObject::connect(&w, SIGNAL(evaluateButtonPressed()),&app, SLOT(evaluateNewMotion()));
    QObject::connect(&w, SIGNAL(drawGraphButtonPressed(bool, bool, bool, bool, bool)),&app, SLOT(drawGraph(bool, bool, bool, bool, bool)));
    QObject::connect(&w, SIGNAL(matLabFunctionPathSelected(QString)), &app, SIGNAL(registerMatlabFunctionPath(QString)));
    QObject::connect(&w, SIGNAL(gfttRadioButtonPressed()), &app, SLOT(setGFTTDetector()));
    QObject::connect(&w, SIGNAL(gftthRadioButtonPressed()), &app, SLOT(setGFTTHDetector()));
    QObject::connect(&w, SIGNAL(surfRadioButtonPressed()), &app, SLOT(setSURFDetector()));
    QObject::connect(&w, SIGNAL(siftRadioButtonPressed()), &app, SLOT(setSIFTDetector()));
    QObject::connect(&w, SIGNAL(fastRadioButtonPressed()), &app, SLOT(setFASTDetector()));


    typedef QMap<int, QPoint> LocationMap;
    qRegisterMetaType<LocationMap>("QMap<int, QPoint>");
    qRegisterMetaType<Video*>("Video*");
    QObject::connect(&w, SIGNAL(pointsSelected(QMap<int, QPoint>)),&app, SLOT(setOriginalPointMotion(QMap<int, QPoint>)));

    // Connect CoreApp events to GUI
    QObject::connect(&app, SIGNAL(originalVideoLoaded(Video*)),&w, SLOT(registerOriginalVideo(Video*)));
    QObject::connect(&app, SIGNAL(newVideoCreated(Video*)),&w, SLOT(registerNewVideo(Video*)));
    QObject::connect(&app, SIGNAL(processStatusChanged(int,bool)),&w, SLOT(showProcessStatus(int,bool)));
    QObject::connect(&app, SIGNAL(processProgressChanged(float)),&w, SLOT(showProcessProgress(float)));

    nonGuiThread->start();

    w.show();
    return a.exec();
}
