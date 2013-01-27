#ifndef GRAPHDRAWER_H
#define GRAPHDRAWER_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include "engine.h"
#include "video.h"

using namespace std;
using namespace cv;

class GraphDrawer : public QObject
{
    Q_OBJECT
public:
    explicit GraphDrawer(QObject *parent = 0);
    ~GraphDrawer();

    void showMotionGraph(Point2f start, const vector<Mat>& transforms);
signals:
    
public slots:
    void setVideo(const Video* video) {this->video = video;}
    void drawOriginalMotionGraph(Point2f start);

private:
    Engine* mEngine;
    const Video* video;
    static mxArray* vectorToMatlabFormat(const vector<double>& original);
    
};

#endif // GRAPHDRAWER_H


/*
 * Example Usage
 *  - Console
 *      - Save x direction and y direction in folder
 *      - Title contains config information
 *
 *
 *  Results Folder
 *      - Original Camera Path - x
 *      - Original Camera Path - y
 *      - New Camera Path - x
 *      - New Camera Path - y
 *      - End Result Video
 *      - Text File with Score and Settings
 */
