#ifndef FRAME_H
#define FRAME_H
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

class Frame
{
public:
    Frame();
    Frame(Mat originalData, int i);

    Mat const& getOriginalData() const {return originalData;}

private:
    Mat originalData;
    int i;

};

#endif // FRAME_H
