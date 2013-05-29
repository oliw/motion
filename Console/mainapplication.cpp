#include "mainapplication.h"
#include <QString>
#include <QStringList>
#include <QStringListIterator>
#include <QTextStream>
#include <QDebug>
#include <QObject>
#include <stdio.h>
#include "coreapplication.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

MainApplication::MainApplication(QString src, QString dst, QRect cropbox, Motion::FEATUREDMETHOD fdmethod, bool salient, int window, QString salientDetails, bool gravitate, bool dumpData, QObject *parent)
    : QObject(parent),src(src),dst(dst),cropbox(cropbox), fdmethod(fdmethod), salient(salient),window(window), salientDetails(salientDetails), gravitate(gravitate),dumpData(dumpData)
{
    QObject::connect(&coreApp, SIGNAL(processProgressChanged(float)), this, SLOT(processProgressChanged(float)));
}

void MainApplication::run()
{
    switch (fdmethod) {
    case Motion::GOODTT:
        coreApp.setGFTTDetector();
        break;
    case Motion::GOODTTH:
        coreApp.setGFTTHDetector();
        break;
    case Motion::FAST:
        coreApp.setFASTDetector();
        break;
    case Motion::SIFT:
        coreApp.setSIFTDetector();
        break;
    case Motion::SURF:
        coreApp.setSURFDetector();
        break;
    default:
        coreApp.setGFTTDetector();
        break;
    }

    qWarning() << "Starting run";
    Video* video;
    qWarning() << "Loading original video";
    video = coreApp.loadOriginalVideo(src);
    video->setCropBox(cropbox.x(), cropbox.y(), cropbox.width(), cropbox.height());
    qWarning() << "Calculating motion in original video";
    coreApp.calculateOriginalMotion(window);
    if (salient) {
        qWarning() << "Loading manual markings for salient feature";
        coreApp.loadFeatures(salientDetails);
        qWarning() << "Calculating new video, preserving salient feature";
        coreApp.calculateNewMotion(salient, gravitate);
    } else {
        qWarning() << "Calculating new video";
        coreApp.calculateNewMotion(false, false);
    }
    qWarning() << "Saving new video";
    coreApp.saveNewVideo(dst);
    if (dumpData) {
        QFileInfo file(dst);
        QString directory = file.path()+"/";
        QString matPath = directory+file.baseName()+".mat";
        qWarning() << "Saving MATLAB .mat files to " << matPath;
        coreApp.saveOriginalGlobalMotionMat(matPath);
        coreApp.saveNewGlobalMotionMat(matPath);
        qWarning() << "Saving Original Cropped Video";
        QString oldCroppedVideoPath = directory+file.baseName()+"_original"+".avi";
        coreApp.saveCroppedOldVideo(oldCroppedVideoPath);
    }
    qWarning() << "Finished";
    emit quit();
}

void MainApplication::processProgressChanged(float) {

}
