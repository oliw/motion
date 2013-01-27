#ifndef VIDEO_H
#define VIDEO_H
#include <QObject>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include "frame.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

class Video : public QObject
{
    Q_OBJECT

public:
    Video(QObject *parent = 0);
    QList<Frame>& getFrames();

    void appendFrame(Frame* frame);

    const Frame* getFrameAt(int frameNumber) const;
    Frame* accessFrameAt(int frameNumber);
    const Mat& getImageAt(int frameNumber) const;
    int getFrameCount() const;

    int getWidth() const;
    int getHeight() const;

    vector<Mat> getAffineTransforms() const;

private:
    mutable QMutex mutex;
    QList<Frame*> frames;
};

#endif // VIDEO_H
