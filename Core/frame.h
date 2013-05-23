#ifndef FRAME_H
#define FRAME_H
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "displacement.h"
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
using namespace std;
using namespace cv;

class Frame : public QObject
{
    Q_OBJECT

public:
    Frame(QObject *parent = 0);
    Frame(const Mat& image, QObject *parent = 0);

    void resize();

    const Mat& getOriginalData() const {QMutexLocker locker(&mutex); return image;}

    void setFeatures (const vector<Point2f>& features);
    const vector<Point2f>& getFeatures() const {QMutexLocker locker(&mutex); return features;}

    void registerDisplacement(const Displacement& displacement);
    vector<Displacement> getDisplacements(int x, int y, int gridSize) const;
    const vector<Displacement>& getDisplacements() const;

    void registerOutliers(const vector<Displacement>& outliers);
    const Mat& getOutlierMask();

    vector<Point2f> getOutliers() const;
    vector<Point2f> getInliers() const;

    void getInliers(vector<Point2f>& srcPoints, vector<Point2f>& destPoints) const;

    void setAffineTransform(const Mat& affine);
    const Mat& getAffineTransform() const {QMutexLocker locker(&mutex); return affine;}

    void setUpdateTransform(const Mat& update);
    const Mat& getUpdateTransform() const {QMutexLocker locker(&mutex); return update;}

    void setFeature(Point2f* feature);
    Point2f* getFeature() {return feature;}

    void trim(Size area);

private:

    mutable QMutex mutex;

    Mat image;

    // Manually Selected Feature
    Point2f* feature;

    // Detected features
    vector<Point2f> features;

    // Optical flow matrices
    Mat dx,dy;            // Set to displacement in correct direction from original point
    Mat displacementMask; // Set to 1 if feature is at this point
    vector<Displacement> displacements;

    // Outlier Rejection
    Mat outlierMask; // Set to 1 if feature at this point is an outlier

    // Affine Transformation 2x3
    Mat affine;

    // Camera Update 2x3
    Mat update;
};

#endif // FRAME_H
