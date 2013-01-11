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
#include <stdio.h>
#include <QDebug>

using namespace std;

VideoProcessor::VideoProcessor() {
}

VideoProcessor::~VideoProcessor() {
}

void VideoProcessor::reset(){
    video = Video();
}

bool VideoProcessor::loadVideo(string path) {
    cv::VideoCapture vc;
    QList<Frame> frames;
    bool videoOpened = vc.open(path);
    if (!videoOpened) {
        qDebug() << "VideoProcessor::loadVideo - Video could not be opened";
        return false;
    }
    qDebug() << "VideoProcessor::loadVideo - Video opened";
    int i = 0;
    Mat buffer;
    Mat bAndWBuffer;
    while (vc.read(buffer)) {
        cvtColor(buffer, bAndWBuffer, CV_BGR2GRAY);
        frames.append(Frame(bAndWBuffer.clone()));
        i++;
    }
    qDebug() << "VideoProcessor::loadVideo - "<< frames.size() << " Video frames grabbed";
    video = Video(frames);
    return true;
}

bool VideoProcessor::detectFeatures() {
    qDebug() << "VideoProcessor::detectFeatures - Feature Detection started";
    int frameCount = video.getFrameCount();
    FeatureDetector* featureDetector = new GoodFeaturesToTrackDetector();
    vector<KeyPoint> bufferPoints;
    for (int i = 0; i < frameCount; i++) {
        qDebug() << "VideoProcessor::detectFeatures - Detecting features in frame " << i <<"/"<<frameCount-1;
        Frame& frame = video.accessFrameAt(i);
        featureDetector->detect(frame.getOriginalData(), bufferPoints);
        qDebug() << "VideoProcessor::detectFeatures - Detected " << bufferPoints.size() << " features";
        frame.setKeyPoints(bufferPoints);
    }
    delete featureDetector;
    return true;
}

bool VideoProcessor::trackFeatures() {
    qDebug() << "VideoProcessor::trackFeatures - Feature Tracking started";
    for (int i = 1; i < video.getFrameCount(); i++) {
        Frame& prevFrame = video.accessFrameAt(i-1);
        Frame& nextFrame = video.accessFrameAt(i);
        int featuresToTrack = prevFrame.getKeyPoints().size();
        qDebug() << "VideoProcessor::trackFeatures - Tracking " << featuresToTrack << " features from frame " << i-1 << " to frame "<<i;
        vector<KeyPoint> prevKeyPts = prevFrame.getKeyPoints();
        vector<Point2f> prevPts, nextPtsBuffer;
        cv::KeyPoint::convert(prevKeyPts, prevPts);
        vector<uchar> status;
        vector<float> err;
        // Initiate optical flow tracking
        calcOpticalFlowPyrLK(prevFrame.getOriginalData(),
                             nextFrame.getOriginalData(),
                             prevPts,
                             nextPtsBuffer,
                             status,
                             err);
        // Remove features that were not tracked correctly
        int featuresCorrectlyTracked = 0;
        vector<KeyPoint>::iterator keyPointIt = prevKeyPts.begin();
        vector<Point2f>::iterator pointIt = nextPtsBuffer.begin();
        vector<uchar>::iterator statusIt = status.begin();
        while (statusIt != status.end()) {
            if (*statusIt == 0) {
                statusIt = status.erase(statusIt);
                keyPointIt = prevKeyPts.erase(keyPointIt);
                pointIt = nextPtsBuffer.erase(pointIt);
            } else {
                ++statusIt;
                ++keyPointIt;
                ++pointIt;
                featuresCorrectlyTracked++;
            }
        }
        // Update Frames with new information
        prevFrame.setKeyPoints(prevKeyPts);
        nextFrame.setDetectedPointsFromPreviousFrame(nextPtsBuffer);
        qDebug() << "VideoProcessor::trackFeatures - " << featuresCorrectlyTracked << "/" << featuresToTrack << "successfully tracked";
    }
    return true;
}

