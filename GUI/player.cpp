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
#include "tools.h"

Player::Player(QObject *parent):QThread(parent)
{
    stopped = true;
    frameRate = 27;
    featuresEnabled = false;
    trackedEnabled = false;
    outliersEnabled = false;
    cropboxEnabled = false;
    video = NULL;
    frameNumber = 0;
}

void Player::setVideo(Video* video)
{
    this->video = video;
    refresh();
}

void Player::play()
{
    if (!isRunning()) {
        if (isStopped()){
            stopped = false;
        }
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
    const Frame* frame = video->getFrameAt(frameNumber);
    const Mat& originalData = frame->getOriginalData();
    Mat image;
    if (featuresEnabled) {
        // Draw Features
        vector<KeyPoint> features;
        KeyPoint::convert(frame->getFeatures(), features);
        cv::drawKeypoints(originalData, features, image);
    } else if (trackedEnabled && frameNumber > 0) {
        // Draw Tracked Features
        const vector<Displacement>& disps = frame->getDisplacements();
        const Frame* prevFrame = video->getFrameAt(frameNumber-1);
        const Mat& prevFrameImg = prevFrame->getOriginalData();
//        vector<Point2f> features1, features2;
//        vector<KeyPoint> featuresk1, featuresk2;
//        vector<DMatch> matches;
//        for (uint i = 0; i < disps.size(); i++) {
//            Displacement d = disps[i];
//            features1.push_back(d.getTo());
//            features2.push_back(d.getFrom());
//            matches.push_back(DMatch(i,i,i));
//        }
//        KeyPoint::convert(features1, featuresk1);
//        KeyPoint::convert(features2, featuresk2);
//        cv::drawMatches(prevFrameImg, featuresk1, originalData, featuresk2, matches, image);
        image = prevFrameImg.clone();
        for (uint i = 0; i < disps.size(); i++) {
            Displacement d = disps[i];
            cv::circle(image, d.getTo(), 1, Scalar(140,255,0));
            cv::line(image, d.getTo(), d.getFrom(), Scalar(140,255,0));
        }
    } else if (outliersEnabled) {
        // Draw outliers
        vector<KeyPoint> outliers, inliers;
        KeyPoint::convert(frame->getOutliers(), outliers);
        KeyPoint::convert(frame->getInliers(), inliers);
        assert(outliers.size() + inliers.size() == frame->getDisplacements().size());
        cv::drawKeypoints(originalData, outliers, image, Scalar(0,0,100));
        cv::drawKeypoints(image, inliers, image, Scalar(0,100,0));
    } else if (cropboxEnabled) {
        const Rect_<int>& cropBox = video->getCropBox();
        image = originalData.clone();
        if (frameNumber == 0) {
            cv::rectangle(image, cropBox, Scalar(0,255,0), 3);
        } else {
            const Mat& update = frame->getUpdateTransform();
            RotatedRect newCrop = Tools::transformRectangle(update, cropBox);
            Point2f verts[4];
            newCrop.points(verts);
            for (int i = 0; i < 4; i++) {
                line(image, verts[i], verts[(i+1)%4], Scalar(0,255,0),3);
            }
        }
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
}

void Player::msleep(int ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);
}

void Player::setFrameRate(int frameRate)
{
    this->frameRate = frameRate;
}

int Player::getFrameRate() const {
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

void Player::setCropboxEnabled(bool enabled)
{
    cropboxEnabled = enabled;
    refresh();
}



