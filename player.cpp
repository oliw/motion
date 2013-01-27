#include <player.h>
#include <video.h>
#include <frame.h>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "MatToQImage.h"

Player::Player(QObject *parent):QThread(parent)
{
    stopped = true;
    frameRate = 27;
    featuresEnabled = false;
    trackedEnabled = false;
    outliersEnabled = false;
    video = NULL;
}

void Player::setVideo(const Video* video)
{
    this->video = video;
    frameNumber = 0;
}

void Player::play()
{
    if (!isRunning()) {
        if (isStopped()){
            stopped = false;
        }
        qDebug() << "Player::play - playing video on a new thread";
        start(LowPriority);
    }
}

void Player::step()
{
    if (!isStopped())
        return;
    if (frameNumber >= video->getFrameCount()-1) {
        stop();
        rewind();
    } else {
        frameNumber++;
        showImage(frameNumber);
    }
}

void Player::showImage(int frameNumber)
{
    if (video->getFrameCount() == 0)
    {
        return;
    }
    qDebug() << "Player::showImage - Showing frameNumber:"<<frameNumber;
    const Frame* frame = video->getFrameAt(frameNumber);
    const Mat& originalData = frame->getOriginalData();
    Mat image;
    if (featuresEnabled) {
        // Draw Features
        vector<KeyPoint> features;
        KeyPoint::convert(frame->getFeatures(), features);
        cv::drawKeypoints(originalData, features, image);
    } else if (trackedEnabled && frameNumber < video->getFrameCount()-1) {
        // Draw Tracked Features
        const vector<Displacement>& disps = frame->getDisplacements();
        const Frame* nextFrame = video->getFrameAt(frameNumber+1);
        const Mat& nextFrameImg = nextFrame->getOriginalData();
        vector<Point2f> features1, features2;
        vector<KeyPoint> featuresk1, featuresk2;
        vector<DMatch> matches;
        for (uint i = 0; i < disps.size(); i++) {
            Displacement d = disps[i];
            features1.push_back(d.getFrom());
            features2.push_back(d.getTo());
            matches.push_back(DMatch(i,i,i));
        }
        KeyPoint::convert(features1, featuresk1);
        KeyPoint::convert(features2, featuresk2);
        cv::drawMatches(originalData, featuresk1, nextFrameImg, featuresk2, matches, image);
    } else if (outliersEnabled) {
        // Draw outliers
        vector<KeyPoint> outliers, inliers;
        KeyPoint::convert(frame->getOutliers(), outliers);
        KeyPoint::convert(frame->getInliers(), inliers);
        assert(outliers.size() + inliers.size() == frame->getDisplacements().size());
        cv::drawKeypoints(originalData, outliers, image, Scalar(0,0,100));
        cv::drawKeypoints(image, inliers, image, Scalar(0,100,0));
    } else {
        image = originalData;
    }
    emit processedImage(MatToQImage(image), frameNumber);
}

void Player::run()
{
    int delay = (1000/frameRate);
    while (!stopped) {
        if (frameNumber == video->getFrameCount()) {
            stop();
            rewind();
        }
        else {
            qDebug() << "Player::run - playing frame " << frameNumber;
            showImage(frameNumber);
            frameNumber++;
            this->msleep(delay);
        }
    }
}

void Player::refresh()
{
    if (video != NULL) {
        showImage(frameNumber);
    }
}

Player::~Player()
{
    mutex.lock();
    stopped = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}

bool Player::isStopped() const
{
    return stopped;
}

void Player::rewind()
{
    frameNumber = 0;
    refresh();
}

void Player::stop()
{
    stopped = true;
    emit playerStopped();
    qDebug() << "Player::stop - Playing stopped";
}

void Player::msleep(int ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);
}

void Player::setFrameRate(int frameRate)
{
    this->frameRate = frameRate;
}

const int Player::getFrameRate() const {
    return frameRate;
}

void Player::setFeaturesEnabled(bool enabled)
{
    featuresEnabled = enabled;
    refresh();
}

void Player::setTrackingEnabled(bool enabled)
{
    trackedEnabled = enabled;
    refresh();
}

void Player::setOutliersEnabled(bool enabled)
{
    outliersEnabled = enabled;
    refresh();
}


