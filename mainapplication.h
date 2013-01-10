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
    void begin();

private:

    // Command Line Paramter functions
    bool parseOptions(QStringList options);
    void showUsage();

    QTextStream* console;

    // Program settings
    QString inputFilePath;
    QString outputFilePath;
};

#endif // MAINAPPLICATION_H
