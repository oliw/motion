#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include <QObject>
#include "videoprocessor.h"
#include "evaluator.h"
#include <QMap>

class CoreApplication : public QObject
{
    Q_OBJECT
public:
    explicit CoreApplication(QObject *parent = 0);

    const static int FEATURE_DETECTION = 1;
    const static int FEATURE_TRACKING = 2;
    const static int OUTLIER_REJECTION = 3;
    const static int ORIGINAL_MOTION = 4;
    const static int LOAD_VIDEO = 5;
    const static int NEW_MOTION = 6;
    const static int NEW_VIDEO = 10;
    const static int SAVE_VIDEO = 11;
    const static int ANALYSE_CROP_VIDEO = 12;
    const static int PLOTTING_MOVEMENT = 13;
    
signals:
    void processStatusChanged(int,bool);
    void processProgressChanged(float);
    void originalVideoLoaded(Video* video);
    void newVideoCreated(Video* video);
    void registerMatlabFunctionPath(QString);
    
public slots:
    // Input
    Video* loadOriginalVideo(QString path);

    // Process Video
    void calculateOriginalMotion(int radius);
    void calculateNewMotion(bool salient, bool centered);

    // Evaluate Results
    void evaluateNewMotion();
    void drawGraph(bool usePointOriginal, bool showOriginal, bool showNew, bool x, bool y);
    void setOriginalPointMotion(QMap<int, QPoint> locations);

    // Output
    void saveNewVideo(QString path);
    void saveCroppedOldVideo(QString path);
    void saveOldVideo(QString path);


    // Misc
    void saveOriginalGlobalMotionMat(QString path);
    void saveNewGlobalMotionMat(QString path);
    void loadFeatures(QString path);
    void saveOriginalFrame(QString path, int frame, bool cropped);
    void saveNewFrame(QString path, int frame);

    // Settings Slots
    void setGFTTDetector();
    void setSURFDetector();
    void setSIFTDetector();
    void setFASTDetector();
    void setGFTTHDetector();


private:
    // Objects Handled
    Video* originalVideo;
    Video* newVideo;

    int videoFourCCCodec;

    // For Loading Video and Processing it
    VideoProcessor vp;

    // For Evaluating Video and drawing Graphs
    Evaluator ev;
    QMap<int, Point2f> originalPointMotion;

    void clear();

};

#endif // COREAPPLICATION_H
