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
#include "graphdrawer.h"
#include <stdio.h>
#include <iostream>
#include <QDebug>
#include <QObject>
#include <vector>
#include <engine.h>

using namespace std;

VideoProcessor::VideoProcessor(QObject *parent):QObject(parent) {
    QObject::connect(&outlierRejector, SIGNAL(progressMade(int, int)), this, SIGNAL(progressMade(int, int)));
}

VideoProcessor::~VideoProcessor() {
}

void VideoProcessor::reset(){
    qDebug() << "Not yet implemented";
    delete video;
}

void VideoProcessor::calculateGlobalMotion() {
    detectFeatures();
    trackFeatures();
    outlierRejection();
    calculateMotionModel();
}

void VideoProcessor::loadVideo(QString path) {
    qDebug() << "Loading video";
    cv::VideoCapture vc;
    bool videoOpened = vc.open(path.toStdString());
    if (!videoOpened) {
        qDebug() << "VideoProcessor::loadVideo - Video could not be opened";
        return;
    }
    videoPath = path;
    Mat buffer;
    Mat bAndWBuffer;
    video = new Video();
    int numFrames = vc.get(CV_CAP_PROP_FRAME_COUNT);
    int currentFrame = 0;
    while (vc.read(buffer)) {
        emit progressMade(currentFrame, numFrames-1);
        cvtColor(buffer, bAndWBuffer, CV_BGR2GRAY);
        Frame* newFrame = new Frame(bAndWBuffer.clone(),video);
        video->appendFrame(newFrame);
        currentFrame++;
    }
    emit videoLoaded(video);
    emit processFinished();
    return;
}

void VideoProcessor::detectFeatures() {
    emit processStarted(FEATURE_DETECTION);
    qDebug() << "VideoProcessor::detectFeatures - Feature Detection started";
    int frameCount = video->getFrameCount();
    FeatureDetector* featureDetector = new GoodFeaturesToTrackDetector();
    vector<KeyPoint> bufferPoints;
    for (int i = 0; i < frameCount; i++) {
        qDebug() << "VideoProcessor::detectFeatures - Detecting features in frame " << i <<"/"<<frameCount-1;
        emit progressMade(i, frameCount-1);
        Frame* frame = video->accessFrameAt(i);
        featureDetector->detect(frame->getOriginalData(), bufferPoints);
        vector<Point2f> features;
        KeyPoint::convert(bufferPoints, features);
        frame->setFeatures(features);
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
    for (int i = 0; i < video->getFrameCount()-1; i++) {
        Frame* prevFrame = video->accessFrameAt(i);
        const Frame* nextFrame = video->accessFrameAt(i+1);
        const vector<Point2f>& features = prevFrame->getFeatures();
        int featuresToTrack = prevFrame->getFeatures().size();
        qDebug() << "VideoProcessor::trackFeatures - Tracking " << featuresToTrack << " features from frame " << i << " to frame "<<i+1;
        emit progressMade(i, video->getFrameCount()-2);
        vector<Point2f> nextPositions;
        vector<uchar> status;
        vector<float> err;
        // Initiate optical flow tracking
        calcOpticalFlowPyrLK(prevFrame->getOriginalData(),
                             nextFrame->getOriginalData(),
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
                prevFrame->registerDisplacement(d);
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

void VideoProcessor::calculateMotionModel() {
    emit processStarted(ORIGINAL_MOTION);
    qDebug() << "VideoProcessor::calculateMotionModel - Calculating original motion";
    for (int i = 0; i < video->getFrameCount()-1; i++) {
        emit progressMade(i, video->getFrameCount()-2);
        Frame* frame = video->accessFrameAt(i);
        vector<Point2f> srcPoints, destPoints;
        frame->getInliers(srcPoints,destPoints);
        Mat affineTransform = estimateRigidTransform(srcPoints, destPoints, true);
        frame->setAffineTransform(affineTransform);
    }
    qDebug() << "VideoProcessor::calculateMotionModel - Original motion detected";
    emit processFinished(ORIGINAL_MOTION);
}


