#ifndef DISPLACEMENT_H
#define DISPLACEMENT_H

#include <opencv2/core/core.hpp>

using namespace cv;

class Displacement
{
public:
    Displacement(const Point2f& from, const Point2f& to);

    static float similarity(const Displacement& a, const Displacement& b);
    const Point2f getDisplacement() const;
    const Point2f& getFrom() const;
    const Point2f& getTo() const;

private:
    Point2f from;
    Point2f to;
};

#endif // DISPLACEMENT_H
