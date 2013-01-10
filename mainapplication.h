#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QStringList>
#include <QTextStream>
#include <stdio.h>

class MainApplication
{
public:
    explicit MainApplication(QStringList options);
    ~MainApplication();


private:
    bool parseOptions(QStringList options);

    QTextStream* console;

    // Program settings
    QString inputFilePath;
    QString outputFilePath;
};

#endif // MAINAPPLICATION_H
