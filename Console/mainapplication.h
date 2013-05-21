#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QStringList>
#include <QTextStream>
#include <stdio.h>
#include <videoprocessor.h>

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

    // Program settings
    QString inputFilePath;
    QString outputFilePath;

    VideoProcessor vp;
    QTextStream* console;


};

#endif // MAINAPPLICATION_H
