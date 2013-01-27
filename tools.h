#ifndef TOOLS_H
#define TOOLS_H
#include <opencv2/core/core.hpp>

using namespace cv;

class Tools
{
public:
    Tools();
    static float eucDistance(Point2f a, Point2f b);
    static void applyAffineTransformations(Point2f start, vector<Mat> trans, vector<double>& time, vector<double>& xs, vector<double>& ys);
};

#endif // TOOLS_H
