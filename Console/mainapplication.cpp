#include "mainapplication.h"
#include <QString>
#include <QStringList>
#include <QStringListIterator>
#include <QTextStream>
#include <QDebug>
#include <stdio.h>
#include "coreapplication.h"

MainApplication::MainApplication()
{
    QThread* workThread = new QThread();
    CoreApplication app;
    app.moveToThread(workThread);
}

MainApplication::~MainApplication()
{
}


void MainApplication::begin()
{

}
