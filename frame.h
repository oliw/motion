#ifndef FRAME_H
#define FRAME_H
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
using namespace std;
using namespace cv;

class Frame
{
public:
    Frame();
    Frame(const Mat& originalData);

    Mat const& getOriginalData() const {return originalData;}

    const vector<KeyPoint>& getKeyPoints() const {return detectedSalientPoints;}
    void setKeyPoints(const vector<KeyPoint>& detectedSalientPoints);


    void setDetectedPointsFromPreviousFrame(const vector<Point2f>& points);

private:
    Mat originalData;
    vector<KeyPoint> detectedSalientPoints;
    vector<Point2f> detectedPointsFromPreviousFrame;
};

#endif // FRAME_H
