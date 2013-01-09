#include "videoprocessor.h"
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QList>
#include "frame.h"
#include "video.h"
#include <stdio.h>
#include <QDebug>

using namespace std;

VideoProcessor::VideoProcessor() {

}

void VideoProcessor::reset(){
    video = Video();
    greyVideo = Video();
}

bool VideoProcessor::loadVideo(string path) {
    cv::VideoCapture vc;
    QList<Frame> frames, greyFrames;
    bool videoOpened = vc.open(path);
    if (!videoOpened) {
        qDebug() << "VideoProcessor::loadVideo - Video could not be opened";
        return false;
    }
    qDebug() << "VideoProcessor::loadVideo - Video opened";
    int i = 0;
    Mat buffer,a,b;
    while (vc.read(buffer)) {
        frames.append(Frame(buffer.clone(),i));
        i++;
    }
    qDebug() << "VideoProcessor::loadVideo - "<< frames.size() << " Video frames grabbed";
    video = Video(frames);
    greyVideo = Video(greyFrames);
    return true;
}
