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
#include "graphdrawer.h"
#include "l1model.h"
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
    QObject::connect(&outlierRejector, SIGNAL(progressMade(int, int)), this, SIGNAL(progressMade(int, int)));
    video = 0;
    croppedVideo = 0;
}

VideoProcessor::~VideoProcessor() {
}

void VideoProcessor::reset(){
    qDebug() << "Not yet implemented";
    delete video;
    delete croppedVideo;
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
    int numFrames = vc.get(CV_CAP_PROP_FRAME_COUNT);
    qDebug() << "Number of Frames:" << numFrames;
    int fps = vc.get(CV_CAP_PROP_FPS);
    video = new Video(numFrames,fps);
    int currentFrame = 0;
    while (vc.read(buffer)) {
        emit progressMade(currentFrame, numFrames-1);
        //cvtColor(buffer, bAndWBuffer, CV_BGR2GRAY);
        Frame* newFrame = new Frame(buffer.clone(),video);
        video->appendFrame(newFrame);
        currentFrame++;
    }
    scoreOriginalVideo();
    emit videoLoaded(video);
    emit processFinished(VIDEO_LOADING);
    return;
}

void VideoProcessor::detectFeatures() {
    emit processStarted(FEATURE_DETECTION);
    detectVideoFeatures(video);
    emit videoUpdated(video);
    emit processFinished(FEATURE_DETECTION);
    return;
}

void VideoProcessor::trackFeatures() {
    emit processStarted(FEATURE_TRACKING);
    trackVideoFeatures(video);
    emit videoUpdated(video);
    emit processFinished(FEATURE_TRACKING);
    return;
}

void VideoProcessor::outlierRejection() {
    emit processStarted(OUTLIER_REJECTION);
    removeVideoOutliers(video);
    emit videoUpdated(video);
    emit processFinished(OUTLIER_REJECTION);
    return;
}

void VideoProcessor::calculateMotionModel() {
    emit processStarted(ORIGINAL_MOTION);
    calculateVideoMotionModel(video);
    emit processFinished(ORIGINAL_MOTION);
}

void VideoProcessor::calculateUpdateTransform() {
    emit processStarted(STILL_MOTION);
    qDebug() << "VideoProcessor::calculateUpdateTransform - Start";
    // Build model
    L1Model model(video);
    vector<Mat> affineTransforms = video->getAffineTransforms();
    model.setDOF(4);
    model.prepare(affineTransforms, video->getCropBox(), video->getWidth(), video->getHeight());
    emit progressMade(1,3);
    qDebug() << "VideoProcessor::calculateUpdateTransform - Solving L1 Problem";
    // Solve model
    model.solve();
    emit progressMade(2,3);
    // Extract Results
    for (int t = 1; t < video->getFrameCount(); t++)
    {
        Mat m = Mat::zeros(2,3,DataType<float>::type);
        for (char letter = 'a'; letter <= 'f'; letter++) {
            assert(model.getVariableSolution(t,letter) < 1e15);
            m.at<float>(L1Model::toRow(letter),L1Model::toCol(letter)) = model.getVariableSolution(t, letter);
        }
        std::stringstream ss;
        ss << "F:"<<t<<" - "<<m;
        qDebug() << QString::fromStdString(ss.str());
        Frame* f = video->accessFrameAt(t);
        f->setUpdateTransform(m);
    }
    emit progressMade(3,3);
    qDebug() << "VideoProcessor::calculateUpdateTransform - Ideal Path Calculated";
    applyCropTransform();
    emit processFinished(STILL_MOTION);
}

void VideoProcessor::applyCropTransform()
{
    qDebug() << "VideoProcessor::applyCropTransform() - Started";
    emit processStarted(CROP_TRANSFORM);
    croppedVideo = new Video(video->getFrameCount());
    Rect cropWindow = video->getCropBox();
    for (int f = 0; f < video->getFrameCount(); f++) {
        qDebug() << "VideoProcessor::applyCropTransform() - Getting cropped version of frame:" << f;
        emit progressMade(f, video->getFrameCount());
        const Frame* frame = video->getFrameAt(f);
        const Mat& img = frame->getOriginalData();
        //Move cropWindow from current position to next position using frame's update transform
        //Extract rectangle
        Mat croppedImage;
        if (f == 0) {
            croppedImage = img(cropWindow);
        } else {
            RotatedRect newCropWindow = Tools::transformRectangle(frame->getUpdateTransform(), cropWindow);
            croppedImage = Tools::getCroppedImage(img,newCropWindow);
        }
        Frame* croppedF = new Frame(croppedImage, croppedVideo);
        croppedVideo->appendFrame(croppedF);
    }
    emit processFinished(CROP_TRANSFORM);
    scoreNewVideo();
    //Tools::trimVideo(croppedVideo);
    qDebug() << "VideoProcessor::applyCropTransform() - Finished";
}

void VideoProcessor::saveCroppedVideo(QString path)
{
    qDebug() << "VideoProcessor::saveCroppedVideo() - Started";
    saveVideo(croppedVideo, path);
    qDebug() << "VideoProcessor::saveCroppedVideo() - Finished";
}

void VideoProcessor::saveVideo(const Video* videoToSave, QString path)
{
    qDebug() << "VideoProcessor::saveVideo() - Started";
    emit processStarted(SAVING_VIDEO);
    String fp = path.toStdString();
    Size frameSize = videoToSave->getSize();
    VideoWriter record(fp, CV_FOURCC('D','I','V','X'),video->getOrigFps(), videoToSave->getSize());
    assert(record.isOpened());
    for (int f = 0; f < videoToSave->getFrameCount(); f++) {
        emit progressMade(f, videoToSave->getFrameCount());
        const Frame* frame = videoToSave->getFrameAt(f);
        const Mat& img = frame->getOriginalData();
        record << img;
    }
    emit processFinished(SAVING_VIDEO);
    qDebug() << "VideoProcessor::saveVideo() - Finished";
}

void VideoProcessor::analyseCroppedVideo() {
    assert(croppedVideo != 0);
    emit processStarted(ANALYSE_CROP_VIDEO);
    detectVideoFeatures(croppedVideo);
    trackVideoFeatures(croppedVideo);
    removeVideoOutliers(croppedVideo);
    calculateVideoMotionModel(croppedVideo);
    emit processFinished(ANALYSE_CROP_VIDEO);
}

void VideoProcessor::detectVideoFeatures(Video* v) {
    qDebug() << "VideoProcessor::detectFeatures - Feature Detection started";
    int frameCount = v->getFrameCount();
    FeatureDetector* featureDetector = new GoodFeaturesToTrackDetector();
    vector<KeyPoint> bufferPoints;
    for (int i = 0; i < frameCount; i++) {
        qDebug() << "VideoProcessor::detectFeatures - Detecting features in frame " << i <<"/"<<frameCount-1;
        emit progressMade(i, frameCount-1);
        Frame* frame = v->accessFrameAt(i);
        featureDetector->detect(frame->getOriginalData(), bufferPoints);
        vector<Point2f> features;
        KeyPoint::convert(bufferPoints, features);
        frame->setFeatures(features);
        qDebug() << "VideoProcessor::detectFeatures - Detected " << bufferPoints.size() << " features";
    }
    delete featureDetector;
}

void VideoProcessor::trackVideoFeatures(Video* v) {
    qDebug() << "VideoProcessor::trackFeatures - Feature Tracking started";
    for (int i = v->getFrameCount()-1; i > 0; i--) {
        Frame* frameT = v->accessFrameAt(i);
        const Frame* framePrev = v->accessFrameAt(i-1);
        const vector<Point2f>& features = frameT->getFeatures();
        int featuresToTrack = frameT->getFeatures().size();
        qDebug() << "VideoProcessor::trackFeatures - Tracking " << featuresToTrack << " features from frame " << i << " to frame "<<i-1;
        emit progressMade(v->getFrameCount()-i, v->getFrameCount());
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
}

void VideoProcessor::removeVideoOutliers(Video* v) {
    qDebug() << "VideoProcessor::outlierRejection - Outlier rejection started";
    outlierRejector.execute(v);
    qDebug() << "VideoProcessor::outlierRejection - Outlier rejection finished";
}

void VideoProcessor::calculateVideoMotionModel(Video* v) {
    qDebug() << "VideoProcessor::calculateMotionModel - Calculating original motion";
    for (int i = 1; i < v->getFrameCount(); i++) {
        emit progressMade(i, v->getFrameCount()-2);
        Frame* frame = v->accessFrameAt(i);
        vector<Point2f> srcPoints, destPoints;
        frame->getInliers(srcPoints,destPoints);
        Mat affineTransform = estimateRigidTransform(srcPoints, destPoints, true);
        frame->setAffineTransform(affineTransform);
    }
    qDebug() << "VideoProcessor::calculateMotionModel - Original motion detected";
}

void VideoProcessor::scoreOriginalVideo() {
    float score = scoreStillness(video);
    emit scoredOriginalVideo(score);
}

void VideoProcessor::scoreNewVideo() {
    float score = scoreStillness(croppedVideo);
    emit scoredOriginalVideo(score);
}

float VideoProcessor::scoreStillness(Video* v) {
    qDebug() << "VideoProcessor::scoreStillness - begin";
    const Mat& referenceImage = v->getFrameAt(0)->getOriginalData();
    Mat res = Mat::zeros(referenceImage.size(), CV_32FC3);

    for (int f = 0; f < frameCount; f++) {
        const Mat& frame = v->getFrameAt(f)->getOriginalData();
        Mat difference;
        absdiff(referenceImage, frame, difference);
        difference.convertTo(CV_32FC3);
        res += difference;
    }


    int p = referenceImage.size().height * referenceImage.size().width;
    int frameCount = v->getFrameCount();
    float result = 0;
    for (int ch = 0; ch < referenceImage.channels() ; ch++) {
        float averageError = 0;
        for (int x = 0; x < referenceImage.cols; x++) {
            for (int y = 0; y < referenceImage.rows; y++) {
                float avgDistance = 0;
                for (int f = 0; f < frameCount; f++) {
                    qDebug() << "VideoProcessor::scoreStillness - frame number " << f;
                    const Mat& frame = v->getFrameAt(f)->getOriginalData();
                    assert(frame.cols == referenceImage.cols && frame.rows == referenceImage.rows);
                    avgDistance += abs(frame.at<Vec3b>(y,x)[ch] - referenceImage.at<Vec3b>(y,x)[ch]);
                }
                avgDistance /= frameCount;
                averageError += avgDistance;
            }
        }
        averageError /= p;
        result += averageError;
    }
    qDebug() << "VideoProcessor::scoreStillness - end";
    return result;
}



