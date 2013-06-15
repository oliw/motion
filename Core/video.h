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

/*
 *
 *  This is the internal representation of the video file
 *  read in (and the video file created).
 *
 */
class Video : public QObject
{
    Q_OBJECT

public:
    Video(int frameCount, int fps = 27, QObject *parent = 0);
    ~Video();

    QList<Frame>& getFrames();
    void appendFrame(Frame* frame);

    const Frame* getFrameAt(int frameNumber) const;
    Frame* accessFrameAt(int frameNumber);
    const Mat& getImageAt(int frameNumber) const;
    int getFrameCount() const;

    Size getSize() const;
    int getWidth() const;
    int getHeight() const;
    int getOrigFps() const {return originalFps;}

    vector<Mat> getAffineTransforms() const;

    void setCropBox(int x, int y, int width, int height);
    const Rect_<int>& getCropBox() const {return cropBox;}

    void setVideoName(const QString& name) {videoName = name;}
    QString getVideoName() {return videoName;}

    void reset();

private:
    mutable QMutex mutex;

    QString videoName;
    QList<Frame*> frames;
    int originalFps;
    Rect_<int> cropBox; // The starting crop box

    void initCropBox();
};

#endif // VIDEO_H
