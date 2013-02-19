#include "video.h"
#include "frame.h"
#include <QList>
#include <QDebug>
#include <opencv2/core/core.hpp>

using namespace cv;

Video::Video(int frameCount, int fps, QObject *parent):QObject(parent),mutex(QMutex::Recursive),originalFps(fps)
{
    frames.reserve(frameCount);
}

Video::~Video()
{
}


void Video::initCropBox()
{
    // default cropbox in the centre 50x50
    int width = 100;
    int x,y;
    qDebug() << getWidth() << "," << getHeight();
    x = (getWidth()/2)-(width/2);
    y = (getHeight()/2)-(width/2);
    cropBox = Rect_<int>(x,y,width,width);
}

void Video::appendFrame(Frame* frame)
{
    QMutexLocker locker(&mutex);
    frames.append(frame);
    if (frames.size() == 1) {
        initCropBox();
    }
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
    for (int i = 1; i < frames.size(); i++) {
        const Mat& transform = frames[i]->getAffineTransform();
        transforms.push_back(transform);
    }
    return transforms;
}

Size Video::getSize() const {
    QMutexLocker locker(&mutex);
    assert (frames.size() > 0);
    return frames.at(0)->getOriginalData().size();
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

void Video::setStillPath(QList<Mat>& stillPath)
{
    this->stillPath = stillPath;
}

const QList<Mat>& Video::getStillPath() const {
    return stillPath;
}

void Video::setCropBox(int x, int y, int width, int height) {
    QMutexLocker locker(&mutex);
    QString msg = QString("Video::setCropBox - Setting Crop Box in current video to %1,%2 width: %3 height %4")
            .arg(QString::number(x),QString::number(y),QString::number(width),QString::number(height));
    qDebug() << msg;
    cropBox = Rect_<int>(x,y,width,height);
}
