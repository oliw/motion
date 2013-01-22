#include "videoprocessor.h"
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <QList>
#include "frame.h"
#include "video.h"
#include "displacement.h"
#include "ransacmodel.h"
#include "tools.h"
#include <stdio.h>
#include <QDebug>
#include <QObject>
#include <vector>

using namespace std;

VideoProcessor::VideoProcessor(QObject *parent):QObject(parent) {
}

VideoProcessor::~VideoProcessor() {
}

void VideoProcessor::reset(){
    video = Video();
}

void VideoProcessor::calculateGlobalMotion() {
    detectFeatures();
    trackFeatures();
    outlierRejection();
}

void VideoProcessor::loadVideo(QString path) {
    qDebug() << "Loading video";
    cv::VideoCapture vc;
    QList<Frame> frames, originalFrames;
    bool videoOpened = vc.open(path.toStdString());
    if (!videoOpened) {
        qDebug() << "VideoProcessor::loadVideo - Video could not be opened";
        return;
    }
    videoPath = path;
    qDebug() << "VideoProcessor::loadVideo - Video opened";
    Mat buffer;
    Mat bAndWBuffer;
    while (vc.read(buffer)) {
        originalFrames.append(Frame(buffer.clone()));
        cvtColor(buffer, bAndWBuffer, CV_BGR2GRAY);
        frames.append(Frame(bAndWBuffer.clone()));
    }
    qDebug() << "VideoProcessor::loadVideo - "<< frames.size() << " Video frames grabbed";
    video = Video(frames);
    originalVideo = Video(originalFrames);
    emit videoLoaded(video);
    return;
}

void VideoProcessor::detectFeatures() {
    emit processStarted(FEATURE_DETECTION);
    qDebug() << "VideoProcessor::detectFeatures - Feature Detection started";
    int frameCount = video.getFrameCount();
    FeatureDetector* featureDetector = new GoodFeaturesToTrackDetector();
    vector<KeyPoint> bufferPoints;
    for (int i = 0; i < frameCount; i++) {
        qDebug() << "VideoProcessor::detectFeatures - Detecting features in frame " << i <<"/"<<frameCount-1;
        Frame& frame = video.accessFrameAt(i);
        featureDetector->detect(frame.getOriginalData(), bufferPoints);
        KeyPoint::convert(bufferPoints, frame.accessFeatures());
        qDebug() << "VideoProcessor::detectFeatures - Detected " << bufferPoints.size() << " features";
    }
    delete featureDetector;
    emit videoUpdated(video);
    emit processFinished(FEATURE_DETECTION);
    return;
}

void VideoProcessor::trackFeatures() {
    emit processStarted(FEATURE_TRACKING);
    qDebug() << "VideoProcessor::trackFeatures - Feature Tracking started";
    for (int i = 0; i < video.getFrameCount()-1; i++) {
        Frame& prevFrame = video.accessFrameAt(i);
        Frame& nextFrame = video.accessFrameAt(i+1);
        const vector<Point2f>& features = prevFrame.getFeatures();
        int featuresToTrack = prevFrame.getFeatures().size();
        qDebug() << "VideoProcessor::trackFeatures - Tracking " << featuresToTrack << " features from frame " << i << " to frame "<<i+1;
        vector<Point2f> nextPositions;
        vector<uchar> status;
        vector<float> err;
        // Initiate optical flow tracking
        calcOpticalFlowPyrLK(prevFrame.getOriginalData(),
                             nextFrame.getOriginalData(),
                             features,
                             nextPositions,
                             status,
                             err);
        // Remove features that were not tracked correctly
        int featuresCorrectlyTracked = 0;
        for (uint j = 0; j < features.size(); j++) {
            if (status[j] == 0) {
                // Feature could not be tracked
            } else {
                // Feature was tracked
                featuresCorrectlyTracked++;
                Displacement d = Displacement(features[j], nextPositions[j]);
                prevFrame.registerDisplacement(d);
            }
        }
        qDebug() << "VideoProcessor::trackFeatures - " << featuresCorrectlyTracked << "/" << featuresToTrack << "successfully tracked";
    }
    emit videoUpdated(video);
    emit processFinished(FEATURE_TRACKING);
    return;
}

void VideoProcessor::outlierRejection() {
    emit processStarted(OUTLIER_REJECTION);
    qDebug() << "VideoProcessor::outlierRejection - Outlier rejection started";
    outlierRejector.execute(video);
    qDebug() << "VideoProcessor::outlierRejection - Outlier rejection finished";
    emit videoUpdated(video);
    emit processFinished(OUTLIER_REJECTION);
    return;
}
