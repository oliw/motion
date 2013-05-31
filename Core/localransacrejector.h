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
    LocalRANSACRejector(int gridSize, int localRansacTolerance, int newInliersThreshold, QObject *parent = 0);
    void execute(Video* video);
    
signals:
    void processProgressChanged(float);
    
public slots:

private:
    // Settings
    int gridSize;
    int localRansacTolerance;
    int newInliersThreshold;

    RansacModel localRansac(const std::vector<Displacement>& points);
    
};

#endif // LOCALRANSACREJECTOR_H
