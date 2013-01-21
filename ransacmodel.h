#ifndef RANSACMODEL_H
#define RANSACMODEL_H

#include <opencv2/core/core.hpp>
#include "displacement.h"

using namespace std;
using namespace cv;

class RansacModel
{
public:
    explicit RansacModel(const Point2f& displacement, const vector<Displacement>& outliers, const vector<Displacement>& inliers);

    float getInlierError();
    const vector<Displacement>& getOutliers() const;
    const vector<Displacement>& getInliers() const;

    Point2f displacement;
    vector<Displacement> outliers;
    vector<Displacement> inliers;
};

#endif // RANSACMODEL_H
