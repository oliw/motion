#include "frame.h"
#include "displacement.h"
#include <opencv2/core/core.hpp>
#include <QDebug>
Frame::Frame()
{
}

Frame::Frame(const Mat& originalData):image(originalData)
{
    dx = Mat::zeros(originalData.rows, originalData.cols, DataType<float>::type);
    dy = Mat::zeros(originalData.rows, originalData.cols, DataType<float>::type);
    displacements.clear();
    displacementMask = Mat::zeros(originalData.rows, originalData.cols, DataType<int>::type);
    outlierMask = Mat::zeros(originalData.rows, originalData.cols, DataType<int>::type);
}

void Frame::registerDisplacement(const Displacement& displacement) {
    const Point2f& feature = displacement.getFrom();
    assert(displacementMask.at<int>(feature) == 0);
    displacementMask.at<int>(feature) = 1;
    dx.at<float>(feature) = displacement.getDisplacement().x;
    dy.at<float>(feature) = displacement.getDisplacement().y;
    displacements.push_back(displacement);
    assert(cv::countNonZero(displacementMask) == displacements.size());
}

const vector<Displacement>& Frame::getDisplacements() const {
    assert(cv::countNonZero(displacementMask) == displacements.size());
    return displacements;
}

vector<Displacement> Frame::getDisplacements(int ox, int oy, int gridSize) const {
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
    for (int i = 0; i < outliers.size(); i++) {
        outlierMask.at<int>(outliers[i].getFrom()) = 1;
    }
}

const Mat& Frame::getOutlierMask()
{
    return outlierMask;
}

vector<Point2f> Frame::getOutliers() const
{
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



