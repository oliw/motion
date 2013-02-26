#ifndef TOOLS_H
#define TOOLS_H
#include <opencv2/core/core.hpp>

using namespace cv;

class Tools
{
public:
    Tools();
    static float eucDistance(Point2f a, Point2f b);
    static Point2f applyAffineTransformation(Mat affine, Point2f src);
    static void applyAffineTransformations(Point2f start, vector<Mat> trans, vector<double>& time, vector<double>& xs, vector<double>& ys);
    static RotatedRect applyTransformation(const Mat& affine, const Rect& origRect);
};

#endif // TOOLS_H
