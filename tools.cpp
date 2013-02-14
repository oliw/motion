#include "tools.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>
#include <iostream>
#include <stdio.h>

Tools::Tools()
{
}

float Tools::eucDistance(Point2f a, Point2f b) {
    return sqrt(pow(b.x - a.x,2) + pow(b.y - a.y,2));
}

Point2f applyAffineTransformation(Mat affine, Point2f src)
{
    affine.convertTo(affine,CV_32FC1,1,0); //NOW A IS FLOAT

    vector<Point3f> vec;
    vec.push_back(Point3f(src.x,src.y,1));

    Mat srcMat = Mat(vec).reshape(1).t();
    Mat dstMat = affine*srcMat; //USE MATRIX ALGEBRA

    return Point2f(dstMat.at<float>(0,0),dstMat.at<float>(1,0));
}

void Tools::applyAffineTransformations(Point2f start, vector<Mat> trans, vector<double>& time, vector<double>& x, vector<double>& y)
{
    // Work Backwards
    time.reserve(trans.size()+1);
    x.reserve(trans.size()+1);
    y.reserve(trans.size()+1);
    time.push_back(trans.size());
    x.push_back(0);
    y.push_back(0);
    Point2f curr(start);
    for (int i = trans.size()-1; i >= 0; i--) {
        time.push_back(i);
        std::stringstream str;
        str << "Applying transformation " << trans[i] << " to " << curr;
        curr = applyAffineTransformation(trans[i], curr);
        x.push_back(curr.x);
        y.push_back(curr.y);
    }
    std::reverse(time.begin(), time.end());
    std::reverse(x.begin(), x.end());
    std::reverse(y.begin(), y.end());
    // Shift everything
    int shiftX = x[0];
    int shiftY = y[0];
    for (uint i = 0; i <= trans.size(); i++) {
        x[i] = x[i] - shiftX;
        y[i] = y[i] - shiftY;
    }
}
