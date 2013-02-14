#include "video.h"
#include "frame.h"
#include <QList>
#include <QDebug>
#include <opencv2/core/core.hpp>

using namespace cv;

Video::Video(int frameCount, QObject *parent):QObject(parent),mutex()
{
    frames.reserve(frameCount);
}

Video::~Video()
{
    delete cropBox;
}


void Video::initCropBox()
{
    const Frame& f = frames.at(0);
    const Mat& img = f.getOriginalData();
    // default cropbox in the centre 50x50
    Size_<int> imgSize = img.size();
    int x,y;
    x = (imgSize.height/2)-(50/2);
    y = (imgSize.width/2)-(50/2);
    cropBox = new Rect_<int>(x,y,50,50);
    qDebug() << "Cropbox initialised";
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
}

