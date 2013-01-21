#ifndef TOOLS_H
#define TOOLS_H
#include <opencv2/core/core.hpp>

using namespace cv;

class Tools
{
public:
    Tools();
    static float eucDistance(Point2f a, Point2f b);
};

#endif // TOOLS_H
