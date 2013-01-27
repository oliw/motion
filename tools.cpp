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

void Tools::applyAffineTransformations(Point2f start, vector<Mat> trans, vector<double>& time, vector<double>& xs, vector<double>& ys)
{
    int timeSteps = trans.size();
    time.push_back(0);
    xs.push_back(start.x);
    ys.push_back(start.y);
    Point2f currentLoc = start;
    for (int i = 0; i < timeSteps; i++) {
        time.push_back(i+1);
        currentLoc = applyAffineTransformation(trans[i],currentLoc);
        xs.push_back(currentLoc.x);
        ys.push_back(currentLoc.y);
    }
}
