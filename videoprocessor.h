#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include "video.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/video/video.hpp"
#include <string>
using namespace std;

class VideoProcessor : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcessor(QObject *parent = 0);
    ~VideoProcessor();

    void reset();
    Video& getVideo() {return video;}

    bool loadVideo(string path);
    bool detectFeatures();
    bool trackFeatures();
    bool outlierRejection();

private:
    Video video;

};

#endif // VIDEOPROCESSOR_H
