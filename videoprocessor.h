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

    void reset();
    const Video * getVideo() {QMutexLocker locker(&mutex); return video;}
    const Video * getCroppedVideo() {QMutexLocker locker(&mutex); return croppedVideo;}

    const static int FEATURE_DETECTION = 1;
    const static int FEATURE_TRACKING = 2;
    const static int OUTLIER_REJECTION = 3;
    const static int ORIGINAL_MOTION = 4;
    const static int VIDEO_LOADING = 5;
    const static int STILL_MOTION = 6;
    const static int CROP_TRANSFORM = 10;
    const static int SAVING_VIDEO = 11;
    const static int ANALYSE_CROP_VIDEO = 12;


signals:
    void videoLoaded(Video* video);
    void videoUpdated(Video* video);
    void processStarted(int processCode = -1);
    void processFinished(int processCode = -1);
    void progressMade(int current, int total);
    void scoredOriginalVideo(float score);
    void scoredNewVideo(float score);

public slots:
    void loadVideo(QString path);
    void calculateGlobalMotion();
    void detectFeatures();
    void trackFeatures();
    void outlierRejection();
    void calculateMotionModel();
    void calculateUpdateTransform();
    void applyCropTransform();
    void saveCroppedVideo(QString path);
    void scoreOriginalVideo();
    void scoreNewVideo();

private:
    mutable QMutex mutex;

    // Video Objects
    Video* video;
    Video* croppedVideo;

    // Original Video Source
    QString videoPath;

    // Processing Tools
    LocalRANSACRejector outlierRejector;

    // Processing Functions
    void detectVideoFeatures(Video* v);
    void trackVideoFeatures(Video* v);
    void removeVideoOutliers(Video* v);
    void calculateVideoMotionModel(Video* v);
    float scoreStillness(Video* v);

    static RansacModel localRansac(const std::vector<Displacement>& points);

    void analyseCroppedVideo();
    void saveVideo(const Video* videoToSave, QString path);
};

#endif // VIDEOPROCESSOR_H
