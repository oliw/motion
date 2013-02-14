#include "videoprocessor.h"
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <coin/ClpSimplex.hpp>
#include <coin/ClpModel.hpp>
#include <coin/OsiSolverInterface.hpp>
#include <coin/OsiClpSolverInterface.hpp>
#include <QList>
#include "frame.h"
#include "video.h"
#include "displacement.h"
#include "ransacmodel.h"
#include "tools.h"
#include "graphdrawer.h"
#include "l1model.h"
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
    emit processStarted(VIDEO_LOADING);
    cv::VideoCapture vc;
    bool videoOpened = vc.open(path.toStdString());
    if (!videoOpened) {
        qDebug() << "VideoProcessor::loadVideo - Video could not be opened";
        emit processFinished(VIDEO_LOADING);
        return;
    }
    videoPath = path;
    Mat buffer;
    Mat bAndWBuffer;
    int numFrames = vc.get(CV_CAP_PROP_FRAME_COUNT);
    video = new Video(numFrames);
    int currentFrame = 0;
    while (vc.read(buffer)) {
        emit progressMade(currentFrame, numFrames-1);
        cvtColor(buffer, bAndWBuffer, CV_BGR2GRAY);
        Frame* newFrame = new Frame(bAndWBuffer.clone(),video);
        video->appendFrame(newFrame);
        currentFrame++;
    }
    emit videoLoaded(video);
    emit processFinished(VIDEO_LOADING);
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
    for (int i = video->getFrameCount()-1; i > 0; i--) {
        Frame* frameT = video->accessFrameAt(i);
        const Frame* framePrev = video->accessFrameAt(i-1);
        const vector<Point2f>& features = frameT->getFeatures();
        int featuresToTrack = frameT->getFeatures().size();
        qDebug() << "VideoProcessor::trackFeatures - Tracking " << featuresToTrack << " features from frame " << i << " to frame "<<i-1;
        emit progressMade(video->getFrameCount()-i, video->getFrameCount());
        vector<Point2f> nextPositions;
        vector<uchar> status;
        vector<float> err;
        // Initiate optical flow tracking
        calcOpticalFlowPyrLK(frameT->getOriginalData(),
                             framePrev->getOriginalData(),
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
                frameT->registerDisplacement(d);
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
    for (int i = 1; i < video->getFrameCount(); i++) {
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

int getParamIndex(int frameCount, int frame, int paramCount, int parameter) {
    return frame*frameCount*paramCount*2 + parameter;
}

int getSlackIndex(int frameCount, int frame, int paramCount, int parameter) {
    return frame*frameCount*paramCount*2 + parameter*2;
}

void VideoProcessor::calculateIdealPath() {
    emit processStarted(STILL_MOTION);
    QList<Mat> stillPath;
    qDebug() << "VideoProcessor::calculateIdealPath - Start";
    OsiSolverInterface* osi = new OsiClpSolverInterface();
    // Build model
    L1Model model(osi,video);
    emit progressMade(1,3);
    // Solve model
    model.solve();
    emit progressMade(2,3);
    // Extract Original Path
    int frameCount = video->getFrameCount();
    stillPath.reserve(frameCount-1);
    for (int i = 0; i < frameCount-1 ; i++)
    {
        Mat m(2,3,DataType<float>::type);
        for (char letter = 'a'; letter <= 'e'; letter++) {
            m.at<float>(L1Model::toRow(letter),L1Model::toCol(letter)) = model.getVariableSolution(i, letter);
        }
        stillPath.push_back(m);
    }
    delete(osi);
    video->setStillPath(stillPath);
    emit progressMade(3,3);
    qDebug() << "VideoProcessor::calculateIdealPath - Ideal Path Calculated";
    emit processFinished(STILL_MOTION);
}


