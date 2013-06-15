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

/*
 *
 *  This is the main object that performs the motion stabilisation
 *  method
 *
 *
 */
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
    void detectFeatures(Video* v, int radius);

    void trackFeatures(Video* v);
    void rejectOutliers(Video* v);

    // Estimates and sets the affine transformation for each frame pair
    // Pre: Frames contain a list of tracked points
    // Post: Frame's affine transformation Mat is set
    void calculateMotionModel(Video* v);


    // Calculates the update transforms needed to reduce movement in the new frame
    // Pre: Frames have affine transformation's calculated
    void calculateUpdateTransform(Video* v);

    // Calculates the update transform ensuring the salient point stays in the frame
    // Pre: The crop box contains the salient feature, the salient feature has been manually marked in each frame
    void calculateSalientUpdateTransform(Video * video, bool centered);

    ////////// SETTINGS //////////
    void setGFTTDetector();
    void setSURFDetector();
    void setSIFTDetector();
    void setFASTDetector();
    void setGFTTHDetector();


private:
    mutable QMutex mutex;

    Ptr<FeatureDetector> featureDetector;
    LocalRANSACRejector outlierRejector;
};

#endif // VIDEOPROCESSOR_H
