#ifndef GRAPHDRAWER_H
#define GRAPHDRAWER_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include "videoprocessor.h"
#include "engine.h"
#include "video.h"

using namespace std;
using namespace cv;

class GraphDrawer : public QObject
{
    Q_OBJECT
public:
    explicit GraphDrawer(VideoProcessor* vp, QObject *parent = 0);
    ~GraphDrawer();

    void showMotionGraph(Point2f start);
signals:
    
public slots:
    void drawOriginalMotionGraph(int x, int y);

private:
    Engine* mEngine;
    VideoProcessor* const vp;
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
