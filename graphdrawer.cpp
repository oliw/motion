#include "graphdrawer.h"
#include <QDebug>
#include "engine.h"
#include "tools.h"
#include "videoprocessor.h"

GraphDrawer::GraphDrawer(VideoProcessor* vp, QObject *parent) :
    QObject(parent),vp(vp)
{
    mEngine = engOpen(NULL);
    assert(mEngine != NULL);
}

GraphDrawer::~GraphDrawer()
{
    engClose(mEngine);
}

mxArray* GraphDrawer::vectorToMatlabFormat(const vector<double>& original)
{
    mxArray *arr = mxCreateDoubleMatrix(original.size(), 1, mxREAL);
    double *p_arr = mxGetPr(arr);
    for (uint i = 0; i < original.size(); i++)
    {
        p_arr[i] = original.at(i);
    }
    return arr;
}

void GraphDrawer::drawOriginalMotionGraph(int x, int y) {
    qDebug() << "GraphDrawer::drawOriginalMotionGraph - Drawing graphs starting at ("<<x<<","<<y<<")";
    const Video* video = vp->getVideo();
    const vector<Mat>& transforms = video->getAffineTransforms();
    showMotionGraph(Point2f(x,y));
}

void GraphDrawer::showMotionGraph(Point2f start) {
    const Video* original = vp->getVideo();
    // Calculate values
    vector<double> times;
    // Original Video Movement
    vector <double> xss, yss;
    Tools::applyAffineTransformations(start, original->getAffineTransforms(), times, xss, yss);
    const Video* cropped = 0;
    // Convert values to Matlab format
    mxArray *time = vectorToMatlabFormat(times);
    mxArray *xs = vectorToMatlabFormat(xss);
    mxArray *ys = vectorToMatlabFormat(yss);
    // Give values to Matlab
    engPutVariable(mEngine, "time", time);
    engPutVariable(mEngine, "xs", xs);
    engPutVariable(mEngine, "ys", ys);
    if (cropped != 0) {
        // Cropped Video Movement
        vector <double> cxss, cyss;
        Tools::applyAffineTransformations(start, cropped->getAffineTransforms(), times, cxss, cyss);
        mxArray *cxs = vectorToMatlabFormat(cxss);
        mxArray *cys = vectorToMatlabFormat(cyss);
        engPutVariable(mEngine, "cxs", cxs);
        engPutVariable(mEngine, "cys", cys);
        // Draw two graphs
        engEvalString(mEngine, "figure; plot(xs,time,cxs,time); xlabel('x'); ylabel('Time /frame'); legend('Original','Cropped')");
        engEvalString(mEngine, "figure; plot(ys,time,cys,time); xlabel('y'); ylabel('Time /frame'); legend('Original','Cropped')");
    } else {
        engEvalString(mEngine, "figure; plot(xs,time); xlabel('x'); ylabel('Time /frame'); legend('Original')");
        engEvalString(mEngine, "figure; plot(ys,time); xlabel('y'); ylabel('Time /frame'); legend('Original')");
    }
}
