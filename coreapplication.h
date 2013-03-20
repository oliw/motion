#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include <QObject>
#include "videoprocessor.h"
#include "evaluator.h"

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
    
signals:
    void processStatusChanged(int,bool);
    void processProgressChanged(float);
    void originalVideoLoaded(Video* video);
    
public slots:
    void loadOriginalVideo(QString path);
    void saveNewVideo(QString path);
    void calculateOriginalMotion();
    void calculateNewMotion();
    void evaluateNewMotion();
    void drawGraph();

private:
    // For Loading Video and Processing it
    VideoProcessor vp;

    // For Evaluating Video and drawing Graphs
    Evaluator ev;

    Video* originalVideo;
    Video* newVideo;


};

#endif // COREAPPLICATION_H
