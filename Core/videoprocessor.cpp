#include "videoprocessor.h"
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
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
#include "l1model.h"
#include "l1salientmodel.h"
#include <stdio.h>
#include <iostream>
#include <QDebug>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <vector>
#include <engine.h>

using namespace std;

VideoProcessor::VideoProcessor(QObject *parent):QObject(parent),mutex(QMutex::Recursive) {
    QObject::connect(&outlierRejector, SIGNAL(processProgressChanged(float)), this, SIGNAL(processProgressChanged(float)));
    //featureDetector = FeatureDetector::create("GFTT");
    featureDetector = Ptr<FeatureDetector>(new GoodFeaturesToTrackDetector(1000,0.01,1.,3,false,0.04));
}

VideoProcessor::~VideoProcessor() {
}

void VideoProcessor::detectFeatures(Video* v, int radius) {
    qDebug() << "VideoProcessor::detectFeatures - Feature Detection started.";
    int frameCount = v->getFrameCount();
    int numFeaturesDetected = 0;
    vector<KeyPoint> bufferPoints;
    for (int i = 0; i < frameCount; i++) {
        qDebug() << "VideoProcessor::detectFeatures - Detecting features in frame " << i <<"/"<<frameCount-1;
        emit processProgressChanged((float)i/frameCount-1);
        Frame* frame = v->accessFrameAt(i);
        const Mat& data = frame->getOriginalData();
        if (frame->getFeature() != 0 && radius > 0){
            // Build mask around salient feature
            qDebug() << "VideoProcessor::detectFeatures - Only detecting features around the salient feature";
            Point2f* point = frame->getFeature();
            cv::Mat mask = Mat::zeros(data.size(), CV_8UC1);
            for (int x = (point->x)-radius; x < (point->x)+radius; x++) {
                for (int y = (point->y)-radius; y < (point->y)+radius; y++) {
                    if (x >= 0 && x < mask.size().width && y >= 0 && y < mask.size().height) {
                        mask.at<char>(Point2f(x,y)) = 1;
                    }
                }
            }
            featureDetector->detect(data, bufferPoints,mask);
        } else {
            cv::Mat edgeMask = Mat::zeros(data.size(), CV_8UC1);
            for (int x = 2; x < v->getWidth()-2; x++) {
                for (int y = 2; y < v->getHeight()-2; y++) {
                    edgeMask.at<char>(Point2f(x,y)) = 1;
                }
            }
            featureDetector->detect(data, bufferPoints, edgeMask);
        }
        vector<Point2f> features;
        KeyPoint::convert(bufferPoints, features);
        numFeaturesDetected += features.size();
        frame->setFeatures(features);
        qDebug() << "VideoProcessor::detectFeatures - Detected " << bufferPoints.size() << " features";
    }
    float avgNumberDetected = numFeaturesDetected / (float)frameCount;
    qDebug() << "VideoProcessor::detectFeatures - Average num Features detected per frame: "<< avgNumberDetected;
}

void VideoProcessor::trackFeatures(Video* v) {
    qDebug() << "VideoProcessor::trackFeatures - Feature Tracking started";
    int avgTrackedFeatures = 0;
    for (int i = v->getFrameCount()-1; i > 0; i--) {
        Frame* frameT = v->accessFrameAt(i);
        const Frame* framePrev = v->accessFrameAt(i-1);
        const vector<Point2f>& features = frameT->getFeatures();
        //qDebug() << "VideoProcessor::trackFeatures - Tracking " << featuresToTrack << " features from frame " << i << " to frame "<<i-1;
        emit processProgressChanged(float(v->getFrameCount()-i)/v->getFrameCount());
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
                avgTrackedFeatures++;
                Displacement d = Displacement(features[j], nextPositions[j]);
                frameT->registerDisplacement(d);
            }
        }
        //qDebug() << "VideoProcessor::trackFeatures - " << featuresCorrectlyTracked << "/" << featuresToTrack << "successfully tracked";
    }
    avgTrackedFeatures /= (float) v->getFrameCount();
    qDebug() << "VideoProcessor::trackFeatures - Avg tracked" << avgTrackedFeatures;
}

void VideoProcessor::rejectOutliers(Video* v) {
    outlierRejector.execute(v);
}

void VideoProcessor::calculateMotionModel(Video* v) {
    qDebug() << "VideoProcessor::calculateMotionModel - Calculating original motion";
    for (int i = 1; i < v->getFrameCount(); i++) {
        emit processProgressChanged(float(i-1)/v->getFrameCount()-1);
        Frame* frame = v->accessFrameAt(i);
        vector<Point2f> srcPoints, destPoints;
        frame->getInliers(srcPoints,destPoints);
        // Weight towards salient point
        // Estimate Rigid Transform DOES RANSAC too!
        Mat affineTransform = estimateRigidTransform(srcPoints, destPoints, true);
        frame->setAffineTransform(affineTransform);
    }
    qDebug() << "VideoProcessor::calculateMotionModel - Original motion detected";
}

void VideoProcessor::calculateSalientUpdateTransform(Video * video, bool centered) {
    qDebug() << "VideoProcessor::calculateSalientUpdateTransform - Start";
    // Build model
    L1SalientModel model(4);
    model.prepare(video, centered);
    emit processProgressChanged(1.0f/3);
    qDebug() << "VideoProcessor::calculateSalientUpdateTransform - Solving L1 Problem";
    // Solve model
    model.solve();
    emit processProgressChanged(2.0f/3);
    // Extract Results
    for (int t = 1; t < video->getFrameCount(); t++)
    {
        Mat w = Mat::zeros(2,3,DataType<float>::type);
        for (char letter = 'a'; letter <= 'f'; letter++) {
            w.at<float>(L1Model::toRow(letter),L1Model::toCol(letter)) = model.getVariableSolution(t, letter);
        }
        Frame* f = video->accessFrameAt(t);
        Mat b;
        cv::invertAffineTransform(w, b);
        f->setUpdateTransform(b.clone());
    }
    emit processProgressChanged(1);
    qDebug() << "VideoProcessor::calculateSalientUpdateTransform - Ideal Path Calculated";
}

void VideoProcessor::calculateUpdateTransform(Video* video) {
    qDebug() << "VideoProcessor::calculateUpdateTransform - Start";
    // Build model
    L1Model model(4);
    model.prepare(video);
    emit processProgressChanged(1.0f/3);
    qDebug() << "VideoProcessor::calculateUpdateTransform - Solving L1 Problem";
    // Solve model
    model.solve();
    emit processProgressChanged(2.0f/3);
    // Extract Results
    for (int t = 1; t < video->getFrameCount(); t++)
    {
        Mat m = Mat::zeros(2,3,DataType<float>::type);
        for (char letter = 'a'; letter <= 'f'; letter++) {
            m.at<float>(L1Model::toRow(letter),L1Model::toCol(letter)) = model.getVariableSolution(t, letter);
        }
        Frame* f = video->accessFrameAt(t);
        f->setUpdateTransform(m);
    }
    emit processProgressChanged(1);
    qDebug() << "VideoProcessor::calculateUpdateTransform - Ideal Path Calculated";
}

void VideoProcessor::applyCropTransform(Video* originalVideo, Video* croppedVideo)
{
    qDebug() << "VideoProcessor::applyCropTransform() - Started";
    Rect cropWindow = originalVideo->getCropBox();
    for (int f = 0; f < originalVideo->getFrameCount(); f++) {
        emit processProgressChanged(float(f)/originalVideo->getFrameCount());
        const Frame* frame = originalVideo->getFrameAt(f);
        const Mat& img = frame->getOriginalData();
        //Move cropWindow from current position to next position using frame's update transform
        //Extract rectangle
        Mat croppedImage;
        if (f == 0) {
            croppedImage = img(cropWindow);
        } else {
            // Rotate Crop Window
            RotatedRect newCropWindow = Tools::transformRectangle(frame->getUpdateTransform(), cropWindow);
            // Crop original video using Rotated Crop Window
            croppedImage = Tools::getCroppedImage(img,newCropWindow);
            resize(croppedImage, croppedImage, originalVideo->getCropBox().size());
        }
        Frame* croppedF = new Frame(croppedImage, croppedVideo);
        croppedVideo->appendFrame(croppedF);
    }
    qDebug() << "VideoProcessor::applyCropTransform() - Finished";
}

void VideoProcessor::setGFTTDetector() {
    featureDetector = FeatureDetector::create("GFTT");
    qDebug() << "VideoProcessor - using Good Features To Track Feature Detector";
}

void VideoProcessor::setSURFDetector() {
    featureDetector = FeatureDetector::create("SURF");
    qDebug() << "VideoProcessor - using SURF Feature Detector";
}

void VideoProcessor::setSIFTDetector() {
    featureDetector = FeatureDetector::create("SIFT");
    qDebug() << "VideoProcessor - using SIFT Feature Detector";
}

void VideoProcessor::setFASTDetector() {
    featureDetector = FeatureDetector::create("FAST");
    qDebug() << "VideoProcessor - using FAST Feature Detector";
}

void VideoProcessor::setGFTTHDetector() {
    featureDetector = FeatureDetector::create("HARRIS");
    qDebug() << "VideoProcessor - using Good Features To Track (With Harris Corner Detector) Feature Detector";
}
