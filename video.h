#ifndef VIDEO_H
#define VIDEO_H
#include <QObject>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QRect>
#include "frame.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

class Video : public QObject
{
    Q_OBJECT

public:
    Video(int frameCount, QObject *parent = 0);
    ~Video();

    QList<Frame>& getFrames();
    void appendFrame(Frame* frame);

    const Frame* getFrameAt(int frameNumber) const;
    Frame* accessFrameAt(int frameNumber);
    const Mat& getImageAt(int frameNumber) const;
    int getFrameCount() const;

    int getWidth() const;
    int getHeight() const;

    vector<Mat> getAffineTransforms() const;

    void setStillPath(QList<Mat>& stillPath);
    const QList<Mat>& getStillPath() const;

    void setCropBox(int x, int y, int width, int height);

private:
    mutable QMutex mutex;

    QList<Frame*> frames;

    Rect_<int>* cropBox; // The starting crop box
    QList<Mat> stillPath;

    void initCropBox();
};

#endif // VIDEO_H
