#include "frame.h"
#include <opencv2/core/core.hpp>
Frame::Frame()
{
}

Frame::Frame(const Mat& originalData):image(originalData)
{
    dx = Mat(originalData.rows, originalData.cols, DataType<float>::type);
    dy = Mat(originalData.rows, originalData.cols, DataType<float>::type);
    featureMatrix = Mat(originalData.rows, originalData.cols, DataType<float>::type);
}

void Frame::registerOpticalFlow(const Point2f& feature, float dx, float dy ) {
    this->dx.at<float>(feature) = dx;
    this->dy.at<float>(feature) = dy;
    featureMatrix.at<float>(feature) = 1;
}

