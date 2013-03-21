#ifndef TOOLS_H
#define TOOLS_H
#include <opencv2/core/core.hpp>
#include "video.h"
#include "frame.h"
#include "evaluator.h"

using namespace cv;

class Tools
{
public:
    Tools();
    static float eucDistance(Point2f a, Point2f b);
    static Point2f applyAffineTransformation(Mat affine, Point2f src);
    static void applyAffineTransformations(Point2f start, vector<Mat> trans, vector<double>& time, vector<double>& xs, vector<double>& ys);
    static RotatedRect transformRectangle(const Mat& affine, const Rect& origRect);
    static Mat getCroppedImage(const Mat& image, const RotatedRect& rect);
    static Point2f QPointToPoint2f(QPoint p);
    static void trimVideo(Video* image);


    // Move coordinates to start from 0,0
    DataSet moveToOriginDataSet(const DataSet& dataSet);

    // Builds coordinates from original manual motion

    // Builds coordinates from original video global motion
    DataSet buildDiscreteDataSet();

    // Builds coordinates with motion update and then camera update
    DataSet buildNewVideoDataSet(Video* video);
};

#endif // TOOLS_H
