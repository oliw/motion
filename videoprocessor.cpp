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
    int gridSize = 50;
    // Divide each frame into grids
    for (int f=0; f<video.getFrameCount()-1; f++) {
        Frame& frame = video.accessFrameAt(f);
        uint actualNum = frame.getDisplacements().size();
        uint numDisplacements = 0;
        for (int x=0; x < video.getWidth(); x+=gridSize)
        {
            for (int y=0; y < video.getHeight(); y+=gridSize)
            {
                //qDebug() << "VideoProcessor::outlierRejection - Detecting outliers in grid at "<<x<<","<<y;
                vector<Displacement> displacements = frame.getDisplacements(x,y, gridSize);
                numDisplacements += displacements.size();
                //qDebug() << "VideoProcessor::outlierRejection - Feature count in grid: "<< displacements.size();
                if (!displacements.empty()) {
                    RansacModel model = localRansac(displacements);
                    //qDebug() << "VideoProcessor::outlierRejection - Outliers:" << model.getOutliers().size() << " Inliers:" << model.getInliers().size();
                    frame.registerOutliers(model.getOutliers());
                }
            }
        }
        assert(actualNum == numDisplacements);
    }
    qDebug() << "VideoProcessor::outlierRejection - Outlier rejection finished";
    emit videoUpdated(video);
    emit processFinished(OUTLIER_REJECTION);
    return;
}

Point2f fitTranslationModel(std::vector<Displacement> points) {
    // Option 1
    Point2f avgDisp = Point2f(0,0);
    for (uint r = 0; r < points.size(); r++) {
        avgDisp += points.at(r).getDisplacement();
    }
    return avgDisp = avgDisp * (1.0 / points.size());
}

RansacModel VideoProcessor::localRansac(const std::vector<Displacement>& points) {

    float tolerance = 2; // Patent says <2 pixels
    std::vector<RansacModel> models;
    for (int i = 0; i < 3; i++)
    {
        // Step 1: Select a random number of points as inliers
        std::vector<Displacement> outliers = points;
        std::vector<Displacement> inliers;
        uint numInitialInliers = (rand() % outliers.size()) + 1;
        assert(points.size() >= numInitialInliers);
        for (uint r = 0; r < numInitialInliers; r++) {
            int randomIndex = rand() % outliers.size();
            Displacement randomPoint = outliers.at(randomIndex);
            outliers.erase(outliers.begin()+randomIndex);
            inliers.push_back(randomPoint);
        }
        /*  Step 2: Fit a displacement model to these points
         *          OPTION 1: Fit a translation model
         *          OPTION 2: Fit a fundamental matrix
         *          OPTION 3: Fit a affine model
         */
        Point2f model = fitTranslationModel(inliers);
        // Step 3: Compare each of remaining points with the new model
        // Step 4: If a point lies within a tolerance make it an inlier also
        int newInliers = 0;
        int newInliersThreshold;
        vector<Displacement>::iterator it = outliers.begin();
        while (it != outliers.end()) {
            const Displacement& disp = *it;
            Point2f estimate = disp.getFrom() + model;
            if (Tools::eucDistance(estimate, disp.getTo()) <= tolerance) {
                // Inlier
                inliers.push_back(disp);
                it = outliers.erase(it);
                newInliers++;
            } else {
                // Outlier
                it++;
            }
        }
        // Step 5: If num of inliers is above a certain threshold, fit a new model
        if (newInliers <= newInliersThreshold)
        {
            model = fitTranslationModel(inliers);
        }
        RansacModel m = RansacModel(model, outliers, inliers);
        models.push_back(m);
        // Step 6: Repeat steps 1-5 i times
    }
    // Step 7: Keep the saved model which has the lowest error score with the inliers
    RansacModel finalResult = models.at(0);
    float finalError = finalResult.getInlierError();
    for (uint i = 1; i < models.size(); i++) {
        float currError = models[i].getInlierError();
        if (currError < finalError)
        {
            finalResult = models[i];
            finalError = currError;
        }
    }
    // Step 8: Done
    return finalResult;
}
