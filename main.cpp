#include "mainwindow.h"
#include "videoprocessor.h"
#include <QApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoProcessor vp;
    MainWindow w(vp);

    w.show();
    return a.exec();
}
