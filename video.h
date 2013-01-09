#ifndef VIDEO_H
#define VIDEO_H
#include <QList>
#include "frame.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

class Video
{
public:
    Video();
    Video(QList<Frame> frames);
    QList<Frame>& getFrames();

    Mat getImageAt(int frameNumber);
    int getFrameCount();
    void play();


private:
    QList<Frame> frames;
};

#endif // VIDEO_H
