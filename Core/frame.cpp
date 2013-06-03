#include "frame.h"
#include "displacement.h"
#include <opencv2/core/core.hpp>
#include <QDebug>

Frame::Frame(QObject *parent):QObject(parent),mutex()
{
    feature = 0;
}

Frame::Frame(const Mat& originalData,QObject *parent):QObject(parent),mutex()
{
    originalData.copyTo(image);
    reset();
    feature = 0;
}

void Frame::reset()
{
    features.clear();
    displacements.clear();
    dx = Mat::zeros(image.rows, image.cols, DataType<float>::type);
    dy = Mat::zeros(image.rows, image.cols, DataType<float>::type);
    displacementMask = Mat::zeros(image.rows, image.cols, DataType<int>::type);
    outlierMask = Mat::zeros(image.rows, image.cols, DataType<int>::type);
    affine = Mat::zeros(2,3,DataType<float>::type);
    update = Mat::zeros(2,3,DataType<float>::type);
}


void Frame::registerDisplacement(const Displacement& displacement) {
    QMutexLocker locker(&mutex);
    const Point2f& feature = displacement.getFrom();
    assert(displacementMask.at<int>(feature) == 0);
    displacementMask.at<int>(feature) = 1;
    dx.at<float>(feature) = displacement.getDisplacement().x;
    dy.at<float>(feature) = displacement.getDisplacement().y;
    displacements.push_back(displacement);
    assert(((uint) cv::countNonZero(displacementMask)) == displacements.size());
}

const vector<Displacement>& Frame::getDisplacements() const {
    QMutexLocker locker(&mutex);
    assert( ((uint) cv::countNonZero(displacementMask)) == displacements.size());
    return displacements;
}

vector<Displacement> Frame::getDisplacements(int ox, int oy, int gridSize) const {
    QMutexLocker locker(&mutex);
    vector<Displacement> displacements;
    Size size = image.size();
    for (int x = ox; x < ox+gridSize && x < size.width; x++) {
        for (int y = oy; y < oy+gridSize && y < size.height; y++) {
            Point2f from = Point2f(x,y);
            if (displacementMask.at<int>(from) == 1) {
                assert(dy.size().width >= x && dy.size().height >= y);
                Point2f to = from + Point2f(dx.at<float>(from), dy.at<float>(from));
                displacements.push_back(Displacement(from, to));
            }
        }
    }
    return displacements;
}

void Frame::registerOutliers(const vector<Displacement>& outliers)
{
    QMutexLocker locker(&mutex);
    for (uint i = 0; i < outliers.size(); i++) {
        outlierMask.at<int>(outliers[i].getFrom()) = 1;
    }
}

const Mat& Frame::getOutlierMask()
{
    QMutexLocker locker(&mutex);
    return outlierMask;
}

vector<Point2f> Frame::getOutliers() const
{
    QMutexLocker locker(&mutex);
    vector<Point2f> outliers;
    for (uint i = 0; i < displacements.size(); i++)
    {
        Displacement disp = displacements[i];
        const Point2f& from = disp.getFrom();
        if (outlierMask.at<int>(from) == 1) {
            outliers.push_back(from);
        }
    }
    return outliers;
}

vector<Point2f> Frame::getInliers() const
{
    QMutexLocker locker(&mutex);
    vector<Point2f> inliers;
    for (uint i = 0; i < displacements.size(); i++)
    {
        Displacement disp = displacements[i];
        const Point2f& from = disp.getFrom();
        if (outlierMask.at<int>(from) != 1) {
            inliers.push_back(from);
        }
    }
    return inliers;
}

void Frame::getInliersAndOutliers(vector<Point2f>& srcPoints, vector<Point2f>& destPoints) const
{
    QMutexLocker locker(&mutex);
    assert(srcPoints.size() == 0 && destPoints.size() == 0);
    for (uint i = 0; i < displacements.size(); i++) {
        const Displacement& disp = displacements.at(i);
        const Point2f src = disp.getFrom();
        const Point2f dest = disp.getTo();
        srcPoints.push_back(src);
        destPoints.push_back(dest);
    }
    assert(srcPoints.size() == destPoints.size());
}

void Frame::getInliers(vector<Point2f>& srcPoints, vector<Point2f>& destPoints) const
{
    QMutexLocker locker(&mutex);
    assert(srcPoints.size() == 0 && destPoints.size() == 0);
    for (uint i = 0; i < displacements.size(); i++) {
        const Displacement& disp = displacements.at(i);
        const Point2f src = disp.getFrom();
        const Point2f dest = disp.getTo();
        if (outlierMask.at<int>(src) != 1) {
            srcPoints.push_back(src);
            destPoints.push_back(dest);
        }
    }
    assert(srcPoints.size() == destPoints.size());
    assert(((uint) cv::countNonZero(outlierMask)) + destPoints.size() == displacements.size());
}

void Frame::setAffineTransform(const Mat& affine)
{
    QMutexLocker locker(&mutex);
    affine.copyTo(this->affine);
}

void Frame::setUpdateTransform(const Mat& update)
{
    QMutexLocker locker(&mutex);
    update.copyTo(this->update);
}


void Frame::setFeatures (const vector<Point2f>& features)
{
    QMutexLocker locker(&mutex);
    this->features = features;
}

void Frame::setFeature(Point2f* feature)
{
    this->feature = feature;
}

vector<Point2f> Frame::getFrom() const
{
    vector<Point2f> points;
    const vector<Displacement> displacements = getDisplacements();
    for (uint i = 0; i < displacements.size(); i++) {
        const Displacement d = displacements.at(i);
        points.push_back(d.getFrom());
    }
    return points;
}

vector<Point2f> Frame::getTo() const
{
    vector<Point2f> points;
    const vector<Displacement> displacements = getDisplacements();
    for (uint i = 0; i < displacements.size(); i++) {
        const Displacement d = displacements.at(i);
        points.push_back(d.getTo());
    }
    return points;
}
