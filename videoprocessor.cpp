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
        KeyPoint::convert(bufferPoints, frame.accessFeatures());
        qDebug() << "VideoProcessor::detectFeatures - Detected " << bufferPoints.size() << " features";
    }
    delete featureDetector;
    return true;
}

bool VideoProcessor::trackFeatures() {
    qDebug() << "VideoProcessor::trackFeatures - Feature Tracking started";
    for (int i = 0; i < video.getFrameCount()-1; i++) {
        Frame& prevFrame = video.accessFrameAt(i);
        Frame& nextFrame = video.accessFrameAt(i+1);
        const vector<Point2f>& features = prevFrame.getFeatures();
        int featuresToTrack = prevFrame.getFeatures().size();
        qDebug() << "VideoProcessor::trackFeatures - Tracking " << featuresToTrack << " features from frame " << i-1 << " to frame "<<i;
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
                prevFrame.registerOpticalFlow(features[j], nextPositions[j].x - features[j].x, nextPositions[j].y - features[j].y);
            }
        }
        qDebug() << "VideoProcessor::trackFeatures - " << featuresCorrectlyTracked << "/" << featuresToTrack << "successfully tracked";
    }
    return true;
}

bool VideoProcessor::outlierRejection() {
    qDebug() << "VideoProcessor::outlierRejection - Outlier rejection started";
    // Divide each frame into grids
    Rect r(0,0,50,50);
    for (int i=0; i<video.getWidth()/50; i++)
    {
        for (int j=0; j<video.getHeight()/50; j++)
        {
           int minX = i*50;
           int maxX = ((i+1)*50 < video.getWidth()) ? (i+1)*50-1 : video.getWidth()-1;
           int minY = j*50;
           int maxY = ((j+1)*50 < video.getWidth()) ? (j+1)*50-1 : video.getHeight()-1;

        }
    }
    return true;
}

