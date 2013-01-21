#include "ransacmodel.h"
#include "displacement.h"
#include "tools.h"
#include <opencv2/core/core.hpp>
#include <QObject>

using namespace std;

RansacModel::RansacModel(const Point2f& displacement, const vector<Displacement>& outliers, const vector<Displacement>& inliers)
{
    this->displacement = displacement;
    this->inliers = inliers;
    this->outliers = outliers;
}

float RansacModel::getInlierError()
{
    float inlierError = 0;
    for (uint i = 0; i < inliers.size(); i++) {
        const Displacement& inlier = inliers.at(i);
        inlierError += Tools::eucDistance(inlier.getFrom()+displacement, inlier.getTo());
    }
    return inlierError;
}

const vector<Displacement>& RansacModel::getOutliers() const
{
    return outliers;
}

const vector<Displacement>& RansacModel::getInliers() const
{
    return inliers;
}
