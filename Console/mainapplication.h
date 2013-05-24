#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QStringList>
#include <QObject>
#include <QTextStream>
#include "coreapplication.h"
#include <stdio.h>
#include <videoprocessor.h>

class MainApplication : public QObject
{
    Q_OBJECT

public:
    explicit MainApplication(QString src, QString dst, QRect cropbox, bool salient, QString salientDetails, bool gravitate, QObject *parent = 0);

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
    bool salient;
    QString salientDetails;
    bool gravitate;

};

#endif // MAINAPPLICATION_H
