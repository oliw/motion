#ifndef FRAME_H
#define FRAME_H
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "feature.h"
using namespace std;
using namespace cv;

class Frame
{
public:
    Frame();
    Frame(const Mat& image);

    Mat const& getOriginalData() const {return image;}

    vector<Point2f>& accessFeatures() {return features;}
    const vector<Point2f>& getFeatures() const {return features;}

    void registerOpticalFlow(const Point2f& feature, float dx, float dy);

private:
    Mat image;

    // Detected features
    vector<Point2f> features;

    // Optical flow matrices
    Mat featureMatrix;
    Mat dx,dy;
};

#endif // FRAME_H
