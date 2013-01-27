#include "video.h"
#include "frame.h"
#include <QList>
#include <QDebug>
#include <opencv2/core/core.hpp>

using namespace cv;

Video::Video(QObject *parent):QObject(parent),mutex()
{

}

void Video::appendFrame(Frame* frame)
{
    QMutexLocker locker(&mutex);
    frames.append(frame);
}


const Mat& Video::getImageAt(int frameNumber) const
{
    QMutexLocker locker(&mutex);
    const Frame& f = frames.at(frameNumber);
    const Mat& img = f.getOriginalData();
    if (!img.data)
    {
        qDebug() << "No image data found in frame " << frameNumber;
    }
    return img;
}

const Frame* Video::getFrameAt(int frameNumber) const
{
    QMutexLocker locker(&mutex);
    return frames.at(frameNumber);
}

Frame* Video::accessFrameAt(int frameNumber)
{
    QMutexLocker locker(&mutex);
    return frames[frameNumber];
}

int Video::getFrameCount() const
{
    QMutexLocker locker(&mutex);
    return frames.size();
}

//TODO This shouldnt be here
vector<Mat> Video::getAffineTransforms() const
{
    QMutexLocker locker(&mutex);
    vector<Mat> transforms;
    for (int i = 0; i < frames.size() -1; i++) {
        const Mat& transform = frames[i]->getAffineTransform();
        transforms.push_back(transform);
    }
    return transforms;
}

int Video::getWidth() const {
    QMutexLocker locker(&mutex);
    if (frames.size() == 0) {
        return 0;
    }
    return frames.at(0)->getOriginalData().size().width;

}
int Video::getHeight() const {
    QMutexLocker locker(&mutex);
    if (frames.size() == 0) {
        return 0;
    }
    return frames.at(0)->getOriginalData().size().height;
}
