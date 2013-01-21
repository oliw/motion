#ifndef FRAME_H
#define FRAME_H
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "displacement.h"
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

    void registerDisplacement(const Displacement& displacement);
    vector<Displacement> getDisplacements(int x, int y, int gridSize) const;
    const vector<Displacement>& getDisplacements() const;

    void registerOutliers(const vector<Displacement>& outliers);
    const Mat& getOutlierMask();

    vector<Point2f> getOutliers() const;
    vector<Point2f> getInliers() const;


private:
    Mat image;

    // Detected features
    vector<Point2f> features;

    // Optical flow matrices
    Mat dx,dy;            // Set to displacement in correct direction from original point
    Mat displacementMask; // Set to 1 if feature is at this point
    vector<Displacement> displacements;

    // Outlier Rejection
    Mat outlierMask; // Set to 1 if feature at this point is an outlier
};

#endif // FRAME_H
