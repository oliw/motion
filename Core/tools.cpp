#include "tools.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>

Tools::Tools()
{
}

float Tools::eucDistance(Point2f a, Point2f b) {
    return sqrt(pow(b.x - a.x,2) + pow(b.y - a.y,2));
}

Point2f Tools::applyAffineTransformation(Mat affine, Point2f src)
{
    std::stringstream ss;
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
        curr = Tools::applyAffineTransformation(trans[i], curr);
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

RotatedRect Tools::transformRectangle(const Mat& affine, const Rect& origRect) {
    qDebug() << "Tools::transformRectangle - Started";
    assert(affine.rows == 2 && affine.cols == 3);
    vector<Point2f> verts;
    verts.push_back(Point2f(origRect.x, origRect.y));
    verts.push_back(Point2f(origRect.x, origRect.y+origRect.height));
    verts.push_back(Point2f(origRect.x+origRect.width, origRect.y));
    verts.push_back(Point2f(origRect.x+origRect.width, origRect.y+origRect.height));
    vector<Point2f> newVerts;
    for (uint i = 0; i < verts.size(); i++) {
        newVerts.push_back(Tools::applyAffineTransformation(affine, verts[i]));
    }
    qDebug() << "Tools::transformRectangle - Finished";
    std::stringstream ss;
    ss << "Affine Trans:" << affine << '\n';
    ss << "Old Coords: " << verts << '\n';
    ss << "New Coords: " << newVerts << '\n';
    qDebug() << QString::fromStdString(ss.str());
    return minAreaRect(newVerts);
}

Mat Tools::getCroppedImage(const Mat& image, const RotatedRect& rect) {
    qDebug() << "Tools::getCroppedImage - Started";
    Mat M, rotated, cropped;
    float angle = rect.angle;
    Size size = rect.size;
    if (angle < -45) {
        angle += 90.;
        double height = size.height;
        size.height = size.width;
        size.width = height;
    }
    M = getRotationMatrix2D(rect.center, angle, 1);
    qDebug() << "Warping Affine start";
    warpAffine(image, rotated, M, image.size(), INTER_CUBIC);
    qDebug() << "Warping Affine end";
    getRectSubPix(rotated, size, rect.center,cropped);
    qDebug() << "Tools::getCroppedImage - Finished";
    return cropped;
}

void Tools::trimVideo(Video* image) {
    qDebug() << "Tools::trimVideo - Start";
    // Find smallest frame
    Size area = image->getFrameAt(0)->getOriginalData().size();
    for (int i = 0; i < image->getFrameCount(); i++) {
        Frame* f = image->accessFrameAt(i);
        Size thisArea = f->getOriginalData().size();
        if (thisArea.width < area.width) {
            area.width = thisArea.width;
        }
        if (thisArea.height < area.height) {
            area.height = thisArea.height;
        }
    }
    qDebug() << "Smallest area is " << area.width << "," << area.height;
    // Trim all frames
    for (int i = 0; i < image->getFrameCount(); i++) {
        qDebug() << "Trimming frame " << i;
        Frame* f = image->accessFrameAt(i);
        f->trim(area);
    }
    qDebug() << "Tools::trimVideo - End";
}

Point2f Tools::QPointToPoint2f(QPoint p) {
    Point2f result;
    float x = p.x();
    float y = p.y();
    result.x = x;
    result.y = y;
    return result;
}

QMap<int, Point2f> Tools::moveToOriginDataSet(const QMap<int, Point2f>& dataSet) {
    Point2f firstPoint = dataSet.begin().value();
    QMap<int, Point2f> result;
    QMapIterator<int, Point2f> i(dataSet);
    while (i.hasNext()) {
        i.next();
        Point2f p = i.value();
        p.x = p.x - firstPoint.x;
        p.y = p.y - firstPoint.y;
        result.insert(i.key(), p);
    }
    return result;
}

