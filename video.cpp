#include "video.h"
#include "frame.h"
#include <QList>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>

using namespace cv;

Video::Video()
{

}

Video::Video(QList<Frame> frames) : frames(frames)
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

int Video::getFrameCount()
{
    return frames.size();
}

void Video::play()
{
    cv::namedWindow("window", CV_WINDOW_AUTOSIZE);
    for (int i = 0; i < frames.size(); i++) {
    cv::imshow("window", getImageAt(i));
    waitKey(50);
    }
}
