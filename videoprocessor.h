#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include "video.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/video/video.hpp"
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
    Video& getVideo() {return video;}

    const static int FEATURE_DETECTION = 1;
    const static int FEATURE_TRACKING = 2;
    const static int OUTLIER_REJECTION = 3;

signals:
    void videoLoaded(const Video& video);
    void videoUpdated(const Video& video);
    void processStarted(int processCode);
    void processFinished(int processCode);

public slots:
    void loadVideo(QString path);
    void calculateGlobalMotion();
    void detectFeatures();
    void trackFeatures();
    void outlierRejection();

private:
    Video video;
    Video originalVideo;
    QString videoPath;

    static RansacModel localRansac(const std::vector<Displacement>& points);

};

#endif // VIDEOPROCESSOR_H
