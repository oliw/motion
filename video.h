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
    Video(const QList<Frame>& frames);
    QList<Frame>& getFrames();

    const Frame& getFrameAt(int frameNumber) const;
    Frame& accessFrameAt(int frameNumber);
    Mat getImageAt(int frameNumber);
    int getFrameCount();

private:
    QList<Frame> frames;
};

#endif // VIDEO_H
