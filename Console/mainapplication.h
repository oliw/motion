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


};

#endif // MAINAPPLICATION_H
