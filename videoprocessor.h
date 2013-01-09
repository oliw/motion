#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include "video.h"
#include <string>
using namespace std;

class VideoProcessor
{

public:
    VideoProcessor();

    bool loadVideo(string path);
    void reset();
    Video& getVideo() {return video;}

private:
    Video video;
    Video greyVideo;
};

#endif // VIDEOPROCESSOR_H
