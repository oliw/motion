#ifndef TOOLS_H
#define TOOLS_H
#include <opencv2/core/core.hpp>
#include "video.h"
#include "frame.h"

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

    static void trimVideo(Video* image);
};

#endif // TOOLS_H
