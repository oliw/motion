#include "video.h"
#include "frame.h"
#include <QList>
#include <QDebug>
#include <opencv2/core/core.hpp>

using namespace cv;

Video::Video()
{

}

Video::Video(const QList<Frame>& frames) : frames(frames)
{
}

Mat Video::getImageAt(int frameNumber)
{
    const Frame& f = frames.at(frameNumber);
    const Mat& img = f.getOriginalData();
    if (!img.data)
    {
        qDebug() << "No image data found in frame " << frameNumber;
    }
    return img;
}

const Frame& Video::getFrameAt(int frameNumber) const
{
    return frames.at(frameNumber);
}

Frame& Video::accessFrameAt(int frameNumber)
{
    return frames[frameNumber];
}

int Video::getFrameCount()
{
    return frames.size();
}
