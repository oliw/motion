#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QStringList>
#include <QObject>
#include <QTextStream>
#include "coreapplication.h"
#include <stdio.h>
#include <videoprocessor.h>

namespace Motion {
    enum FEATUREDMETHOD {GOODTT, GOODTTH, SIFT, FAST, SURF};
}

class MainApplication : public QObject
{
    Q_OBJECT

public:
    explicit MainApplication(QString src, QString dst, QRect cropbox, Motion::FEATUREDMETHOD fdmethod, bool salient, int window, QString salientDetails, bool gravitate, bool dumpData, QObject *parent = 0);

signals:
    void quit();

public slots:
    void processProgressChanged(float);
    void run();

private:
    CoreApplication coreApp;

    QString src;
    QString dst;
    QRect cropbox;
    Motion::FEATUREDMETHOD fdmethod;
    bool salient;
    int window;
    QString salientDetails;
    bool gravitate;
    bool dumpData;

};

#endif // MAINAPPLICATION_H
