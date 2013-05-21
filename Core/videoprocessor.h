#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include <QMutex>
#include "video.h"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/video/video.hpp"
#include "localransacrejector.h"
#include "ransacmodel.h"
#include <string>
using namespace std;

class VideoProcessor : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcessor(QObject *parent = 0);
    ~VideoProcessor();

signals:
    void processProgressChanged(float fractionComplete);

public slots:
    void applyCropTransform(Video* originalVideo, Video* croppedVideo);
    // Processing Functions
    void detectFeatures(Video* v);
    void trackFeatures(Video* v);
    void rejectOutliers(Video* v);
    void calculateMotionModel(Video* v);
    void calculateUpdateTransform(Video* v);

private:
    mutable QMutex mutex;

    // Processing Tools
    LocalRANSACRejector outlierRejector;

    //float scoreStillness(Video* v);

    //void analyseCroppedVideo();
};

#endif // VIDEOPROCESSOR_H
