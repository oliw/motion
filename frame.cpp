#include "frame.h"
#include <opencv2/core/core.hpp>
Frame::Frame()
{
}

Frame::Frame(const Mat& originalData):originalData(originalData)
{

}

void Frame::setKeyPoints(const vector<KeyPoint>& keypoints) {
    detectedSalientPoints = keypoints;
}

void Frame::setDetectedPointsFromPreviousFrame(const vector<Point2f>& points) {
    detectedPointsFromPreviousFrame = points;
}
