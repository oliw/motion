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

MainApplication::MainApplication(QString src, QString dst, QRect cropbox, bool salient, QString salientDetails, bool gravitate, QObject *parent)
    : QObject(parent),src(src),dst(dst),cropbox(cropbox),salient(salient), salientDetails(salientDetails), gravitate(gravitate)
{
    QObject::connect(&coreApp, SIGNAL(processProgressChanged(float)), this, SLOT(processProgressChanged(float)));
}

void MainApplication::run()
{
    qWarning() << "Starting run";
    Video* video;
    qWarning() << "Loading original video";
    video = coreApp.loadOriginalVideo(src);
    video->setCropBox(cropbox.x(), cropbox.y(), cropbox.width(), cropbox.height());
    qWarning() << "Calculating motion in original video";
    coreApp.calculateOriginalMotion();
    if (salient) {
        qDebug() << "Not yet implemented salient bit";
        assert(false);
    } else {
        qWarning() << "Calculating new video";
        coreApp.calculateNewMotion(false, false);
    }
    qWarning() << "Saving new video";
    coreApp.saveNewVideo(dst);
    emit quit();
}

void MainApplication::processProgressChanged(float) {

}
