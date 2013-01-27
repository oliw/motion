#ifndef LOCALRANSACREJECTOR_H
#define LOCALRANSACREJECTOR_H

#include <QObject>
#include "ransacmodel.h"
#include "video.h"

class LocalRANSACRejector : public QObject
{
    Q_OBJECT
public:
    explicit LocalRANSACRejector(QObject *parent = 0);
    LocalRANSACRejector(int gridSize, int localRansacTolerance, QObject *parent = 0);
    void execute(Video* video);
    
signals:
    void progressMade(int current, int total);
    
public slots:

private:
    // Settings
    int gridSize;
    int localRansacTolerance;



    RansacModel localRansac(const std::vector<Displacement>& points);
    
};

#endif // LOCALRANSACREJECTOR_H
