#include <QCoreApplication>
#include "mainapplication.h"
#include "invalidargumentsexception.h"
#include <QDebug>
#include <QStringList>
#include <QTextStream>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    try
    {
        MainApplication main(a.arguments());
        main.begin();
    }
    catch(InvalidArgumentsException *e)
    {
        //qFatal("Invalid Arguments");
    }

    return a.exec();
}
